package halwarsing.net.halchatandroid.main;

import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.util.Log;

import org.json.JSONObject;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.concurrent.CompletableFuture;

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
    public File directory;
    private List<HDDownloadOrder> orderDownload=new ArrayList<>();
    private List<HDUploadOrder> orderUpload=new ArrayList<>();
    private HashMap<String,CompletableFuture<File>> downloadPromises=new HashMap<>();
    private HDUploadFileEvent uploadFileEvent;


    private static class HDDownloadOrder {
        protected String id;
        protected boolean isUpdate;
        public HDDownloadOrder(String id,boolean isUpdate) {
            this.id=id;
            this.isUpdate=isUpdate;
        }
    }

    private static class HDUploadOrder {
        protected String path,contentType;
        int publicType,systemType;
        File file;
        boolean isReplace;

        public HDUploadOrder(String path,int publicType,int systemType,File file,String contentType,boolean isReplace) {
            this.path=path;
            this.publicType=publicType;
            this.systemType=systemType;
            this.file=file;
            this.contentType=contentType;
            this.isReplace=isReplace;
        }
    }

    public HalDrive(Context scontext, SQLiteDatabase sdb, String scodeUser) {
        context=scontext;
        db=sdb;
        codeUser=scodeUser;
        directory=new File(scontext.getExternalFilesDir(null),PATH_HD_FILES);


        directory.mkdirs();
    }

    private CompletableFuture<Void> downloadFile(String id, boolean isUpdate) {
        if(!isUpdate) {
            db.execSQL("INSERT INTO `files` (`id`, `path`, `name`, `fromId`, `updated`) VALUES (?, '-1', '-1', -1, -1)", new String[]{id});
        }

        return TaskExecutorManager.getInstance().submitDownload("downloadFile:"+id, () -> {
            try {
                HttpsURLConnection connection = HalChatFunctionsLib.getHTTPSRequest("https://haldrive.halwarsing.net/api?req=getInfo&id=" + id, codeUser);
                connection.connect();
                if (connection.getResponseCode() != HttpURLConnection.HTTP_OK) {
                    Log.e(TAG, "Server returned HTTP " + connection.getResponseCode() + " " + connection.getResponseMessage());
                    return null;
                }

                String jsonString = HalChatFunctionsLib.getStringFromConnection(connection);
                JSONObject jsonObject = new JSONObject(jsonString);
                if (jsonObject.getInt("errorCode") == 0) {
                    JSONObject data = jsonObject.getJSONObject("data");
                    connection = HalChatFunctionsLib.getHTTPSRequest("https://haldrive.halwarsing.net/file/" + id, codeUser);
                    connection.connect();

                    File saveFile=new File(directory,id+".hdf");
                    //Save file


                    Cursor cursors;
                    if(isUpdate){
                        cursors=db.rawQuery("SELECT * FROM `files` WHERE `id`=?", new String[]{id});
                    } else {
                        cursors=db.rawQuery("SELECT * FROM `files` WHERE `id`=? AND `updated`=-1", new String[]{id});
                    }

                    boolean isEx=cursors.moveToFirst();
                    cursors.close();

                    if (isEx) {
                        try(InputStream inputStream=new BufferedInputStream(connection.getInputStream());
                            FileOutputStream outputStream=new FileOutputStream(saveFile)) {

                            byte[] buffer = new byte[4096];
                            int bytesRead;
                            while ((bytesRead = inputStream.read(buffer)) != -1) {
                                outputStream.write(buffer, 0, bytesRead);
                            }
                            inputStream.close();
                            outputStream.close();
                            db.execSQL("UPDATE `files` SET `path`=?, `name`=?, `fromId`=?, `updated`=?, `isFolder`=? WHERE `id`=?", new String[]{data.getString("path"),data.getString("name"), String.valueOf(data.getLong("fromId")), String.valueOf(data.getLong("updated")), String.valueOf(data.getInt("isFolder")), id});
                            Log.e(TAG, "HalDrive File " + id + " successfully downloaded");
                        } catch (Exception e) {
                            Log.e(TAG,"ERROR SAVE FILE: ",e);
                        }
                    } else {
                        Log.e(TAG, "HalDrive File is already existing: " + id);
                    }
                } else {
                    Log.d(TAG, "Error getInfo: " + jsonObject.getInt("errorCode") + ";" + jsonObject.getString("error"));
                }
            } catch (Exception e) {
                Log.e(TAG, "Failed to fetch HalDrive API", e);
            }

            return null;
        });
    }

    //Скачивание файла
    public CompletableFuture<Void> addHalDriveFile(String id) {
        Cursor cursor=db.rawQuery("SELECT * FROM `files` WHERE `id`=?",new String[]{id});
        if (!cursor.moveToFirst()) {
            cursor.close();
            return downloadFile(id,false);
        } else {
            cursor.close();
            Log.d(TAG,"File is exists "+ id);
        }
        return CompletableFuture.completedFuture(null);
    }

    public CompletableFuture<Void> updateHalDriveFile(String id) {
        Cursor cursor=db.rawQuery("SELECT * FROM `files` WHERE `id`=?",new String[]{id});
        if (!cursor.moveToFirst()) {
            cursor.close();
            return downloadFile(id,false);
        } else {
            cursor.close();
            return downloadFile(id,true);
        }
    }

    public void uploadHDFile(String path,int publicType,int systemType,File file,String contentType,boolean isReplace) {
        orderUpload.add(new HDUploadOrder(path,publicType,systemType,file,contentType,isReplace));
        if(orderUpload.size()==1) {
            TaskExecutorManager.getInstance().submitUpload("uploadHDFile",()->{
                HDUploadOrder uo=orderUpload.get(0);
                String idFile=uploadHDFileWithoutThread(uo.path,uo.publicType,uo.systemType,uo.file,uo.contentType,uo.isReplace);
                if(idFile!=null) {
                    uploadFileEvent.onUpload(getHDFileById(idFile));
                }
                orderUpload.remove(0);
                return null;
            });
        }
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
                updateHalDriveFile(jsonObject.getJSONObject("data").getString("id"));
                return jsonObject.getJSONObject("data").getString("id");
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
        File file=new File(directory,id+".hdf");

        Log.e(TAG,id+";"+String.valueOf(file.exists()));
        if(!file.exists()) {
            if(downloadPromises.containsKey(id)) {
                return downloadPromises.get(id);
            }

            CompletableFuture<File> future=new CompletableFuture<>();

            downloadPromises.put(id,future);

            downloadFile(id,false).thenAccept(v->{
                future.complete(new File(directory,id+".hdf"));
                Log.e(TAG,"Файл скачался: "+id);
                downloadPromises.remove(id);
            }).exceptionally(throwable -> {future.completeExceptionally(throwable);return null;});
            return future;
        }
        return CompletableFuture.completedFuture(file);
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
        cursor.close();
        return file;
    }

    protected HDFile getHDFileByCursor(Cursor cursor) {
        if(cursor==null)return null;
        return new HDFile(cursor.getLong(0),cursor.getString(1),cursor.getString(2),
                cursor.getString(3),cursor.getLong(4),cursor.getLong(5),cursor.getInt(6)==1);
    }

    protected Cursor getCursorById(String id) {
        Cursor cursor=db.rawQuery("SELECT * FROM `files` WHERE `id`=?",new String[]{id});
        if(cursor.moveToFirst()) {
            return cursor;
        }
        cursor.close();
        return null;
    }

    protected void downloadFiles() {
        Cursor cursor=db.rawQuery("SELECT * FROM `files` WHERE `updated`=-1",null);
        if(cursor.moveToFirst()) {
            do {
                downloadFile(cursor.getString(1),true);
            } while (cursor.moveToNext());
        }
        cursor.close();
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
}
