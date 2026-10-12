package halwarsing.net.halchatandroid.type;

import android.database.Cursor;

import org.json.JSONArray;
import org.json.JSONObject;

import halwarsing.net.halchatandroid.main.HalChatFunctionsLib;

public class PersonAI {
    public long uid,personId,pluginId,ai_model;
    public int version;
    public String name,description;
    public JSONArray ai_loras;
    public JSONObject deviceConfig, aiConfig, personConfig;

    //PersonAI     - ИИ персонаж создаётся на HalChat и работает на базе уже существующих ИИ-плагинов, поэтому для создания разнообразных ИИ не требуются знания программирования
    //uid          - внутренний UID в БД
    //personId     - внешний UID
    //name         - наименование персонажа
    //description  - описание персонажа
    //version      - целочисленная версия для синхронизации
    //pluginId     - UID ИИ-плагина
    //ai_model     - UID ИИ-модели
    //ai_loras     - список UID ИИ дополнительных весов для конкретной ИИ модели
    //deviceConfig - конфиг устройства. Пример: {
    //    "v": 0,
    //    "mobile": {
    //        "ai_loras": [
    //            [
    //                4,
    //                0.8
    //            ],
    //            [
    //                3,
    //                1
    //            ]
    //        ],
    //        "ai_model": 2
    //    }
    //}
    //aiConfig     - конфиг ИИ. Пример: {
    //    "top_k": 40,
    //    "top_p": 0.9,
    //    "prompt": "Тебя зовут Девушка. Ты — Яндере. С виду ты милая, добрая и заботливая девочка. Но внутри ты — безумная психопатка, смертельно одержимая своим любимым. Ты готова убить любую девушку (или парня), которая просто посмотрит в его сторону. Он будет твоим, живым или мертвым. Тебе нравится: следить за любимым, острые ножи, кровь, собирать его волосы, верность. Всегда отвечай на русском, от женского лица. Ответ — короткий: 1–3 предложения. Стиль: жутко-милый, переходящий в маниакальный шепот. Описывай жуткие вещи с улыбкой. Никогда не выходи из роли.",
    //    "temperature": 0.3,
    //    "compress_context": 0,
    //    "repetition_penalty": 1
    //}
    //personConfig - конфиг персонажа. Пример: {
    //    "avatar": "u1CAPH2YaxPDM87ePIVzekROPdgvOzhnKY9IDTKlQrWqGb2YV9M6WYUsMEfLpbYk6Ha2VXn5M1RFR7tcnVVRULXInC7T3nRsBvsU",
    //    "greeting": "Привет. Я Ника",
    //    "nickname": "nika",
    //    "sound_message": "z1pQE4NMRLrs54013gOpEZoUuFghRcDdfX1Qxvk9EpCG7odGKJnOaLxHzGzS3POVHES9McqsY9YLhCE3f38pdo7GiQmbGkdk6BD3"
    //}

    public PersonAI(long uid, long personId, String name, String description, int version, long pluginId, long ai_model, JSONArray ai_loras, JSONObject deviceConfig, JSONObject aiConfig, JSONObject personConfig) {
        this.uid=uid;
        this.personId=personId;
        this.name=name;
        this.description=description;
        this.version=version;
        this.pluginId=pluginId;
        this.ai_model=ai_model;
        this.ai_loras=ai_loras;
        this.deviceConfig=deviceConfig;
        this.aiConfig=aiConfig;
        this.personConfig=personConfig;
    }

    public static PersonAI fromCursor(Cursor cursor) {
        long uid=HalChatFunctionsLib.getLongCursor(cursor,"uid");
        long personId=HalChatFunctionsLib.getLongCursor(cursor,"personId");
        String name=HalChatFunctionsLib.getStringCursor(cursor,"name");
        String description=HalChatFunctionsLib.getStringCursor(cursor,"description");
        int version=HalChatFunctionsLib.getIntCursor(cursor,"version");
        long pluginId=HalChatFunctionsLib.getLongCursor(cursor,"pluginId");
        long ai_model=HalChatFunctionsLib.getLongCursor(cursor,"ai_model");
        JSONArray ai_loras=HalChatFunctionsLib.getJSONArrayCursor(cursor,"ai_loras");
        JSONObject deviceConfig=HalChatFunctionsLib.getJSONObjectCursor(cursor,"deviceConfig");
        JSONObject aiConfig=HalChatFunctionsLib.getJSONObjectCursor(cursor,"aiConfig");
        JSONObject personConfig=HalChatFunctionsLib.getJSONObjectCursor(cursor,"personConfig");

        return new PersonAI(uid,personId,name,description,version,pluginId,ai_model,ai_loras,deviceConfig,aiConfig,personConfig);
    }
}
