package halwarsing.net.halchatandroid.main;

import android.annotation.SuppressLint;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.util.Log;

import org.json.JSONObject;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ConcurrentHashMap;

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
    private final ConcurrentHashMap<String,CompletableFuture<File>> downloadPromises=new ConcurrentHashMap<>();
    private HDUploadFileEvent uploadFileEvent;

    //Если файл был загружен с устройства, чтобы не скачивать его, добавляем сразу в базу
    public HDFile addLocalFile(String path,String name,long fromId, long updated, File file, String id, String mimeType, String fileType, String mimeData) {
        if(isFileExists(id)||!file.exists()) {
           return getHDFileById(id);
        }

        File newFile=new File(directory,id+".hdf");

        boolean isRenamed=file.renameTo(newFile);

        if(isRenamed) {
            db.execSQL("INSERT INTO `files` (`id`, `path`, `name`, `fromId`, `updated`, `mimeType`, `fileType`, `mimeData`) VALUES (?, ?, ?, ?, ?, ?, ?, ?)", new String[]{id, path, name, String.valueOf(fromId), String.valueOf(updated),mimeType,fileType,mimeData});
        } else {
            Log.e(TAG,"Rename failed");
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

    private CompletableFuture<Void> downloadFile(String id, boolean isUpdate) {
        return downloadFile(id, isUpdate, new HDDownloadFileEvent() {
            @Override
            public void onProgress(float progress, long total, long loaded) {

            }

            @Override
            public void onDownloaded(HDFile file) {

            }
        });
    }

    private CompletableFuture<Void> downloadFile(String id, boolean isUpdate, HDDownloadFileEvent downloadFileEvent) {
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
                    File partFile=new File(directory,id+".hdf.part");
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
                            FileOutputStream outputStream=new FileOutputStream(partFile)) {

                            byte[] buffer = new byte[65536];
                            int bytesRead;
                            long loaded=0;
                            long lastProgress=0;
                            long total=connection.getContentLengthLong();
                            while ((bytesRead = inputStream.read(buffer)) != -1) {
                                outputStream.write(buffer, 0, bytesRead);
                                loaded+=bytesRead;

                                long now=System.currentTimeMillis();
                                if(now-lastProgress>=100) {
                                    lastProgress=now;
                                    downloadFileEvent.onProgress(loaded*100.0f/total,total,loaded);
                                }
                            }
                            downloadFileEvent.onProgress(100.0f,total,loaded);

                            inputStream.close();
                            outputStream.close();
                            if(total>0&&loaded!=total)throw new IOException("Incomplete download "+loaded+"/"+total);
                            if(!partFile.renameTo(saveFile))throw new IOException("Failed rename "+partFile);
                            db.execSQL("UPDATE `files` SET `path`=?, `name`=?, `fromId`=?, `updated`=?, `isFolder`=?, `mimeType`=?, `fileType`=?, `mimeData`=? WHERE `id`=?",
                                    new String[]{data.getString("path"),data.getString("name"), String.valueOf(data.getLong("fromId")),
                                            String.valueOf(data.getLong("updated")), String.valueOf(data.getInt("isFolder")), data.getString("mimeType"),
                                            data.getString("fileType"), data.getString("mimeData"), id});
                            Log.e(TAG, "HalDrive File " + id + " successfully downloaded");
                        } catch (Exception e) {
                            partFile.delete();
                            Log.e(TAG,"ERROR SAVE FILE: "+id,e);
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

    //Скачивание файла
    @SuppressLint("Range")
    public CompletableFuture<Void> addHalDriveFile(String id) {
        Cursor cursor=db.rawQuery("SELECT * FROM `files` WHERE `id`=?",new String[]{id});
        if (!cursor.moveToFirst()) {
            cursor.close();
            return downloadFile(id,false);
        } else {
            if(cursor.getLong(cursor.getColumnIndex("updated"))==-1) {
                db.execSQL("DELETE FROM `files` WHERE `id`=?",new String[]{id});
                return downloadFile(id,false);
            }
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
        return getFileById(id, new HDDownloadFileEvent() {
            @Override
            public void onProgress(float progress, long total, long loaded) {

            }

            @Override
            public void onDownloaded(HDFile file) {

            }
        });
    }

    public CompletableFuture<File> getFileById(String id, HDDownloadFileEvent downloadFileEvent) {
        File file=new File(directory,id+".hdf");

        Log.e(TAG,id+";"+file.exists());
        if(file.exists()) {
            downloadFileEvent.onDownloaded(getHDFileById(id));
            return CompletableFuture.completedFuture(file);
        }

        CompletableFuture<File> result = new CompletableFuture<>();
        CompletableFuture<File> existing = downloadPromises.putIfAbsent(id, result);
        if (existing != null) {
            existing.thenAccept(f->{
                downloadFileEvent.onDownloaded(getHDFileById(id));
            });
            return existing;
        }

        boolean hasDatabaseEntry = isFileExists(id);
        downloadFile(id, hasDatabaseEntry, downloadFileEvent).whenComplete((unused, error) -> {
            File downloadedFile = new File(directory, id + ".hdf");

            if (error != null) {
                result.completeExceptionally(error);
            } else if (!downloadedFile.exists()) {
                result.completeExceptionally(
                        new IOException("HalDrive file was not downloaded: " + id)
                );
            } else {
                Log.e(TAG, "Файл скачался: " + id);
                result.complete(downloadedFile);
            }

            downloadFileEvent.onDownloaded(getHDFileById(id));

            downloadPromises.remove(id, result);
        });
        return result;
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
        try {
            return new HDFile(cursor.getLong(0),cursor.getString(1),cursor.getString(2),
                    cursor.getString(3),cursor.getLong(4),cursor.getLong(5),cursor.getInt(6)==1,
                    cursor.getString(7),cursor.getString(8),cursor.getString(9));
        } catch (Exception e) {
            try {
                return new HDFile(cursor.getLong(0), cursor.getString(1), cursor.getString(2),
                        cursor.getString(3), cursor.getLong(4), cursor.getLong(5), cursor.getInt(6) == 1,
                        cursor.getString(7), cursor.getString(8), "");
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

    public interface HDDownloadFileEvent {
        void onProgress(float progress, long total, long loaded);
        void onDownloaded(HDFile file);
    }
}
