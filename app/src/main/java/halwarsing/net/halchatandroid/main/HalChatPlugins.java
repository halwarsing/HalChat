package halwarsing.net.halchatandroid.main;

import android.annotation.SuppressLint;
import android.content.Context;
import android.database.Cursor;
import android.util.Log;

import org.json.JSONObject;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import halwarsing.net.halchatandroid.halsm.HalSMExecutable;
import halwarsing.net.halchatandroid.halsm.HalSMNative;
import halwarsing.net.halchatandroid.halsm.LocalLLMHalSM;
import halwarsing.net.halchatandroid.type.ChatPlugin;
import halwarsing.net.halchatandroid.type.HCMessage;

public class HalChatPlugins {
    public static volatile HalChatPlugins Main;
    public final long chatId;

    private static final String TAG="HCPlugins";

    public final HalChatPluginsInterface pluginsInterface;
    public final HalSMExecutable hsme;
    public final HalChat hc;
    public final Context context;
    public final String description;
    public final boolean is_start_chat;
    private final ExecutorService hsmExecutor=Executors.newSingleThreadExecutor();

    public HalChatPlugins(long chatId, HalChatPluginsInterface pluginsInterface, Context context, String description, boolean is_start_chat) {
        this.chatId=chatId;
        this.pluginsInterface=pluginsInterface;
        this.context=context;
        HalChatApp app=(HalChatApp)context;
        hc=app.getHalChat();
        hsme=new HalSMExecutable();
        this.description=description;
        this.is_start_chat=is_start_chat;

        hsme.init(new HalSMNative.Host() {
            @Override
            public void print(String text) {
                Log.d(TAG,text);
            }

            @Override
            public void printError(String text) {
                Log.e(TAG,text);
            }

            @Override
            public String input(String prompt) {
                return "";
            }
        });

        hsme.addModule("HalChat",HalChatModule.class);
        hsme.addModule("LocalLLM",LocalLLMHalSM.class);
    }

    public static synchronized HalChatPlugins open(long chatId, HalChatPluginsInterface pluginsInterface, Context context, String description, boolean is_start_chat) {
        close();
        Main=new HalChatPlugins(chatId,pluginsInterface,context,description,is_start_chat);
        return Main;
    }

    public static synchronized void close() {
        if(Main==null)return;
        LocalLLMHalSM.close();
        Main.hsmExecutor.shutdown();
        Main=null;
    }

    public static synchronized void close(HalChatPlugins plugins) {
        if(Main==plugins)close();
    }

    public void post(Runnable task) {
        if(hsmExecutor.isShutdown())return;
        hsmExecutor.execute(()->{
            try {
                task.run();
            } catch (Exception e) {
                Log.e(TAG,"Error in HalSM thread: ",e);
            }
        });
    }

    public static void runEvent(String name, List<?> args, Map<String,?> pluginTag) {
        HalChatPlugins plugins=Main;
        if(plugins==null)return;
        plugins.post(()->{
            if(plugins.hsme.isLoadedPlugins)plugins.hsme.runEvent(name,args,pluginTag);
        });
    }

    public static CompletableFuture<Void> loadPlugin(ChatPlugin chatPlugin,Object data) {
        HalChatPlugins plugins=Main;
        return plugins.hc.hd.getFileById(chatPlugin.plugin.plugin).thenCompose(filePlugin->{
            CompletableFuture<Void> future=new CompletableFuture<>();
            plugins.post(()->{
                try {
                    plugins.hsme.loadPlugin(filePlugin.getAbsolutePath(),String.valueOf(chatPlugin.plugin.pluginId),data);
                } finally {
                    future.complete(null);
                }
            });
            return future;
        });
    }

    public static CompletableFuture<Void> loadPlugin(ChatPlugin chatPlugin) {
        return loadPlugin(chatPlugin,new ArrayList<>());
    }

    public List<HCMessage> getLastMessages(int count) {
        return hc.chatGroupChats.getChatLastMessages(chatId,-1,count,true);
    }

    public void setStorageData(String pluginId, String key, String value) {
        Cursor cursor=hc.db.rawQuery("SELECT * FROM `storagePlugins` WHERE `pluginId`=? AND `key`=?",new String[]{pluginId,key});

        if(cursor.moveToFirst()) {
            cursor.close();
            hc.db.execSQL("UPDATE `storagePlugins` SET `value`=? WHERE `pluginId`=? AND `key`=?",new String[]{value,pluginId,key});
            return;
        }

        cursor.close();
        hc.db.execSQL("INSERT INTO `storagePlugins` (`pluginId`, `key`, `value`) VALUES (?, ?, ?)",new String[]{pluginId,key,value});
    }

    public String getStorageData(String pluginId, String key) {
        Cursor cursor=hc.db.rawQuery("SELECT * FROM `storagePlugins` WHERE `pluginId`=? AND `key`=?",new String[]{pluginId,key});

        if(cursor.moveToFirst()) {
            @SuppressLint("Range") String value=cursor.getString(cursor.getColumnIndex("value"));
            cursor.close();
            return value;
        }

        cursor.close();
        return null;
    }

    public interface HalChatPluginsInterface {
        void showMessage(String text);
        void setPluginDisabledIcon(String iconUrl);
        void disableUserSend();
        void enableUserSend();
        void setMessageInput(String text);
        String getMessageInput();
        void sendMessagePlugin(String text, List<String> attachments, String pluginData, String soundMsg);
        void editMessagePlugin(long msgId, String text);
        void editLocalMessagePlugin(long msgId, String text);
        void onGenerateDone();
        void stopLoadScreen();
        void createLoadScreen(JSONObject options);
        void updateLoadScreen(JSONObject progress);
    }
}
