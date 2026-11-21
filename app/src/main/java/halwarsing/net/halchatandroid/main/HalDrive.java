package halwarsing.net.halchatandroid.main;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.provider.MediaStore;
import android.util.Log;
import android.webkit.MimeTypeMap;

import androidx.core.content.FileProvider;

import com.bumptech.glide.Glide;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.concurrent.Callable;
import java.util.concurrent.CancellationException;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

import javax.net.ssl.HttpsURLConnection;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.type.HDFile;

//Класс для взаимодействия с API HalDrive и всеми файлами экосистемы и мессенджера
public class HalDrive {
    private final Context context;
    private final SQLiteDatabase db;
    private final String codeUser;
    private static final String TAG="HCAHD";
    public static final String PATH_HD_FILES="HalDriveFiles/";
    public static final String SAVE_DIR="HalChat";
    public File directory;
    private HDUploadFileEvent uploadFileEvent;

    //Downloads
    private static final int CONNECT_TIMEOUT_MS=15000;
    private static final int READ_TIMEOUT_MS=30000; // на одно чтение, не на весь файл
    private final ConcurrentHashMap<String,CompletableFuture<File>> downloadPromises=new ConcurrentHashMap<>();
    private final ConcurrentHashMap<String,List<HDDownloadFileEvent>> downloadListeners=new ConcurrentHashMap<>();
    private final ConcurrentHashMap<String,long[]> downloadProgress=new ConcurrentHashMap<>(); // {loaded,total}
    private final Set<String> cancelledDownloads=ConcurrentHashMap.newKeySet();

    /**
     * Свой отправленный файл: кладётся в HalDriveFiles как уже скачанный, без повторного скачивания.
     * Информация (имя, тип, размер) берётся из getInfo
     *
     * @param id ID файла в HalDrive
     * @param file Отправленный файл (временная копия, перемещается)
     * @return HDFile из базы или null, если getInfo не ответил (информация дозапросится при показе)
     */
    public HDFile addUploadedFile(String id,File file) {
        File saveFile=getLocalFile(id);
        boolean isReplaced=saveFile.exists();
        if(file.exists()&&!file.equals(saveFile)&&!file.renameTo(saveFile)) {
            // renameTo не работает между разделами (cache → external), тогда копируем через .part
            File partFile=new File(directory,id+".hdf.part");
            try(InputStream in=new FileInputStream(file);OutputStream out=new FileOutputStream(partFile)) {
                copyStream(in,out);
            } catch(IOException e) {
                Log.e(TAG,"Unable to keep uploaded file "+id,e);
            }
            if(!partFile.renameTo(saveFile))partFile.delete();
        }
        if(isReplaced)clearGlideCache();
        try {
            fetchFileInfo(id);
        } catch(Exception e) {
            Log.e(TAG,"Unable to get info of uploaded file "+id,e);
        }
        return getHDFileById(id);
    }


    public HalDrive(Context scontext, SQLiteDatabase sdb, String scodeUser) {
        context=scontext;
        db=sdb;
        codeUser=scodeUser;
        directory=new File(scontext.getExternalFilesDir(null),PATH_HD_FILES);
        directory.mkdirs();
    }

    /**
     * Запрашивает информацию о файле в HalDrive (getInfo) и сохраняет её в `files`
     *
     * @param id ID файла
     * @return data из ответа getInfo
     * @throws Exception Ошибка сети или нет доступа к файлу
     */
    private JSONObject fetchFileInfo(String id) throws Exception {
        HttpsURLConnection connection=HalChatFunctionsLib.getHTTPSRequest("https://haldrive.halwarsing.net/api?req=getInfo&id="+id,codeUser);
        connection.setConnectTimeout(CONNECT_TIMEOUT_MS);
        connection.setReadTimeout(READ_TIMEOUT_MS);
        connection.connect();
        if(connection.getResponseCode()!=HttpURLConnection.HTTP_OK) {
            throw new IOException("Server returned HTTP "+connection.getResponseCode()+" "+connection.getResponseMessage());
        }
        JSONObject jsonObject=new JSONObject(HalChatFunctionsLib.getStringFromConnection(connection));
        if(jsonObject.getInt("errorCode")!=0) {
            throw new IOException("Error getInfo: "+jsonObject.getInt("errorCode")+";"+jsonObject.getString("error"));
        }
        JSONObject data=jsonObject.getJSONObject("data");
        saveFileInfo(id,data);
        return data;
    }

    /**
     * Добавляет или обновляет информацию о файле в `files`
     *
     * @param id ID файла
     * @param data data из ответа getInfo
     * @return void
     * @throws JSONException Нет нужного поля в data
     */
    private synchronized void saveFileInfo(String id,JSONObject data) throws JSONException {
        String[] values=new String[]{data.getString("path"),data.getString("name"),String.valueOf(data.getLong("fromId")),
                String.valueOf(data.getLong("updated")),String.valueOf(data.getInt("isFolder")),data.getString("mimeType"),
                data.getString("fileType"),data.getString("mimeData"),String.valueOf(data.optLong("size",-1)),id};
        if(isFileExists(id)) {
            db.execSQL("UPDATE `files` SET `path`=?, `name`=?, `fromId`=?, `updated`=?, `isFolder`=?, `mimeType`=?, `fileType`=?, `mimeData`=?, `size`=? WHERE `id`=?",values);
        } else {
            db.execSQL("INSERT INTO `files` (`path`, `name`, `fromId`, `updated`, `isFolder`, `mimeType`, `fileType`, `mimeData`, `size`, `id`) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",values);
        }
    }

    /**
     * Информация о файле без скачивания содержимого (имя, тип, размер, размеры картинки)
     *
     * @param id ID файла
     * @return Future с HDFile из базы. getInfo запрашивается, если записи нет или у нескачанного файла неизвестен размер (записи до v20)
     */
    public CompletableFuture<HDFile> getFileInfo(String id) {
        HDFile file=getHDFileById(id);
        if(file!=null&&file.updated!=-1&&(file.size!=-1||isDownloaded(id)))return CompletableFuture.completedFuture(file);
        return TaskExecutorManager.getInstance().submitDownload("fileInfo:"+id,()->{
            try {
                fetchFileInfo(id);
            } catch(Exception e) {
                // Без сети показываем то, что уже есть в базе
                if(file==null||file.updated==-1)throw e;
                Log.w(TAG,"getInfo failed, using cached info "+id,e);
            }
            return getHDFileById(id);
        });
    }

    /**
     * Скачивает файл в HalDriveFiles/id.hdf через .part, с прогрессом и отменой
     *
     * @param id ID файла
     * @return Скачанный файл
     * @throws Exception Ошибка сети, нет доступа, файл скачан не полностью или загрузка отменена
     */
    private File downloadFileSync(String id) throws Exception {
        if(cancelledDownloads.contains(id))throw new CancellationException("Download cancelled "+id);
        fetchFileInfo(id);

        HttpsURLConnection connection=HalChatFunctionsLib.getHTTPSRequest("https://haldrive.halwarsing.net/file/"+id,codeUser);
        connection.setConnectTimeout(CONNECT_TIMEOUT_MS);
        connection.setReadTimeout(READ_TIMEOUT_MS);
        // Без gzip: иначе Android скрывает Content-Length и прогресс не посчитать
        connection.setRequestProperty("Accept-Encoding","identity");
        connection.connect();
        if(connection.getResponseCode()!=HttpURLConnection.HTTP_OK) {
            throw new IOException("Server returned HTTP "+connection.getResponseCode()+" "+connection.getResponseMessage());
        }

        File saveFile=new File(directory,id+".hdf");
        File partFile=new File(directory,id+".hdf.part");
        try(InputStream inputStream=new BufferedInputStream(connection.getInputStream());
            FileOutputStream outputStream=new FileOutputStream(partFile)) {

            byte[] buffer=new byte[65536];
            int bytesRead;
            long loaded=0;
            long lastProgress=0;
            long total=connection.getContentLengthLong();
            while((bytesRead=inputStream.read(buffer))!=-1) {
                if(cancelledDownloads.contains(id))throw new CancellationException("Download cancelled "+id);
                outputStream.write(buffer,0,bytesRead);
                loaded+=bytesRead;

                long now=System.currentTimeMillis();
                if(now-lastProgress>=100) {
                    lastProgress=now;
                    notifyProgress(id,loaded,total);
                }
            }
            notifyProgress(id,loaded,total);
            if(total>0&&loaded!=total)throw new IOException("Incomplete download "+loaded+"/"+total);
        } catch(Exception e) {
            partFile.delete();
            throw e;
        } finally {
            connection.disconnect();
        }

        boolean isReplaced=saveFile.exists();
        if(!partFile.renameTo(saveFile)) {
            partFile.delete();
            throw new IOException("Failed rename "+partFile);
        }
        if(isReplaced)clearGlideCache();
        Log.d(TAG,"HalDrive File "+id+" successfully downloaded");
        return saveFile;
    }

    /**
     * Очищает кэш Glide после замены содержимого файла с тем же id (Glide кэширует по пути файла).
     * Вызывать не из главного потока
     *
     * @return void
     */
    private void clearGlideCache() {
        new Handler(Looper.getMainLooper()).post(()->Glide.get(context).clearMemory());
        Glide.get(context).clearDiskCache();
    }

    private void notifyProgress(String id,long loaded,long total) {
        downloadProgress.put(id,new long[]{loaded,total});
        List<HDDownloadFileEvent> listeners=downloadListeners.get(id);
        if(listeners==null)return;
        float progress=total>0?loaded*100.0f/total:-1;
        for(HDDownloadFileEvent listener:listeners)listener.onProgress(progress,total,loaded);
    }

    /**
     * Запускает скачивание файла, одно на ID (повторный вызов возвращает ту же загрузку)
     *
     * @param id ID файла
     * @param isAttachment true - вложение чата (отдельный пул, не мешает аватаркам и эмодзи)
     * @return Future со скачанным файлом
     */
    private CompletableFuture<File> startDownload(String id,boolean isAttachment) {
        CompletableFuture<File> result=new CompletableFuture<>();
        CompletableFuture<File> existing=downloadPromises.putIfAbsent(id,result);
        if(existing!=null)return existing;

        cancelledDownloads.remove(id);
        Callable<File> task=()->downloadFileSync(id);
        CompletableFuture<File> download=isAttachment
                ? TaskExecutorManager.getInstance().submitFileDownload("downloadFile:"+id,task)
                : TaskExecutorManager.getInstance().submitDownload("downloadFile:"+id,task);
        download.whenComplete((file,error)->{
            downloadPromises.remove(id,result);
            downloadProgress.remove(id);
            cancelledDownloads.remove(id);
            if(error!=null) {
                Log.e(TAG,"Unable to download file "+id,error);
                result.completeExceptionally(error);
            } else {
                result.complete(file);
            }
        });
        return result;
    }

    /**
     * Скачивание с событиями прогресса. onDownloaded вызывается и при ошибке (проверяйте isDownloaded)
     *
     * @param id ID файла
     * @param isAttachment true - вложение чата
     * @param downloadFileEvent События загрузки
     * @return Future со скачанным файлом
     */
    private CompletableFuture<File> downloadWithEvent(String id,boolean isAttachment,HDDownloadFileEvent downloadFileEvent) {
        downloadListeners.computeIfAbsent(id,k->new CopyOnWriteArrayList<>()).add(downloadFileEvent);
        long[] progress=downloadProgress.get(id);
        if(progress!=null)downloadFileEvent.onProgress(progress[1]>0?progress[0]*100.0f/progress[1]:-1,progress[1],progress[0]);

        CompletableFuture<File> future=startDownload(id,isAttachment);
        future.whenComplete((file,error)->{
            List<HDDownloadFileEvent> listeners=downloadListeners.get(id);
            if(listeners!=null) {
                listeners.remove(downloadFileEvent);
                if(listeners.isEmpty())downloadListeners.remove(id,listeners);
            }
            downloadFileEvent.onDownloaded(getHDFileById(id));
        });
        return future;
    }

    protected void deleteFile(String id) {
        Cursor cursor=db.rawQuery("SELECT * FROM `files` WHERE `id`=?",new String[]{id});
        if (!cursor.moveToFirst()) {
            cursor.close();
            return;
        }

        cursor.close();

        File file=new File(directory,id+".hdf");
        if(file.exists())file.delete();

        db.execSQL("DELETE FROM `files` WHERE `id`=?",new String[]{id});
    }

    /**
     * Скачивает файл, если его ещё нет на устройстве (аватарки, документы приложения)
     *
     * @param id ID файла
     * @return Future, завершается после попытки скачивания, без ошибки
     */
    public CompletableFuture<Void> addHalDriveFile(String id) {
        if(isDownloaded(id)) {
            Log.d(TAG,"File is exists "+id);
            return CompletableFuture.completedFuture(null);
        }
        return startDownload(id,false).handle((file,error)->null);
    }

    public void uploadHDFile(String path,int publicType,int systemType,File file,String contentType,boolean isReplace) {
        String taskTag = "uploadHDFile:" + path + ":" + file.getAbsolutePath();
        TaskExecutorManager.getInstance().submitUpload(taskTag,()->{
            String idFile=uploadHDFileWithoutThread(
                    path,
                    publicType,
                    systemType,
                    file,
                    contentType,
                    isReplace
            );
            if(idFile!=null && uploadFileEvent!=null) {
                uploadFileEvent.onUpload(getHDFileById(idFile));
            }
            return null;
        });
    }

    //Публикация файла
    public String uploadHDFileWithoutThread(String path,int publicType,int systemType,File file,String contentType,boolean isReplace) {
        try {
            HttpsURLConnection connection=HalChatFunctionsLib.postHTTPSRequestWithoutData("https://haldrive.halwarsing.net/api?req=uploadFile&replace="+(isReplace?"true":"false"),codeUser);

            MultipartHelper multipart=new MultipartHelper(connection);
            multipart.addFilePart(file,contentType,"file");
            multipart.addStringPart(path,"path");
            multipart.addStringPart(String.valueOf(publicType),"publicType");
            multipart.addStringPart(String.valueOf(systemType),"systemType");
            multipart.makeRequest();

            connection.connect();
            if (connection.getResponseCode() != HttpURLConnection.HTTP_OK) {
                Log.e(TAG, "Server returned HTTP " + connection.getResponseCode() + " " + connection.getResponseMessage());
                return null;
            }

            String jsonString = HalChatFunctionsLib.getStringFromConnection(connection);
            JSONObject jsonObject = new JSONObject(jsonString);
            if(jsonObject.getInt("errorCode")==0) {
                String id=jsonObject.getJSONObject("data").getString("id");
                addUploadedFile(id,file);
                return id;
            }
        } catch (Exception e) {
            Log.e(TAG,"Failed to fetch HalDrive API",e);
        }
        return null;
    }

    public void checkUsersIconsIsDownloaded() {
        Cursor userCursor=db.rawQuery("SELECT * FROM `users`",null);
        if (userCursor.moveToFirst()) {
            do {
                Cursor cursor=db.rawQuery("SELECT * FROM `files` WHERE id=?",new String[]{userCursor.getString(3)});
                if (!cursor.moveToFirst()) {
                    Log.d(TAG,"Finded not downloaded user icon "+userCursor.getString(3));
                    addHalDriveFile(userCursor.getString(3));
                }
                cursor.close();
            } while (userCursor.moveToNext());
        }

        userCursor.close();
    }

    public CompletableFuture<File> getFileById(String id) {
        return getFileById(id, new HDDownloadFileEvent() {
            @Override
            public void onProgress(float progress, long total, long loaded) {

            }

            @Override
            public void onDownloaded(HDFile file) {

            }
        });
    }

    /**
     * Файл с устройства, при отсутствии скачивается (аватарки, эмодзи, голосовые, плагины)
     *
     * @param id ID файла
     * @param downloadFileEvent События загрузки, onDownloaded вызывается и при ошибке
     * @return Future с файлом
     */
    public CompletableFuture<File> getFileById(String id, HDDownloadFileEvent downloadFileEvent) {
        File file=getLocalFile(id);
        if(file.exists()) {
            downloadFileEvent.onDownloaded(getHDFileById(id));
            return CompletableFuture.completedFuture(file);
        }
        return downloadWithEvent(id,false,downloadFileEvent);
    }

    /**
     * Скачивает вложение чата в отдельном пуле (по нажатию или при показе в открытом чате)
     *
     * @param id ID файла
     * @param downloadFileEvent События загрузки, onDownloaded вызывается и при ошибке и отмене
     * @return Future с файлом
     */
    public CompletableFuture<File> downloadAttachment(String id, HDDownloadFileEvent downloadFileEvent) {
        File file=getLocalFile(id);
        if(file.exists()) {
            downloadFileEvent.onDownloaded(getHDFileById(id));
            return CompletableFuture.completedFuture(file);
        }
        return downloadWithEvent(id,true,downloadFileEvent);
    }

    /**
     * Отменяет скачивание файла, недокачанная часть удаляется
     *
     * @param id ID файла
     * @return void
     */
    public void cancelDownload(String id) {
        if(downloadPromises.containsKey(id))cancelledDownloads.add(id);
    }

    public boolean isDownloading(String id) {
        return downloadPromises.containsKey(id);
    }

    public boolean isDownloaded(String id) {
        return getLocalFile(id).exists();
    }

    public File getLocalFile(String id) {
        return new File(directory,id+".hdf");
    }

    /**
     * Текущий прогресс скачивания
     *
     * @param id ID файла
     * @return {loaded,total} в байтах (total=-1, если сервер не сообщил размер) или null, если не скачивается
     */
    public long[] getDownloadProgress(String id) {
        return downloadProgress.get(id);
    }

    public boolean isFileExists(String id){
        Cursor cursor=db.rawQuery("SELECT * FROM `files` WHERE `id`=?",new String[]{id});
        if(cursor.moveToFirst()) {
            cursor.close();
            return true;
        }
        cursor.close();
        return false;
    }

    public HDFile getHDFileById(String id) {
        Cursor cursor=getCursorById(id);
        HDFile file=getHDFileByCursor(cursor);
        if(cursor!=null)cursor.close();
        return file;
    }

    protected HDFile getHDFileByCursor(Cursor cursor) {
        if(cursor==null)return null;
        int sizeColumn=cursor.getColumnIndex("size");
        long size=sizeColumn==-1?-1:cursor.getLong(sizeColumn);
        try {
            return new HDFile(cursor.getLong(0),cursor.getString(1),cursor.getString(2),
                    cursor.getString(3),cursor.getLong(4),cursor.getLong(5),cursor.getInt(6)==1,
                    cursor.getString(7),cursor.getString(8),cursor.getString(9),size);
        } catch (Exception e) {
            try {
                return new HDFile(cursor.getLong(0), cursor.getString(1), cursor.getString(2),
                        cursor.getString(3), cursor.getLong(4), cursor.getLong(5), cursor.getInt(6) == 1,
                        cursor.getString(7), cursor.getString(8), "", size);
            } catch (Exception ed) {
                Log.e(TAG,"Failed HDFILE",ed);
            }
        }
        return null;
    }

    protected Cursor getCursorById(String id) {
        Cursor cursor=db.rawQuery("SELECT * FROM `files` WHERE `id`=?",new String[]{id});
        if(cursor.moveToFirst()) {
            return cursor;
        }
        cursor.close();
        return null;
    }

    /**
     * Дозапрашивает информацию о файлах, для которых её ещё нет. Содержимое не скачивается:
     * аватарки и эмодзи скачиваются при показе, вложения — в чате
     *
     * @return void
     */
    protected void downloadFiles() {
        List<String> ids=new ArrayList<>();
        Cursor cursor=db.rawQuery("SELECT `id` FROM `files` WHERE `updated`=-1",null);
        if(cursor.moveToFirst()) {
            do {
                ids.add(cursor.getString(0));
            } while (cursor.moveToNext());
        }
        cursor.close();
        for(String id:ids)getFileInfo(id);
    }

    /**
     * MIME-тип файла: по расширению имени (как Загрузки), иначе из HalDrive.
     * Сервер для docx/xlsx может отдавать application/zip, с ним Word не открывает файл
     *
     * @param hdFile Файл
     * @return MIME-тип, по умолчанию application/octet-stream
     */
    public static String getMimeType(HDFile hdFile) {
        String mimeType=HalDriveFileProvider.getMimeTypeByName(hdFile.name);
        if(!mimeType.equals("application/octet-stream"))return mimeType;
        if(hdFile.mimeType!=null&&hdFile.mimeType.contains("/"))return hdFile.mimeType;
        return mimeType;
    }

    /**
     * Имя для сохранения: имя из HalDrive без запрещённых символов, с расширением по MIME-типу, если его нет
     *
     * @param hdFile Файл
     * @return Имя файла
     */
    public static String getSaveFileName(HDFile hdFile) {
        String name=hdFile.name==null||hdFile.name.isEmpty()||hdFile.name.equals("-1")?hdFile.id:hdFile.name;
        name=name.replaceAll("[\\\\/:*?\"<>|\\x00-\\x1F]","_");
        if(name.lastIndexOf('.')<=0) {
            String ext=MimeTypeMap.getSingleton().getExtensionFromMimeType(getMimeType(hdFile));
            if(ext!=null)name+="."+ext;
        }
        return name;
    }

    /**
     * Копирует скачанный файл в Загрузки/HalChat с настоящим именем и форматом.
     * До Android 10 нужно разрешение WRITE_EXTERNAL_STORAGE
     *
     * @param hdFile Файл (уже скачанный)
     * @return content:// URI сохранённого файла для открытия
     * @throws IOException Файл не скачан или не удалось записать
     */
    public Uri saveToDownloads(HDFile hdFile) throws IOException {
        File file=getLocalFile(hdFile.id);
        if(!file.exists())throw new IOException("File is not downloaded "+hdFile.id);
        String name=getSaveFileName(hdFile);
        String mimeType=getMimeType(hdFile);

        if(Build.VERSION.SDK_INT>=Build.VERSION_CODES.Q) {
            ContentResolver resolver=context.getContentResolver();
            ContentValues values=new ContentValues();
            values.put(MediaStore.Downloads.DISPLAY_NAME,name);
            values.put(MediaStore.Downloads.MIME_TYPE,mimeType);
            values.put(MediaStore.Downloads.RELATIVE_PATH,Environment.DIRECTORY_DOWNLOADS+"/"+SAVE_DIR);
            values.put(MediaStore.Downloads.IS_PENDING,1);
            Uri uri=resolver.insert(MediaStore.Downloads.EXTERNAL_CONTENT_URI,values);
            if(uri==null)throw new IOException("MediaStore insert failed");
            try(InputStream in=new FileInputStream(file);OutputStream out=resolver.openOutputStream(uri)) {
                if(out==null)throw new IOException("MediaStore open failed");
                copyStream(in,out);
            } catch(IOException e) {
                resolver.delete(uri,null,null);
                throw e;
            }
            values.clear();
            values.put(MediaStore.Downloads.IS_PENDING,0);
            resolver.update(uri,values,null,null);
            return uri;
        }

        File dir=new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS),SAVE_DIR);
        if(!dir.exists()&&!dir.mkdirs())throw new IOException("Failed mkdirs "+dir);
        File out=getFreeFile(dir,name);
        try(InputStream in=new FileInputStream(file);OutputStream os=new FileOutputStream(out)) {
            copyStream(in,os);
        } catch(IOException e) {
            out.delete();
            throw e;
        }
        MediaScannerConnection.scanFile(context,new String[]{out.getAbsolutePath()},new String[]{mimeType},null);
        return FileProvider.getUriForFile(context,context.getPackageName()+".provider",out);
    }

    //Как MediaStore на Android 10+: "name (1).ext"
    private static File getFreeFile(File dir,String name) {
        File out=new File(dir,name);
        int dot=name.lastIndexOf('.');
        String base=dot>0?name.substring(0,dot):name;
        String ext=dot>0?name.substring(dot):"";
        for(int i=1;out.exists();i++)out=new File(dir,base+" ("+i+")"+ext);
        return out;
    }

    private static void copyStream(InputStream in,OutputStream out) throws IOException {
        byte[] buffer=new byte[65536];
        int read;
        while((read=in.read(buffer))!=-1)out.write(buffer,0,read);
    }

    protected String getFormatType(String mimeType) {
        if(mimeType.equals("folder"))return "folder";
        String out="unknown_file";
        if(mimeType.startsWith("video/")) {
            out="video";
        } else if(mimeType.startsWith("image/")) {
            out="image";
        }
        return out;
    }

    protected int getFileIcon(String format) {
        if(format.equals("folder"))return R.drawable.folder;
        if(format.equals("image"))return  R.drawable.image;
        if(format.equals("video"))return  R.drawable.video;
        if(format.equals("txt"))return    R.drawable.txt;
        return R.drawable.unknown_file;
    }

    protected void setOnUploadFile(HDUploadFileEvent uploadFileEvent) {
        this.uploadFileEvent=uploadFileEvent;
    }

    public interface HDUploadFileEvent {
        void onUpload(HDFile file);
    }

    public interface HDDownloadFileEvent {
        void onProgress(float progress, long total, long loaded);
        void onDownloaded(HDFile file);
    }
}
