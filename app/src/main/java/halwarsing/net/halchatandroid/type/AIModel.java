package halwarsing.net.halchatandroid.type;

import android.database.Cursor;

import org.json.JSONArray;

import halwarsing.net.halchatandroid.main.HalChatFunctionsLib;

public class AIModel {
    public long uid, modelId, sizeFiles;
    public int countFiles,quantanization;
    public boolean isMobileSupport;
    public float countParameters;
    public String name,description;
    public JSONArray files;

    //AIModel         - ИИ модель на HalChat
    //uid             - внутренний UID в БД
    //modelId         - внешний UID
    //files           - список файлов ИИ
    //name            - наименование ИИ
    //description     - описание ИИ
    //countFiles      - список файлов
    //sizeFiles       - и общий размер файлов для быстрого расчёта времени загрузки и подготовки возможного места
    //quantanization  - квантанизация ИИ модели:
    //                      0: Q1_0
    //                      1: TERNARY
    //                      2: Q4_0
    //                      3: q4f16
    //countParameters - кол-во параметров модели в миллиардах
    //isMobileSupport  - поддерживает ли мобильные устройства

    public AIModel(long uid, long modelId, JSONArray files, String name, String description, int countFiles, long sizeFiles, int quantanization, float countParameters, boolean isMobileSupport) {
        this.uid=uid;
        this.modelId=modelId;
        this.files=files;
        this.name=name;
        this.description=description;
        this.countFiles=countFiles;
        this.sizeFiles=sizeFiles;
        this.quantanization=quantanization;
        this.countParameters=countParameters;
        this.isMobileSupport=isMobileSupport;
    }

    public static AIModel fromCursor(Cursor cursor) {
        long uid=HalChatFunctionsLib.getLongCursor(cursor,"uid");
        long modelId=HalChatFunctionsLib.getLongCursor(cursor,"modelId");
        JSONArray files=HalChatFunctionsLib.getJSONArrayCursor(cursor,"files");
        String name=HalChatFunctionsLib.getStringCursor(cursor,"name");
        String description=HalChatFunctionsLib.getStringCursor(cursor,"description");
        int countFiles=HalChatFunctionsLib.getIntCursor(cursor,"countFiles");
        long sizeFiles=HalChatFunctionsLib.getLongCursor(cursor,"sizeFiles");
        int quantanization=HalChatFunctionsLib.getIntCursor(cursor,"quantanization");
        float countParameters=HalChatFunctionsLib.getFloatCursor(cursor,"countParameters");
        boolean isMobileSupport=HalChatFunctionsLib.getBooleanCursor(cursor,"isMobileSupport");
        return new AIModel(uid,modelId,files,name,description,countFiles,sizeFiles,quantanization,countParameters,isMobileSupport);
    }
}
