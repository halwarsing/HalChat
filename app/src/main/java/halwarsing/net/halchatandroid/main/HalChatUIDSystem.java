package halwarsing.net.halchatandroid.main;

import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;

public class HalChatUIDSystem {
    private static final String TAG="HCUIDSYS";
    private final SQLiteDatabase db;
    protected static final String[] UIONames={"msg","chats","chatUsers","chatActions","actions"};

    public HalChatUIDSystem(SQLiteDatabase db) {
        this.db=db;
    }

    protected long getUID(String key) {
        long out=-2;
        Cursor cursor=db.rawQuery("SELECT * FROM `HCUID` WHERE `name`=? LIMIT 1",new String[]{key});
        if(cursor.moveToFirst()) {
            out=cursor.getLong(2);
        }
        cursor.close();
        return out;
    }

    protected void setUID(String key,long value) {
        db.execSQL("UPDATE `HCUID` SET `value`=? WHERE `name`=?",new String[]{String.valueOf(value),key});
    }
 }
