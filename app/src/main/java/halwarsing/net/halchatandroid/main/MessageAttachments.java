package halwarsing.net.halchatandroid.main;

import android.content.ActivityNotFoundException;
import android.content.Intent;
import android.net.Uri;
import android.text.format.Formatter;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.content.res.AppCompatResources;

import com.bumptech.glide.Glide;
import com.bumptech.glide.load.engine.DiskCacheStrategy;
import com.google.android.flexbox.FlexboxLayout;
import com.google.android.material.progressindicator.CircularProgressIndicator;
import com.google.android.material.snackbar.Snackbar;

import org.json.JSONArray;

import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ConcurrentHashMap;
import java.util.function.BooleanSupplier;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.type.HCMessage;
import halwarsing.net.halchatandroid.type.HDFile;

/* Состояние вложения
скачано - картинка/превью видео, у видео кнопка ▶, у файла размер
скачивается - круглый прогресс с крестиком (нажатие отменяет), "загружено / всего"
не скачано - кнопка скачивания; картинки скачиваются сами при показе в открытом чате, видео и файлы по нажатию
*/
//Вложения сообщения в чате: показ, скачивание с прогрессом, открытие и сохранение в Загрузки
public class MessageAttachments {
    private static final String TAG="HCAATTACH";
    private static final int MAX_MEDIA_WIDTH_DP=250;
    private final ChatActivity chatActivity;
    private final HalChat hc;
    private final Set<String> cancelledByUser=ConcurrentHashMap.newKeySet();

    public MessageAttachments(ChatActivity chatActivity,HalChat hc) {
        this.chatActivity=chatActivity;
        this.hc=hc;
    }

    private static class AttachmentView {
        View root;
        ImageView icon,actionIcon;
        View action;
        CircularProgressIndicator progress;
        TextView status;
        HDFile file;
        String type;
        boolean isMedia;
    }

    /**
     * Тип вложения для показа: по fileType из HalDrive, иначе по MIME-типу
     *
     * @param hdFile Файл
     * @return image, video или file
     */
    private static String getType(HDFile hdFile) {
        if("image".equals(hdFile.fileType)||"video".equals(hdFile.fileType))return hdFile.fileType;
        String mimeType=HalDrive.getMimeType(hdFile);
        if(mimeType.startsWith("image/"))return "image";
        if(mimeType.startsWith("video/"))return "video";
        return "file";
    }

    /**
     * Показывает вложения сообщения. Ячейки добавляются сразу, чтобы сохранить порядок, содержимое — после getInfo
     *
     * @param container Контейнер вложений сообщения
     * @param message Сообщение
     * @param isBound true, пока ячейка списка показывает это сообщение
     * @return void
     */
    public void bind(FlexboxLayout container,HCMessage message,BooleanSupplier isBound) {
        JSONArray attachments=message.attachments;
        if(attachments==null)return;
        for(int i=0;i<attachments.length();i++) {
            String id=attachments.optString(i,"");
            if(id.isEmpty()||id.equals("-1"))continue;
            FrameLayout slot=new FrameLayout(chatActivity);
            container.addView(slot);
            hc.hd.getFileInfo(id).whenComplete((hdFile,error)->chatActivity.runOnUiThread(()->{
                if(!isBound.getAsBoolean())return;
                if(error!=null||hdFile==null) {
                    Log.e(TAG,"Unable to load attachment info "+id,error);
                    return;
                }
                slot.addView(createView(slot,hdFile,message,isBound).root);
            }));
        }
    }

    private AttachmentView createView(ViewGroup parent,HDFile hdFile,HCMessage message,BooleanSupplier isBound) {
        AttachmentView av=new AttachmentView();
        av.file=hdFile;
        av.type=getType(hdFile);
        av.isMedia=!av.type.equals("file");
        av.root=LayoutInflater.from(chatActivity).inflate(av.isMedia?R.layout.file_image_item:R.layout.file_item,parent,false);
        av.icon=av.root.findViewById(R.id.fileicon);
        av.action=av.root.findViewById(R.id.fileAction);
        av.actionIcon=av.root.findViewById(R.id.fileActionIcon);
        av.progress=av.root.findViewById(R.id.fileProgress);
        av.status=av.root.findViewById(R.id.fileStatus);

        if(av.isMedia) {
            setMediaSize(av,hdFile);
            if(av.type.equals("video"))av.icon.setImageResource(R.drawable.video);
        } else {
            av.icon.setImageDrawable(AppCompatResources.getDrawable(chatActivity,hc.hd.getFileIcon(hdFile.fileType)));
            ((TextView)av.root.findViewById(R.id.filename)).setText(hdFile.name);
        }

        av.root.setOnClickListener((View v)->onClick(av,isBound));
        av.root.setOnLongClickListener((View v)->{
            chatActivity.showMessageContextMenu(message,v);
            return true;
        });

        boolean isDownloaded=hc.hd.isDownloaded(hdFile.id);
        if(isDownloaded) {
            showDownloaded(av);
        } else if(hc.hd.isDownloading(hdFile.id)||av.type.equals("image")) {
            startDownload(av,isBound,false);
        } else {
            showIdle(av);
        }
        return av;
    }

    //Размер по mimeData (width/height), чтобы ячейка не прыгала после скачивания
    private void setMediaSize(AttachmentView av,HDFile hdFile) {
        if(hdFile.mimeData==null)return;
        int originalWidth=hdFile.mimeData.optInt("width");
        int originalHeight=hdFile.mimeData.optInt("height");
        if(originalWidth<=0||originalHeight<=0)return;

        int maxWidthPx=(int)(MAX_MEDIA_WIDTH_DP*chatActivity.getResources().getDisplayMetrics().density);
        int targetWidth=Math.min(originalWidth,maxWidthPx);
        int targetHeight=(int)(targetWidth*((float)originalHeight/originalWidth));
        ViewGroup.LayoutParams params=av.icon.getLayoutParams();
        params.width=targetWidth;
        params.height=targetHeight;
        av.icon.setLayoutParams(params);
    }

    private void onClick(AttachmentView av,BooleanSupplier isBound) {
        String id=av.file.id;
        if(hc.hd.isDownloading(id)) {
            cancelledByUser.add(id);
            hc.hd.cancelDownload(id);
        } else if(hc.hd.isDownloaded(id)) {
            openAttachment(av.file);
        } else {
            startDownload(av,isBound,true);
        }
    }

    /**
     * Скачивает вложение и обновляет ячейку по прогрессу
     *
     * @param av Ячейка вложения
     * @param isBound true, пока ячейка показывает это сообщение
     * @param isUserAction true - по нажатию (при ошибке показывается сообщение)
     * @return void
     */
    private void startDownload(AttachmentView av,BooleanSupplier isBound,boolean isUserAction) {
        String id=av.file.id;
        cancelledByUser.remove(id);
        showProgress(av,0,-1);
        hc.hd.downloadAttachment(id,new HalDrive.HDDownloadFileEvent() {
            @Override
            public void onProgress(float progress,long total,long loaded) {
                chatActivity.runOnUiThread(()->{
                    if(isBound.getAsBoolean())showProgress(av,loaded,total);
                });
            }

            @Override
            public void onDownloaded(HDFile file) {
                boolean isDownloaded=hc.hd.isDownloaded(id);
                boolean isCancelled=cancelledByUser.remove(id);
                chatActivity.runOnUiThread(()->{
                    if(isUserAction&&!isDownloaded&&!isCancelled) {
                        Toast.makeText(chatActivity,R.string.file_download_failed,Toast.LENGTH_SHORT).show();
                    }
                    if(!isBound.getAsBoolean())return;
                    if(file!=null)av.file=file;
                    if(isDownloaded)showDownloaded(av); else showIdle(av);
                });
            }
        });
    }

    private void showIdle(AttachmentView av) {
        av.action.setVisibility(View.VISIBLE);
        av.progress.setVisibility(View.GONE);
        av.progress.setIndeterminate(true);
        av.actionIcon.setImageResource(R.drawable.ic_file_download);
        boolean hasSize=av.file.size>0;
        if(hasSize) {
            av.status.setText(Formatter.formatShortFileSize(chatActivity,av.file.size));
        } else if(!av.isMedia) {
            av.status.setText(R.string.file_download);
        }
        if(av.type.equals("video"))av.icon.setImageResource(R.drawable.video);
        av.status.setVisibility(hasSize||!av.isMedia?View.VISIBLE:View.GONE);
    }

    private void showProgress(AttachmentView av,long loaded,long total) {
        if(total<=0)total=av.file.size; // сервер не прислал Content-Length — размер из getInfo
        av.action.setVisibility(View.VISIBLE);
        av.actionIcon.setImageResource(R.drawable.ic_close);
        av.progress.setVisibility(View.VISIBLE);
        if(total>0) {
            if(av.progress.isIndeterminate()) {
                // Переключать на определённый прогресс можно только у скрытого индикатора
                av.progress.setVisibility(View.INVISIBLE);
                av.progress.setIndeterminate(false);
                av.progress.setVisibility(View.VISIBLE);
            }
            av.progress.setProgressCompat((int)Math.min(100,loaded*100/total),true);
            av.status.setText(chatActivity.getString(R.string.file_progress,
                    Formatter.formatShortFileSize(chatActivity,loaded),Formatter.formatShortFileSize(chatActivity,total)));
        } else {
            av.status.setText(loaded>0?Formatter.formatShortFileSize(chatActivity,loaded):"");
        }
        av.status.setVisibility(av.status.getText().length()>0?View.VISIBLE:View.GONE);
    }

    private void showDownloaded(AttachmentView av) {
        File file=hc.hd.getLocalFile(av.file.id);
        av.progress.setVisibility(View.GONE);
        String size=Formatter.formatShortFileSize(chatActivity,file.length());

        if(av.type.equals("image")) {
            av.action.setVisibility(View.GONE);
            av.status.setVisibility(View.GONE);
            loadPreview(av,file,R.drawable.image);
        } else if(av.type.equals("video")) {
            av.action.setVisibility(View.VISIBLE);
            av.actionIcon.setImageResource(R.drawable.ic_file_play);
            av.status.setText(size);
            av.status.setVisibility(View.VISIBLE);
            // Glide берёт кадр видео из локального файла
            loadPreview(av,file,R.drawable.video);
        } else {
            av.action.setVisibility(View.GONE);
            av.status.setText(size);
            av.status.setVisibility(View.VISIBLE);
        }
    }

    private void loadPreview(AttachmentView av,File file,int placeholder) {
        Glide.with(av.root)
                .load(file)
                .diskCacheStrategy(DiskCacheStrategy.ALL)
                .skipMemoryCache(false)
                .placeholder(placeholder)
                .error(placeholder)
                .into(av.icon);
    }

    /**
     * Открывает скачанное вложение: видео во встроенном плеере, остальное в выбранном приложении
     *
     * @param hdFile Файл
     * @return void
     */
    public void openAttachment(HDFile hdFile) {
        File file=hc.hd.getLocalFile(hdFile.id);
        if(getType(hdFile).equals("video")) {
            Intent intent=new Intent(chatActivity,VideoPlayerActivity.class);
            intent.putExtra(VideoPlayerActivity.EXTRA_PATH,file.getAbsolutePath());
            chatActivity.startActivity(intent);
            return;
        }
        Uri uri=HalDriveFileProvider.getUri(chatActivity,hdFile);
        openUri(uri,HalDriveFileProvider.getMimeTypeByName(uri.getLastPathSegment()));
    }

    private void openUri(Uri uri,String mimeType) {
        Intent intent=new Intent(Intent.ACTION_VIEW);
        intent.setDataAndType(uri,mimeType);
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        try {
            chatActivity.startActivity(Intent.createChooser(intent,chatActivity.getString(R.string.file_open)));
        } catch(ActivityNotFoundException e) {
            Toast.makeText(chatActivity,R.string.file_no_app,Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * Скачивает (если нужно) и сохраняет вложения сообщения в Загрузки/HalChat.
     * Разрешение на запись до Android 10 проверяет вызывающий
     *
     * @param message Сообщение с вложениями
     * @return void
     */
    public void saveFiles(HCMessage message) {
        List<CompletableFuture<Uri>> saves=new ArrayList<>();
        List<HDFile> files=new ArrayList<>();
        for(int i=0;i<message.attachments.length();i++) {
            String id=message.attachments.optString(i,"");
            if(id.isEmpty()||id.equals("-1"))continue;
            saves.add(hc.hd.downloadAttachment(id,new HalDrive.HDDownloadFileEvent() {
                @Override
                public void onProgress(float progress,long total,long loaded) {}

                @Override
                public void onDownloaded(HDFile file) {}
            }).thenCompose(file->TaskExecutorManager.getInstance().submitFileDownload("saveFile:"+id,()->{
                HDFile hdFile=hc.hd.getHDFileById(id);
                synchronized(files) {files.add(hdFile);}
                return hc.hd.saveToDownloads(hdFile);
            })));
        }
        if(saves.isEmpty())return;

        CompletableFuture.allOf(saves.toArray(new CompletableFuture[0])).whenComplete((v,error)->chatActivity.runOnUiThread(()->{
            if(chatActivity.isFinishing()||chatActivity.isDestroyed())return;
            if(error!=null) {
                Log.e(TAG,"Unable to save files",error);
                Toast.makeText(chatActivity,R.string.file_save_failed,Toast.LENGTH_SHORT).show();
                return;
            }
            Snackbar snackbar=Snackbar.make(chatActivity.findViewById(android.R.id.content),R.string.file_saved,Snackbar.LENGTH_LONG);
            if(saves.size()==1&&files.size()==1) {
                Uri uri=saves.get(0).join();
                String mimeType=HalDrive.getMimeType(files.get(0));
                snackbar.setAction(R.string.file_open,(View view)->openUri(uri,mimeType));
            }
            snackbar.show();
        }));
    }
}
