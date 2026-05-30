package halwarsing.net.halchatandroid.main;

import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.util.Log;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;

import halwarsing.net.halchatandroid.type.HNUser;

//Класс для взаимодействия с пользователями HalNet
public class HalNetUsers {
    private final SQLiteDatabase db;
    private final HalDrive hd;
    private final HalChat hc;
    private static final String TAG="HNU";

    public HalNetUsers(HalChat hc,SQLiteDatabase sdb,HalDrive shd) {
        this.db=sdb;
        this.hd=shd;
        this.hc=hc;
    }

    public boolean addUser(HNUser user) {
        Cursor userCursor=db.rawQuery("SELECT * FROM `users` WHERE id=?",new String[]{String.valueOf(user.id)});
        if (!userCursor.moveToFirst()) {
            userCursor.close();
            db.execSQL("INSERT INTO `users` (`id`, `nickname`, `icon`, `isBot`) VALUES (?, ?, ?, ?)",new String[]{String.valueOf(user.id),user.nickname,user.icon,user.isBot?"1":"0"});
            hd.addHalDriveFile(user.icon);
            return true;
        }
        userCursor.close();
        return false;
    }

    protected HNUser loadUserWithoutThread(long userId) {
        try {
            JSONObject postData=new JSONObject();
            postData.put("userId",userId);
            JSONObject res=hc.chatWS.apiReq("getInfoUser",postData).get();
            try {
                if(res.getInt("errorCode")==0) {
                    Log.e(TAG,"Successful load message");
                    HNUser user=jsonInfoUserToHNUser(res);
                    addUser(user);
                    return user;
                }
            } catch (Exception e) {
                Log.e(TAG,"getInfoUser",e);
            }
        } catch (Exception e) {
            Log.e(TAG,"loadUser",e);
        }
        return null;
    }

    protected CompletableFuture<HNUser> loadUser(long userId) {
        return TaskExecutorManager.getInstance().submitCompletableUserSync("loadHNUser:id" + userId, () -> loadUserWithoutThread(userId));
    }

    protected HNUser jsonInfoUserToHNUser(JSONObject data) throws JSONException {
        return new HNUser(-1, data.getLong("id"),data.getString("nickname"),data.getString("icon"),data.getInt("isBot")==1,null);
    }

    protected Cursor getUserCursorByUserId(long userId) {
        Cursor userCursor=db.rawQuery("SELECT * FROM `users` WHERE `id`=?",new String[]{String.valueOf(userId)});
        if(userCursor.moveToFirst()) {
            return userCursor;
        }
        userCursor.close();
        return null;
    }

    protected CompletableFuture<HNUser> getUserFromCursor(Cursor userCursor) {
        if(userCursor==null)return CompletableFuture.completedFuture(null);
        CompletableFuture<HNUser> future=new CompletableFuture<>();
        hd.getFileById(userCursor.getString(3)).thenAccept(file->{
            future.complete(new HNUser(
                    userCursor.getLong(0),
                    userCursor.getLong(1),
                    userCursor.getString(2),
                    userCursor.getString(3),
                    userCursor.getInt(4)==1,
                    file.getAbsolutePath()));
        }).exceptionally(throwable -> {
            future.completeExceptionally(throwable);
            return null;
        });

        return future;
    }

    protected CompletableFuture<HNUser> getUserByUserId(long userId,boolean isLoad) throws ExecutionException, InterruptedException {
        Cursor cursor=getUserCursorByUserId(userId);
        if(cursor==null) {
            if(isLoad)return loadUser(userId);
            return CompletableFuture.completedFuture(null);
        }

        CompletableFuture<HNUser> future=new CompletableFuture<>();
        getUserFromCursor(cursor).thenAccept(user->{
            cursor.close();
            future.complete(user);
        }).exceptionally(throwable -> {
            cursor.close();
            future.completeExceptionally(throwable);
            return null;
        });
        return future;
    }

    protected CompletableFuture<HNUser> getUserByUserId(long userId) {
        try {
            return getUserByUserId(userId,true);
        } catch (ExecutionException | InterruptedException e) {
            Log.e(TAG,"getUserByUserId",e);
        }
        return null;
    }

    protected String getIconFromCursor(Cursor cursor) {
        return cursor.getString(3);
    }
}
