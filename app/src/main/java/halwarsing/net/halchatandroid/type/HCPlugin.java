package halwarsing.net.halchatandroid.type;

import android.database.Cursor;

import org.json.JSONObject;

import halwarsing.net.halchatandroid.main.HalChatFunctionsLib;

public class HCPlugin {
    public long uid,pluginId,versionCode;
    public String name,description,icon,plugin,version;
    public int type;

    //HCPlugin    - основная информация о плагине на HalChat
    //pluginId    - UID плагина
    //name        - наименование плагина
    //description - описание плагина
    //icon        - иконка
    //plugin      - HSME скомпилированный плагин
    //type        - тип плагина:
    //                  0: обычный визуальный или добавляющий новый функционал внутри чата
    //                  1: обычный ИИ-плагин добавляет возможность предзагрузки необходимых файлов и библиотеки HalSMLocalLLM, но все настройки и предзагрузка нужных файлов лежит на плагине
    //                  2: ИИ-плагин с заданными данными под чат это такой же ИИ-плагин, но он универсален под ИИ модели на HalChat, он может быть в каждом чате настроен по разному, то есть можно использовать
    //                  разные модели, и разное поведение
    //version     - символьная версия плагина для отображения пользователям
    //versionCode - внутреннея версия для синхронизации

    public HCPlugin(long uid, long pluginId, String name, String description, String icon, String plugin, int type, String version, long versionCode) {
        this.uid=uid;
        this.pluginId=pluginId;
        this.name=name;
        this.description=description;
        this.icon=icon;
        this.plugin=plugin;
        this.type=type;
        this.version=version;
        this.versionCode=versionCode;
    }

    public static HCPlugin fromCursor(Cursor cursor) {
        long uid,pluginId,versionCode;
        String name,description,icon,plugin,version;
        int type;

        uid=HalChatFunctionsLib.getLongCursor(cursor,"uid");
        pluginId=HalChatFunctionsLib.getLongCursor(cursor,"pluginId");
        versionCode=HalChatFunctionsLib.getLongCursor(cursor,"versionCode");
        name=HalChatFunctionsLib.getStringCursor(cursor,"name");
        description=HalChatFunctionsLib.getStringCursor(cursor,"description");
        icon=HalChatFunctionsLib.getStringCursor(cursor,"icon");
        plugin=HalChatFunctionsLib.getStringCursor(cursor,"plugin");
        version=HalChatFunctionsLib.getStringCursor(cursor,"version");
        type=HalChatFunctionsLib.getIntCursor(cursor,"type");

        return new HCPlugin(uid,pluginId,name,description,icon,plugin,type,version,versionCode);
    }
}
