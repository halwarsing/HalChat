package halwarsing.net.halchatandroid.main;

import android.annotation.SuppressLint;
import android.content.ContentValues;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ConcurrentHashMap;

import halwarsing.net.halchatandroid.type.Emoji;
import halwarsing.net.halchatandroid.type.EmojiPack;
import halwarsing.net.halchatandroid.type.Pixel;
import halwarsing.net.halchatandroid.type.PixelsPack;

//Система синхронзизации эмодзи и пикселей, а также быстрый доступ из всей системы к ним

//TODO если пак был удалён, проверить и удалить
public class EmojiPixelSystem {
    private static final String TAG="EPSys";
    private SQLiteDatabase db;
    private HalChat hc;
    private final ConcurrentHashMap<Long,CompletableFuture<Emoji>> pendingEmojiLoads=
            new ConcurrentHashMap<>();

    public EmojiPixelSystem(SQLiteDatabase db, HalChat hc) {
        this.db=db;
        this.hc=hc;
    }

    protected void sync() {
        hc.hcapi.apiReq("getListEmoji",new JSONObject()).thenAccept(data->{
            try {
                if(data.getLong("errorCode")==0) {
                    JSONArray packs=data.getJSONArray("data");

                    for(int i=0;i<packs.length();i++) {
                        EmojiPack pack=emojiPackFromJson(packs.getJSONObject(i));

                        addNewEmojiPack(pack);
                    }
                } else {
                    Log.e(TAG,"getListEmojiError: "+data.getLong("errorCode")+";"+data.getString("error"));
                }
            } catch (JSONException e) {
                Log.e(TAG,"getListEmoji",e);
            }
        });


        hc.hcapi.apiReq("getListPixelPacks",new JSONObject()).thenAccept(data->{
            try {
                if(data.getLong("errorCode")==0) {
                    JSONArray packs=data.getJSONArray("packs");

                    for(int i=0;i<packs.length();i++) {
                        PixelsPack pack=pixelsPackFromJson(packs.getJSONObject(i));

                        addNewPixelsPack(pack);
                    }
                } else {
                    Log.e(TAG,"getListEmojiError: "+data.getLong("errorCode")+";"+data.getString("error"));
                }
            } catch (JSONException e) {
                Log.e(TAG,"getListEmoji",e);
            }
        });
    }

    //Emoji
    protected EmojiPack emojiPackFromJson(JSONObject jsonData) {
        long packId;
        String name,icon;
        try {
            packId=jsonData.getLong("id");
            name=jsonData.getString("name");
            icon=jsonData.getString("icon");
            return new EmojiPack(-1,packId,name,icon);
        } catch (JSONException e) {
            Log.e(TAG,"EmojiPackFromJson",e);
        }
        return null;
    }

    @SuppressLint("Range")
    protected EmojiPack emojiPackFromCursor(Cursor cursor) {
        long uid,packId;
        String name,icon;
        uid=cursor.getLong(cursor.getColumnIndex("uid"));
        packId=cursor.getLong(cursor.getColumnIndex("packId"));
        name=cursor.getString(cursor.getColumnIndex("name"));
        icon=cursor.getString(cursor.getColumnIndex("icon"));
        return new EmojiPack(uid,packId,icon,name);
    }


    protected Emoji emojiFromJson(JSONObject jsonData,long fromPack) {
        long emojiId;
        String value,image;
        try {
            emojiId=jsonData.getLong("id");
            value=jsonData.getString("value");
            image=jsonData.getString("image");
            //TODO IMAGE64
            return new Emoji(-1,emojiId,fromPack,image,image,value);
        } catch (JSONException e) {
            Log.e(TAG,"PixelsPackFromJson",e);
        }
        return null;
    }

    @SuppressLint("Range")
    protected Emoji emojiFromCursor(Cursor cursor) {
        long uid,emojiId,fromPack;
        String value,image,image64;
        uid=cursor.getLong(cursor.getColumnIndex("uid"));
        emojiId=cursor.getLong(cursor.getColumnIndex("emojiId"));
        fromPack=cursor.getLong(cursor.getColumnIndex("fromPack"));
        value=cursor.getString(cursor.getColumnIndex("value"));
        image=cursor.getString(cursor.getColumnIndex("image"));
        image64=cursor.getString(cursor.getColumnIndex("image64"));
        return new Emoji(uid,emojiId,fromPack,image,image64,value);
    }

    protected void addNewEmojiPack(EmojiPack emojiPack) {
        ContentValues values = new ContentValues();
        values.put("packId", emojiPack.packId);
        values.put("icon", emojiPack.icon);
        values.put("name", emojiPack.name);

        db.insertWithOnConflict(
                "emoji_packs",
                null,
                values,
                SQLiteDatabase.CONFLICT_IGNORE
        );

        hc.hd.getFileById(emojiPack.icon);

        //download pixels
        JSONObject postData=new JSONObject();
        try {
            postData.put("packId",emojiPack.packId);
            hc.hcapi.apiReq("getEmojiPack",postData).thenAccept(data->{
                try {
                    if(data.getLong("errorCode")==0) {
                        JSONArray emojis=data.getJSONArray("emoji");

                        for(int i=0;i<emojis.length();i++) {
                            Emoji emoji=emojiFromJson(emojis.getJSONObject(i), emojiPack.packId);
                            addNewEmoji(emoji);
                        }
                    } else {
                        Log.e(TAG,"getEmojiPackError: "+data.getLong("errorCode")+";"+data.getString("error"));
                    }
                } catch (JSONException e) {
                    Log.e(TAG,"getEmojiPack",e);
                }
            });
        } catch (JSONException e) {
            Log.e(TAG,"addNewPixelsPack",e);
        }
    }

    protected void addNewEmoji(Emoji emoji) {
        ContentValues values = new ContentValues();
        values.put("emojiId", emoji.emojiId);
        values.put("value",emoji.value);
        values.put("fromPack",emoji.fromPack);
        values.put("image",emoji.image);
        values.put("image64",emoji.image64);

        db.insertWithOnConflict(
                "emoji",
                null,
                values,
                SQLiteDatabase.CONFLICT_IGNORE
        );

        hc.hd.getFileById(emoji.image);
        hc.hd.getFileById(emoji.image64);
    }

    protected List<Emoji> getListEmoji() {
        List<Emoji> out=new ArrayList<>();
        Cursor cursor=db.rawQuery("SELECT * FROM `emoji` ORDER BY `emojiId` ASC",null);

        if(cursor.moveToFirst()) {
            do {
                out.add(emojiFromCursor(cursor));
            } while (cursor.moveToNext());
        }

        cursor.close();

        return out;
    }

    public Emoji getEmojiById(long emojiId) {
        Emoji out=null;
        Cursor cursor=db.rawQuery("SELECT * FROM `emoji` WHERE `emojiId`=?",new String[]{String.valueOf(emojiId)});

        if(cursor.moveToFirst()) {
            out=emojiFromCursor(cursor);
        }

        cursor.close();
        return out;
    }

    public CompletableFuture<Emoji> getEmojiByIdAsync(long emojiId) {
        Emoji cached=getEmojiById(emojiId);
        if(cached!=null) {
            return CompletableFuture.completedFuture(cached);
        }

        CompletableFuture<Emoji> newFuture=new CompletableFuture<>();
        CompletableFuture<Emoji> existing=pendingEmojiLoads.putIfAbsent(emojiId,newFuture);
        if(existing!=null) {
            return existing;
        }

        try {
            JSONObject postData=new JSONObject();
            postData.put("id",emojiId);
            hc.hcapi.apiReq("getEmojiPackByEmoji",postData).thenAccept(data->{
                try {
                    if(data.optLong("errorCode",-1)!=0) {
                        newFuture.complete(null);
                        return;
                    }

                    long packId=data.getLong("packId");
                    JSONArray emojis=data.getJSONArray("emoji");
                    for(int i=0;i<emojis.length();i++) {
                        addNewEmoji(emojiFromJson(emojis.getJSONObject(i),packId));
                    }
                    newFuture.complete(getEmojiById(emojiId));
                } catch(Exception error) {
                    newFuture.completeExceptionally(error);
                }
            }).exceptionally(error->{
                newFuture.completeExceptionally(error);
                return null;
            });
        } catch(JSONException error) {
            newFuture.completeExceptionally(error);
        }

        newFuture.whenComplete((emoji,error)->pendingEmojiLoads.remove(emojiId,newFuture));
        return newFuture;
    }

    //Pixels
    protected PixelsPack pixelsPackFromJson(JSONObject jsonData) {
        long packId;
        String name,icon;
        try {
            packId=jsonData.getLong("uid");
            name=jsonData.getString("name");
            icon=jsonData.getString("icon");
            return new PixelsPack(-1,packId,name,icon);
        } catch (JSONException e) {
            Log.e(TAG,"PixelsPackFromJson",e);
        }
        return null;
    }

    @SuppressLint("Range")
    protected PixelsPack pixelsPackFromCursor(Cursor cursor) {
        long uid,packId;
        String name,icon;
        uid=cursor.getLong(cursor.getColumnIndex("uid"));
        packId=cursor.getLong(cursor.getColumnIndex("packId"));
        name=cursor.getString(cursor.getColumnIndex("name"));
        icon=cursor.getString(cursor.getColumnIndex("icon"));
        return new PixelsPack(uid,packId,name,icon);
    }


    protected Pixel pixelFromJson(JSONObject jsonData,long fromPack) {
        long pixelId;
        String value,image;
        try {
            pixelId=jsonData.getLong("uid");
            value=jsonData.getString("value");
            image=jsonData.getString("image");
            return new Pixel(-1,pixelId,value,fromPack,image);
        } catch (JSONException e) {
            Log.e(TAG,"PixelsPackFromJson",e);
        }
        return null;
    }

    @SuppressLint("Range")
    protected Pixel pixelFromCursor(Cursor cursor) {
        long uid,pixelId,fromPack;
        String value,image;
        uid=cursor.getLong(cursor.getColumnIndex("uid"));
        pixelId=cursor.getLong(cursor.getColumnIndex("pixelId"));
        fromPack=cursor.getLong(cursor.getColumnIndex("fromPack"));
        value=cursor.getString(cursor.getColumnIndex("value"));
        image=cursor.getString(cursor.getColumnIndex("image"));
        return new Pixel(uid,pixelId,value,fromPack,image);
    }

    protected void addNewPixelsPack(PixelsPack pixelsPack) {
        ContentValues values = new ContentValues();
        values.put("packId", pixelsPack.packId);
        values.put("icon",   pixelsPack.icon);
        values.put("name",   pixelsPack.name);

        db.insertWithOnConflict(
                "pixels_packs",
                null,
                values,
                SQLiteDatabase.CONFLICT_IGNORE
        );

        hc.hd.getFileById(pixelsPack.icon);

        //download pixels
        JSONObject postData=new JSONObject();
        try {
            postData.put("packId",pixelsPack.packId);
            hc.hcapi.apiReq("getPixelPack",postData).thenAccept(data->{
                try {
                    if(data.getLong("errorCode")==0) {
                        JSONObject pack=data.getJSONObject("pack");
                        JSONArray pixels=pack.getJSONArray("pixels");

                        for(int i=0;i<pixels.length();i++) {
                            Pixel pixel=pixelFromJson(pixels.getJSONObject(i), pixelsPack.packId);
                            addNewPixel(pixel);
                        }
                    } else {
                        Log.e(TAG,"getPixelPackError: "+data.getLong("errorCode")+";"+data.getString("error"));
                    }
                } catch (JSONException e) {
                    Log.e(TAG,"getPixelPack",e);
                }
            });
        } catch (JSONException e) {
            Log.e(TAG,"addNewPixelsPack",e);
        }
    }

    protected void addNewPixel(Pixel pixel) {
        ContentValues values = new ContentValues();
        values.put("pixelId", pixel.pixelId);
        values.put("value",pixel.value);
        values.put("fromPack",pixel.fromPack);
        values.put("image",pixel.image);

        db.insertWithOnConflict(
                "pixels",
                null,
                values,
                SQLiteDatabase.CONFLICT_IGNORE
        );

        hc.hd.getFileById(pixel.image);
    }

    protected List<Pixel> getListPixel() {
        List<Pixel> out=new ArrayList<>();
        Cursor cursor=db.rawQuery("SELECT * FROM `pixels` ORDER BY `pixelId` ASC",null);

        if(cursor.moveToFirst()) {
            do {
                out.add(pixelFromCursor(cursor));
            } while (cursor.moveToNext());
        }

        cursor.close();

        return out;
    }

    public Pixel getPixelById(long pixelId) {
        Pixel out=null;
        Cursor cursor=db.rawQuery("SELECT * FROM `pixels` WHERE `pixelId`=?",new String[]{String.valueOf(pixelId)});

        if(cursor.moveToFirst()) {
            out=pixelFromCursor(cursor);
        }

        cursor.close();
        return out;
    }

}
