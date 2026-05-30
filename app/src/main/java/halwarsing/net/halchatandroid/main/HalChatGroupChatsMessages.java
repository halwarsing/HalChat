package halwarsing.net.halchatandroid.main;

import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.util.Log;

import java.time.Instant;
import java.time.LocalDate;
import java.time.LocalDateTime;
import java.time.ZoneId;
import java.time.ZoneOffset;
import java.time.temporal.TemporalField;
import java.util.TimeZone;

//Класс для взаимодействия с сообщениями
public class HalChatGroupChatsMessages {
    private static final String TAG="HCGCM";
    private final SQLiteDatabase db;

    public HalChatGroupChatsMessages(SQLiteDatabase sdb) {
        db=sdb;
    }


    public static final String convertTime(long time) {
        LocalDateTime date=Instant.ofEpochMilli(time*1000L).atZone(TimeZone.getDefault().toZoneId()).toLocalDateTime();
        String h=String.valueOf(date.getHour());
        String m=String.valueOf(date.getMinute());
        return (h.length()==1?"0"+h:h)+":"+(m.length()==1?"0"+m:m);
    }



}
