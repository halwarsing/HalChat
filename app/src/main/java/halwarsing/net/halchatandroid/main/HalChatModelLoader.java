package halwarsing.net.halchatandroid.main;

import android.content.Context;
import android.database.Cursor;
import android.util.Log;

import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;

import halwarsing.net.halchatandroid.halsm.LocalLLMHalSM;
import halwarsing.net.halchatandroid.type.HDFile;

public class HalChatModelLoader {
    private final HalChat hc;
    private final Context context;
    private static final String TAG="HCMLoader";
    private boolean loadedFiles=true;

    public HalChatModelLoader(HalChat hc, Context context) {
        this.hc=hc;
        this.context=context;
    }

    public Map<String,String> loadModelFiles(long aiModel, long version, Map<String,String> files, LocalLLMHalSM.AIProgressReporter aiProgress) {
        Set<Map.Entry<String,String>> entries=files.entrySet();
        int i=0;
        Map<String,String> out=new HashMap<>();

        try {
            Map.Entry<String,String> lastEntry=null;
            for(Map.Entry<String,String> entry:entries) {
                final int indexFile=i;
                loadFile(entry.getValue(), aiModel, version, entry.getKey(), (loaded, total, pct, fromCache, filename) -> aiProgress.progress("downloading",pct,loaded,total,fromCache,filename,indexFile,files.size(),"model")).get();
                out.put(entry.getKey(),new File(hc.hd.directory,entry.getValue()+".hdf").getAbsolutePath());
                i++;
                lastEntry=entry;
            }

            loadedFiles=true;
            aiProgress.progress("initializing",100.0f,0L,0L,false,lastEntry==null?"":lastEntry.getKey(),i-1,files.size(),"model");
        } catch (ExecutionException | InterruptedException e){
            Log.e(TAG,"Error loadModel",e);
        }

        return out;
    }

    private CompletableFuture<Void> loadFile(String fileId, long aiModel, long version, String filename, HCMLLoadFile hcmlLoadFile) {
        Cursor cursor=hc.db.rawQuery("SELECT * FROM `cacheAI` WHERE `aiModel`=? AND `file`=?", new String[]{String.valueOf(aiModel),fileId});
        if(cursor.moveToFirst()) {
            long uid=cursor.getLong(0);
            long aim=cursor.getLong(1);
            long ver=cursor.getLong(2);
            String fid=cursor.getString(3);

            cursor.close();

            if(version!=ver) {
                //TODO UPDATE AI
            }

            return CompletableFuture.completedFuture(null);
        }

        cursor.close();

        CompletableFuture<Void> future=new CompletableFuture<>();

        hc.hd.getFileById(fileId, new HalDrive.HDDownloadFileEvent() {
            @Override
            public void onProgress(float progress, long total, long loaded) {
                hcmlLoadFile.progress(loaded,total,progress,false,filename);
            }

            @Override
            public void onDownloaded(HDFile file) {
                if(!new File(hc.hd.directory,fileId+".hdf").exists()) {
                    future.completeExceptionally(new IOException("Model file not downloaded: "+filename));
                    return;
                }
                hc.db.execSQL("INSERT OR IGNORE INTO `cacheAI` (`aiModel`, `version`, `file`) VALUES (?, ?, ?)",new String[]{String.valueOf(aiModel),String.valueOf(version),fileId});
                future.complete(null);
            }
        });

        return future;
    }

    private interface HCMLLoadFile {
        void progress(long loaded, long total, float pct, boolean fromCache, String filename);
    }
}
