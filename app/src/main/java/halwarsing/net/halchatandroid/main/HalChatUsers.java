package halwarsing.net.halchatandroid.main;

import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CompletableFuture;
import java.util.stream.Collectors;

import halwarsing.net.halchatandroid.type.HCChat;
import halwarsing.net.halchatandroid.type.HCUser;
import halwarsing.net.halchatandroid.type.HNUser;

//Класс для взаимодействия с пользователями чатов
public class HalChatUsers {
    private final SQLiteDatabase db;
    private HalChat hc;
    private static final String TAG="HCU";

    public HalChatUsers(SQLiteDatabase sdb,HalChat hc) {
        db=sdb;this.hc=hc;
    }

    public boolean addUser(HCUser user) {
        Cursor userCursor=db.rawQuery("SELECT * FROM `groupChatsUsers` WHERE id=?",new String[]{String.valueOf(user.id)});
        if (!userCursor.moveToFirst()) {
            userCursor.close();
            db.execSQL("INSERT INTO `groupChatsUsers` (`id`, `chatId`, `toId`, `permissions`, `isJoin`) VALUES (?, ?, ?, ?, ?)",new String[]{String.valueOf(user.id),String.valueOf(user.chatId),String.valueOf(user.toId),String.valueOf(user.permissions),user.isJoin?"1":"0"});
            return true;
        }
        userCursor.close();
        return false;
    }

    public CompletableFuture<HCUser> getChatUser(long chatId,long userId) {
        CompletableFuture<HCUser> future=new CompletableFuture<>();
        try {
            JSONObject postData=new JSONObject();
            postData.put("chatId",chatId);
            postData.put("userId",userId);
            hc.chatWS.apiReq("getChatUser",postData).thenAccept(res->{
                try {
                    if(res.getInt("errorCode")==0) {
                        HNUser hnuser=hc.hnUsers.getUserByUserId(userId).get();
                        HCUser hcuser=getChatUserFromJson(res.getJSONObject("user"),chatId,hnuser);
                        future.complete(hcuser);
                    } else {
                        future.complete(null);
                    }
                } catch (Exception e) {
                    Log.e(TAG,"getChatUserWS",e);
                    future.completeExceptionally(e);
                }
            });
        } catch (Exception e) {
            Log.e(TAG,"getChatUser",e);
            future.completeExceptionally(e);
        }
        return future;
    }

    public CompletableFuture<List<HCUser>> getChatUsers(long chatId) {
        /*Cursor userCur=db.rawQuery("SELECT * FROM `groupChatsUsers` WHERE `isJoin`=1 ORDER BY `uid` DESC LIMIT 100",null);
        List<HCUser> out=new ArrayList<>();
        if (userCur!=null&&userCur.moveToFirst()) {
            do {
                out.add(getChatUserFromCursor(userCur));
            } while (userCur.moveToNext());
        }
        userCur.close();
        return out;*/
        try {
            JSONObject postData=new JSONObject();
            postData.put("chatId",chatId);
            postData.put("count",1000);
            postData.put("lastId",-1);
            return hc.chatWS.apiReq("getChatUsers",postData).thenCompose(res->{
                try {
                    if(res.getInt("errorCode")==0) {
                        JSONArray usersArr=res.getJSONArray("users");
                        List<CompletableFuture<HCUser>> futures=new ArrayList<>();

                        for(int i=0;i<usersArr.length();i++) {
                            //CompletableFuture<HNUser> f=hc.hnUsers.getUserByUserId(usersArr.getJSONObject(i).getLong("id"),true);
                            final JSONObject u=usersArr.getJSONObject(i);
                            final long userId=u.getLong("id");
                            CompletableFuture<HCUser> uf=hc.hnUsers.getUserByUserId(userId,true).thenApply(hnUser -> {
                                try {
                                    return getChatUserFromJson(u,chatId,hnUser);
                                } catch (JSONException e) {
                                    Log.e(TAG,"getUserByUserId",e);
                                }
                                return null;
                            });
                            //NAF: AddUser
                            futures.add(uf);
                        }

                        return CompletableFuture.allOf(futures.toArray(new CompletableFuture[0]))
                                .thenApply(v->
                                        futures.stream()
                                                .map(CompletableFuture::join)
                                                .collect(Collectors.toList()));
                    } else {
                        Log.e(TAG,"getChatsUsersCW: "+res);
                    }
                } catch (Exception e) {
                    Log.e(TAG,"getChatUsersCW",e);
                }
                return CompletableFuture.completedFuture(null);
            });
        } catch (Exception e) {
            Log.e(TAG,"getChatUsers",e);
        }
        return CompletableFuture.completedFuture(null);
    }

    protected HCUser getChatUserFromJson(JSONObject user,long chatId,HNUser hnUser) throws JSONException {
        return new HCUser(user.getLong("uid"),user.getLong("id"),chatId,user.getLong("id"),(byte)user.getInt("permissions"),true,hnUser);
    }
}
