package halwarsing.net.halchatandroid.type;

import android.database.Cursor;

import org.json.JSONObject;

import halwarsing.net.halchatandroid.main.HalChatFunctionsLib;
import halwarsing.net.halchatandroid.main.HalChatPlugins;

public class ChatPlugin {
    public long uid,chatId;
    public HCPlugin plugin;
    public int dataType;
    public long dataId,chatPluginId;
    public Object data=null;

    //ChatPlugin   - информация о плагине внутри определённого чата
    //uid          - местный UID в БД
    //chatPluginId - UID во внешней БД для синхронизации
    //plugin       - информация о самом плагине
    //dataType     - тип предзаданных данных в конкретном чате:
    //                   0: не задано
    //                   1: Person AI
    //dataId       - UID данных
    //data         - данные, если нет, равны null

    public ChatPlugin(HCPlugin plugin, long uid, long chatPluginId, long chatId, int dataType, long dataId) {
        this.plugin=plugin;
        this.dataType=dataType;
        this.dataId=dataId;
        this.chatId=chatId;
        this.uid=uid;
        this.chatPluginId=chatPluginId;
    }

    public static ChatPlugin fromCursor(Cursor cursor, HCPlugin plugin) {
        int dataType=HalChatFunctionsLib.getIntCursor(cursor,"dataType");
        long dataId=HalChatFunctionsLib.getLongCursor(cursor,"dataId");
        long chatPluginId=HalChatFunctionsLib.getLongCursor(cursor,"chatPluginId");
        long uid=HalChatFunctionsLib.getLongCursor(cursor,"uid");
        long chatId=HalChatFunctionsLib.getLongCursor(cursor,"chatId");
        return new ChatPlugin(plugin,uid,chatPluginId,chatId,dataType,dataId);
    }
}
