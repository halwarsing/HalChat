package halwarsing.net.halchatandroid.main;

import android.database.Cursor;

public class HalChatSettingsApp {
    private final HalChat hc;
    protected static final String KEY_TRANSFER_PASSWORDS="trpsw";
    protected static final String KEY_MUTE_NOTIFICATIONS="mute";

    public HalChatSettingsApp(HalChat hc) {
        this.hc=hc;
        addDefaultSettingsUser();
    }

    //DEFAULT APP SETTINGS

    protected void addDefaultSettingsUser() {
        addParameter(KEY_TRANSFER_PASSWORDS,true,false);
        addParameter(KEY_MUTE_NOTIFICATIONS,false,false);
    }

    //ADD & SET

    private boolean addParameter(String key, String value, boolean replace) {
        if(hasParameter(key)) {
            if(replace) {
                return setParameter(key, value);
            }
            return false;
        }
        hc.db.execSQL("INSERT INTO `SettingsApp` (`fromId`, `key`, `value`) VALUES(?, ?, ?)",new String[]{String.valueOf(hc.idUser),key,value});
        return true;
    }

    private boolean addParameter(String key, boolean value, boolean replace) {
        return addParameter(key,value?"1":"0",replace);
    }

    //SET

    protected boolean setParameter(String key,String value) {
        Cursor cursor=hc.db.rawQuery("SELECT * FROM `SettingsApp` WHERE `key`=? AND `isDelete`=0 AND `fromId`=?",new String[]{key,String.valueOf(hc.idUser)});
        if(cursor.moveToFirst()) {
            hc.db.execSQL("UPDATE `SettingsApp` SET `value`=? WHERE `uid`=?",new String[]{value,String.valueOf(cursor.getLong(0))});
            cursor.close();
            return true;
        }
        cursor.close();
        return false;
    }

    protected boolean setParameter(String key, boolean value) {
        return setParameter(key,value?"1":"0");
    }


    //CHECK
    protected boolean hasParameter(String key) {
        Cursor cursor=hc.db.rawQuery("SELECT * FROM `SettingsApp` WHERE `key`=? AND `isDelete`=0 AND `fromId`=?",new String[]{key,String.valueOf(hc.idUser)});
        boolean out=cursor.moveToFirst();
        cursor.close();
        return out;
    }



    //GET
    protected String getParameter(String key) {
        Cursor cursor=hc.db.rawQuery("SELECT * FROM `SettingsApp` WHERE `key`=? AND `isDelete`=0 AND `fromId`=?",new String[]{key,String.valueOf(hc.idUser)});
        if(cursor.moveToFirst()) {
            String out=cursor.getString(3);
            cursor.close();
            return out;
        }
        cursor.close();
        return null;
    }

    protected boolean getBoolean(String key) {
        return getParameter(key).equals("1");
    }
}
