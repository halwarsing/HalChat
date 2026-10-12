package halwarsing.net.halchatandroid.type;

import android.database.Cursor;

import halwarsing.net.halchatandroid.main.HalChatFunctionsLib;

public class AILoRA {
    public long uid, loraId, size, ai;
    public String name, description, file;

    //AILoRA      - дополнительные/модифицирующие веса основной ИИ модели, можно добавлять новые знания или удалять защиту
    //uid         - внутренний UID в БД
    //loraId      - внешний UID
    //name        - наименование
    //description - описание
    //file        - файл LoRA
    //size        - размер в байтах
    //ai          - для какой ИИ модели

    public AILoRA(long uid, long loraId, String name, String description, String file, long size, long ai) {
        this.uid=uid;
        this.loraId=loraId;
        this.name=name;
        this.description=description;
        this.file=file;
        this.size=size;
        this.ai=ai;
    }

    public static AILoRA fromCursor(Cursor cursor) {
        long uid=HalChatFunctionsLib.getLongCursor(cursor,"uid");
        long loraId=HalChatFunctionsLib.getLongCursor(cursor,"loraId");
        String name=HalChatFunctionsLib.getStringCursor(cursor,"name");
        String description=HalChatFunctionsLib.getStringCursor(cursor,"description");
        String file=HalChatFunctionsLib.getStringCursor(cursor,"file");
        long size=HalChatFunctionsLib.getLongCursor(cursor,"size");
        long ai=HalChatFunctionsLib.getLongCursor(cursor,"ai");
        return new AILoRA(uid,loraId,name,description,file,size,ai);
    }
}
