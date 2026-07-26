package halwarsing.net.halchatandroid.main;

import static halwarsing.net.halchatandroid.main.HalChatFunctionsLib.getTimeFromSeconds;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.widget.Toast;


import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;

import javax.net.ssl.HttpsURLConnection;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.Future;
import java.util.function.Function;

import halwarsing.net.halchatandroid.encryption.AESGCMHelper;
import halwarsing.net.halchatandroid.type.HCChat;
import halwarsing.net.halchatandroid.type.HCFile;
import halwarsing.net.halchatandroid.type.HCMessage;
import halwarsing.net.halchatandroid.type.HCUser;
import halwarsing.net.halchatandroid.type.HNUser;
import halwarsing.net.halchatandroid.type.Pixel;
import okhttp3.FormBody;
import okhttp3.HttpUrl;
import okhttp3.MultipartBody;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.RequestBody;


//Класс для взаимодействия с чатами
public class HalChatGroupChats {
    private static final String TAG="HCAHCGC";
    private final SQLiteDatabase db;
    private final String codeUser;
    private final long userId;
    private final HalChat hc;

    private static final ForkJoinPool cryptoPool = new ForkJoinPool(Math.max(2, Runtime.getRuntime().availableProcessors() - 1));

    private HCGCSendMessage sendMessageEvent;

    private List<HCFileOrder> orderChatFiles=new ArrayList<>();
    private HCFileUploadEvent fileUploadEvent;

    private HashMap<Long,Long> lastChatIds=new HashMap<>();

    public boolean hasPrivateChat(long toId) {
        Cursor userCur=db.rawQuery("SELECT * FROM `groupChatsUsers` WHERE `toId`=?",new String[]{String.valueOf(toId)});
        if(userCur.moveToFirst()) {
            do {
                Cursor userChat = db.rawQuery("SELECT * FROM `groupChats` WHERE `chatUID`=? AND `chatType`=1",new String[]{String.valueOf(userCur.getLong(2))});
                //TODO Добавить проверку что пользователь тоже находится в этом чате.
                if(userChat.moveToFirst()) {
                    userChat.close();
                    return true;
                }
                userChat.close();
            } while (userCur.moveToNext());
        }
        userCur.close();
        return false;
    }

    public long getPrivateChat(long toId) {
        Cursor userCur=db.rawQuery("SELECT * FROM `groupChatsUsers` WHERE `toId`=?",new String[]{String.valueOf(toId)});
        if(userCur.moveToFirst()) {
            do {
                Cursor userChat = db.rawQuery("SELECT * FROM `groupChats` WHERE `chatUID`=? AND `chatType`=1",new String[]{String.valueOf(userCur.getLong(2))});
                //Добавить проверку что пользователь тоже находится в этом чате.
                if(userChat.moveToFirst()) {
                    long out=userChat.getLong(1);
                    userChat.close();
                    return out;
                }
                userChat.close();
            } while (userCur.moveToNext());
        }
        userCur.close();
        return -1;
    }

    public CompletableFuture<HCChat> getInfoChatOnline(long chatId) {
        CompletableFuture<HCChat> future=new CompletableFuture<>();
        if(hasChatInDB(chatId)) {
            future.complete(getChatInfo(chatId));
            return future;
        }

        try {
            JSONObject params = new JSONObject();
            params.put("chatId", chatId);
            hc.hcapi.apiReq("getChat", params).thenAccept(data -> {
                try {
                    if(data.getInt("errorCode")==0) {
                        future.complete(getChatFromJSON(data.getJSONObject("data")));
                    } else {
                        Log.e(TAG,"Error getChat: "+data.getInt("errorCode")+";"+data.getString("error"));
                        future.complete(null);
                    }
                } catch (JSONException e) {
                    Log.e(TAG,"getChat",e);
                    future.complete(null);
                }
            });
        } catch (JSONException e) {
            Log.e(TAG,"getInfoChatOnline",e);
            future.complete(null);
        }

        return future;
    }

    private static class HCFileOrder {
        protected File file;
        protected String contentType;
        protected long chatId;
        protected int uid;
        public HCFileOrder(File file,String contentType,long chatId,int uid) {
            this.file=file;
            this.contentType=contentType;
            this.chatId=chatId;
            this.uid=uid;
        }
    }

    protected interface HCFileUploadEvent {
        void onUpload(HCFile file, int uid);
        void onProgress(HCFile file,int uid,int percent);
    }

    public HalChatGroupChats(SQLiteDatabase sdb,String codeUser, long userId,HalChat hc) {
        db=sdb;
        this.codeUser=codeUser;
        this.hc=hc;
        this.userId=userId;
    }



    //Проверка есть ли список пользователей
    public void checkChatsUsers() {
        Log.d(TAG,"Start scanning chats users");
    }

    protected void deleteChatUser(long chatId,long toId) {
        db.execSQL("DELETE FROM `groupChatsUsers` WHERE `chatId`=? AND `toId`=?", new String[]{String.valueOf(chatId),String.valueOf(toId)});
    }

    public void updateIcon(String id,long uid) {
        db.execSQL("UPDATE `groupChats` SET `icon`=? WHERE `uid`=?",new String[]{id,String.valueOf(uid)});
    }

    protected CompletableFuture<Void> addNewChat(long chatId) {
        try {
            if (!hasChatInDB(chatId)) {
                JSONObject postData = new JSONObject();
                postData.put("chatId", chatId);
                return hc.hcapi.apiReq("getChat", postData).thenAccept(data -> {
                    try {
                        if(data.getInt("errorCode")==0) {
                            HCChat chat=getChatFromJSON(data.getJSONObject("data"));
                            if (!hasChatInDB(chatId)) {
                                Log.e(TAG,"NEW CHAT: "+chat.icon);
                                db.execSQL("INSERT INTO `groupChats` (`chatUID`, `id`, `name`, `icon`, `fromMe`, `created`, `publicType`, `isAllowMessages`, `chatType`, `isAllowComments`, `description`) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", new String[]{
                                        String.valueOf(chat.chatUID),
                                        chat.id,
                                        chat.name,
                                        chat.icon,
                                        chat.fromMe ? "1" : "0",
                                        String.valueOf(chat.created),
                                        String.valueOf(chat.publicType),
                                        String.valueOf(chat.isAllowMessages),
                                        String.valueOf(chat.chatType),
                                        String.valueOf(chat.isAllowComments),
                                        chat.description
                                });
                            }
                            hc.passwordSync.requestMissingPassword(chat.uid);
                            hc.hd.addHalDriveFile(chat.icon);
                            hc.onNewChat(getChatInfo(chatId));
                        } else {
                            Log.e(TAG,"getChat: "+data.getString("error")+";"+data.getInt("errorCode"));
                        }
                    } catch (Exception e) {
                        Log.e(TAG,"getChat",e);
                    }
                });
            }
        } catch (Exception e) {
            Log.e(TAG,"Error in addNewChat",e);
        }
        return CompletableFuture.completedFuture(null);
    }

    //Проверка на отправку сообщений
    protected void checkSendMessages() {
        Cursor sendCur=db.rawQuery("SELECT * FROM `sendMessages`",null);
        if(sendCur.moveToFirst()) {
            do {
                try {
                    final long uid=sendCur.getLong(0);
                    long t=System.currentTimeMillis();
                    JSONObject mdata=null;
                    String sdata=sendCur.getString(11);
                    if(sdata!=null&&!sdata.equals("null")&&!sdata.equals("NULL"))mdata=new JSONObject(sdata);
                    HCMessage msg=new HCMessage(-1,-1,sendCur.getLong(1),sendCur.getLong(2),t/1000L,sendCur.getLong(3),sendCur.getLong(4),
                            sendCur.getString(5),new JSONArray(),sendCur.getString(6),sendCur.getString(7),sendCur.getString(8),
                            HalChatFunctionsLib.hexStringToByteArray(sendCur.getString(9)), false,false,false,sendCur.getInt(10),true,true,
                            mdata,sendCur.getLong(12),sendCur.getLong(13),false,sendCur.getInt(14));
                    //TODO Сделать отправку опроса после появления интернета

                    sendMessage(msg,null).thenAccept(data->{
                        if(data!=null) {
                            db.execSQL("DELETE FROM `sendMessages` WHERE `uid`=?", new String[]{String.valueOf(uid)});
                        }
                    });
                } catch (JSONException e) {
                    Log.e(TAG,"SM",e);
                }
            } while (sendCur.moveToNext());
        }
        sendCur.close();
    }

    //Проверка новых сообщений
    protected void checkChats() {
        //Copy Message LastIDS Chats
        Cursor chatCur=db.rawQuery("SELECT * FROM `groupChats` WHERE `isDelete`=0",null);
        if (chatCur.moveToFirst()) {
            do {
                try {
                    HCChat chat=getChatFromCursor(chatCur);
                    HCMessage lmsg=getLastMessage(chat);
                    if(lmsg==null) {
                        lastChatIds.put(chat.chatUID,-1L);
                    } else {
                        lastChatIds.put(chat.chatUID,lmsg.uid);
                    }
                } catch (JSONException e) {
                    Log.e(TAG,"CopyLIDS",e);
                }
            } while (chatCur.moveToNext());
        }

        chatCur.close();

        Log.e(TAG,"Successfully CopyLIDS");


        Log.d(TAG,"Start scanning chats");

        hc.hcapi.apiReq("getListChats",new JSONObject()).thenAccept(jsonObject->{
            try {
                if (jsonObject.getInt("errorCode") == 0) {
                    JSONArray chats = jsonObject.getJSONArray("chats");

                    if (chats.length()>0) {
                        List<JSONObject> listChats=new ArrayList<>();

                        for (int i=0;i<chats.length();i++) {
                            JSONObject addOut=chats.getJSONObject(i);
                            if (addOut.isNull("lastMessage")||!hasPasswordChat(addOut.getLong("uid"))) {
                                addOut.put("chatTime",addOut.getLong("created"));
                            } else {
                                addOut.put("chatTime",addOut.getJSONObject("lastMessage").getLong("time"));
                            }
                            listChats.add(addOut);
                        }

                        listChats.sort((o1, o2) -> {
                            try {
                                return o1.getLong("chatTime") < o2.getLong("chatTime") ? 1 : -1;
                            } catch (JSONException e) {
                                Log.e(TAG, "Error json in sort", e);
                            }
                            return 0;
                        });

                        for (int i=0;i<listChats.size();i++) {
                            JSONObject chat=listChats.get(i);
                            if (!hasChatInDB(chat.getLong("uid"))) {
                                addNewChat(chat.getLong("uid"));
                                Log.e(TAG,"Add new chat");
                            }
                            checkNUpdateChatInfo(getChatFromJSON(chat));
                            updateAllPollsChat(chat.getLong("uid"));
                        }

                        Log.e(TAG,"Scanning chats is over");
                        Log.e(TAG,"Start loading messages");
                        hc.chatGroupChats.loadMessages();
                        hc.chatGroupChats.loadOldMessages();
                    } else {
                        Log.e(TAG,"No chats");
                    }
                } else {
                    Log.d(TAG, "Error checkChats: " + jsonObject.getInt("errorCode") + ";" + jsonObject.getString("error"));
                }
            } catch (JSONException e) {
                Log.e(TAG,"getListChats",e);
            }
        });
    }

    private void checkNUpdateChatInfo(HCChat chat) {
        HCChat origChat=getChatInfo(chat.chatUID);

        if(origChat.chatType!=chat.chatType) {
            updateChatType(origChat.uid,chat);
        }

        if(!origChat.description.equals(chat.description)) {
            db.execSQL("UPDATE `groupChats` SET `description`=? WHERE `uid`=?",new String[]{String.valueOf(chat.description),String.valueOf(origChat.uid)});
        }

        if(origChat.fromMe!=chat.fromMe) {
            //Сменился владелец чата
            db.execSQL("UPDATE `groupChats` SET `fromMe`=? WHERE `uid`=?",new String[]{chat.fromMe?"1":"0",String.valueOf(origChat.uid)});
        }

        if(!origChat.icon.equals(chat.icon)) {
            db.execSQL("UPDATE `groupChats` SET `icon`=? WHERE `uid`=?", new String[]{String.valueOf(chat.icon), String.valueOf(origChat.uid)});
            hc.hd.addHalDriveFile(chat.icon);
        }

        if(!origChat.name.equals(chat.name)) {
            db.execSQL("UPDATE `groupChats` SET `name`=? WHERE `uid`=?",new String[]{String.valueOf(chat.name),String.valueOf(origChat.uid)});
        }

        if(origChat.isAllowComments!=chat.isAllowComments) {
            db.execSQL("UPDATE `groupChats` SET `isAllowComments`=? WHERE `uid`=?",new String[]{chat.isAllowComments?"1":"0",String.valueOf(origChat.uid)});
        }

        if(origChat.isAllowMessages!=chat.isAllowMessages) {
            db.execSQL("UPDATE `groupChats` SET `isAllowMessages`=? WHERE `uid`=?",new String[]{chat.isAllowMessages?"1":"0",String.valueOf(origChat.uid)});
        }

        if(origChat.isDelete!=chat.isDelete) {
            //Это либо чат был удалён, либо восстановлен
            db.execSQL("UPDATE `groupChats` SET `isDelete`=? WHERE `uid`=?",new String[]{chat.isDelete?"1":"0",String.valueOf(origChat.uid)});
        }

        if(!origChat.id.equals(chat.id)) {
            db.execSQL("UPDATE `groupChats` SET `id`=? WHERE `uid`=?",new String[]{chat.id,String.valueOf(origChat.uid)});
        }

        if(origChat.publicType!=chat.publicType) {
            //Сменился тип публичности
            db.execSQL("UPDATE `groupChats` SET `publicType`=? WHERE `uid`=?",new String[]{String.valueOf(chat.publicType),String.valueOf(origChat.uid)});

        }
    }

    private void updateChatType(long uid,HCChat chat) {
        db.execSQL("UPDATE `groupChats` SET `chatType`=? WHERE `uid`=?",new String[]{String.valueOf(chat.chatType),String.valueOf(uid)});
    }

    @SuppressLint("Range")
    protected Future<Void> checkChat(long chatId) {
        //TODO FIX SYNC

        return TaskExecutorManager.getInstance().submitDecryptChatActivity("sync-chat:"+chatId,()->{
            Cursor chatCur =db.rawQuery("SELECT * FROM `groupChats` WHERE `isDelete`=0 AND `password`!='-1' AND `chatUID`=?",new String[]{String.valueOf(chatId)});
            if (chatCur.moveToFirst()) {
                try {
                    HCChat chat=getChatFromCursor(chatCur);
                    boolean isEnd=chatCur.getShort(chatCur.getColumnIndex("isEnd"))==1;

                    try {
                        //TODO FIX LOAD MESSAGES
                        HCMessage lmsg=getLastMessage(chat);
                        long lastMessageId=lmsg==null?-1:lmsg.msgId;
                        syncChatNewMessagesAsync(chat,lastMessageId);
                    } catch (Exception e) {
                        Log.e(TAG,"loadMessages",e);
                    }
                    if(!isEnd) {
                        try {
                            Log.e(TAG,"LOAD OLD MESSAGES CHECK CHAT");
                            while(loadOldMessagesChat(chat).get()){
                                Log.e(TAG,"LOAD ONE");
                            }
                        } catch (Exception e) {
                            Log.e(TAG,"loadOldMessages",e);
                        }
                    }
                } catch (Exception e) {
                    Log.e(TAG,"GetCountMessages",e);
                }
            }
            chatCur.close();
            return null;
        });
    }

    private CompletableFuture<Boolean> loadOldMessagesChat(HCChat chat) {
        JSONObject postData=new JSONObject();
        CompletableFuture<Boolean> future=new CompletableFuture<>();
        try {
            HCMessage oldMsg = getFirstMessage(chat);

            postData.put("chatId", chat.chatUID);
            postData.put("limit", 100);

            if (oldMsg != null) {
                postData.put("start", oldMsg.msgId);
            }
            hc.hcapi.apiReq("getMessages", postData).thenAccept(res -> {
                try {
                    if (res.getInt("errorCode") == 0) {
                        JSONArray msgs = res.getJSONArray("messages");
                        List<HCMessage> messages=new ArrayList<>();

                        long startTime = System.currentTimeMillis();

                        Log.e(TAG, "Начинаем параллельную расшифровку " + messages.size() + " сообщений...");

                        for (int i = 0; i < msgs.length(); i++) {
                            HCMessage msg=jsonMsgToHCMSG(msgs.getJSONObject(i));

                            msg=decryptMessage(msg,chat.password);

                            if(msg.isHalEnc) {
                                msg=deencryptMessage(msg,chat.password);
                            }
                            messages.add(msg);
                        }

                        Log.e(TAG, "Расшифровка завершена за " + (System.currentTimeMillis() - startTime) + " мс");

                        for (int i = 0; i < messages.size(); i++) {
                            Log.e(TAG, "LOAD GOID: " + i);
                            HCMessage msg=messages.get(i);
                            addMessageToChat(msg, false);
                        }

                        if (msgs.length() < 100) {
                            db.execSQL("UPDATE `groupChats` SET `isEnd`=1 WHERE `chatUID`=?",new String[]{String.valueOf(chat.chatUID)});
                            future.complete(false);
                            return;
                        }
                    } else {
                        Log.e(TAG, "getMessages: " + res);
                    }
                } catch (Exception e) {
                    Log.e(TAG, "getMessages", e);
                }
                future.complete(true);
            });
        } catch (Exception e) {
            Log.e(TAG, "loadLastMessages", e);
            future.complete(false);
        }

        return future;
    }

    protected void loadAllMessages(long chatId) throws JSONException, ExecutionException, InterruptedException {
        JSONObject postData=new JSONObject();
        postData.put("chatId",chatId);
        postData.put("limit",1000);

        while(true) {
            JSONObject res=hc.hcapi.apiReq("getMessages", postData).get();
            if(res.getInt("errorCode")==0) {
                JSONArray msgs = res.getJSONArray("messages");

                for(int i=0;i<msgs.length();i++) {
                    HCMessage msg=jsonMsgToHCMSG(msgs.getJSONObject(i));
                    addMessageToChat(msg,false);
                }
                if(msgs.length()<1000)break;
                HCMessage lastMsg=jsonMsgToHCMSG(msgs.getJSONObject(999));
                postData.put("start",lastMsg.msgId);
            } else {
                Log.e(TAG,"loadAllMessagesError: "+res);
                break;
            }
        }
    }

    protected boolean addChatWithoutThread(long uid, String password) {
        if(!hasChatInDB(uid)) {
            try {
                JSONObject postData=new JSONObject();
                postData.put("chatId",uid);
                JSONObject jsonObject=hc.hcapi.apiReq("getChat",postData).get();
                if (jsonObject.getInt("errorCode") == 0) {
                    JSONObject chat = jsonObject.getJSONObject("data");
                    if (!hasChatInDB(chat.getLong("uid"))) {
                        db.execSQL("INSERT INTO `groupChats` (`chatUID`, `id`, `name`, `icon`, `fromMe`, `created`, `publicType`, `isAllowMessages`, `chatType`, `isAllowComments`, `password`, `description`) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,?)", new String[]{
                                String.valueOf(chat.getLong("uid")),
                                chat.getString("id"),
                                chat.getString("name"),
                                chat.getString("icon"),
                                chat.getBoolean("fromMe") ? "1" : "0",
                                String.valueOf(chat.getLong("created")),
                                String.valueOf(chat.getInt("publicType")),
                                String.valueOf(chat.getInt("isAllowMessages")),
                                String.valueOf(chat.getInt("chatType")),
                                String.valueOf(chat.getInt("isAllowComments")),
                                password,
                                chat.getString("description")
                        });
                        hc.passwordSync.requestMissingPassword(chat.getLong("uid"));
                        hc.hd.addHalDriveFile(chat.getString("icon"));
                        Cursor chatCur=getChatCursorById(chat.getLong("uid"));
                        HCChat hchat = getChatFromCursor(chatCur);
                        chatCur.close();
                        while(loadLastMessagesWithoutThread(hchat).get());
                        hc.onNewChat(hchat);
                        loadChatUsers(hchat.chatUID);
                        Log.d(TAG, "Add new chat");
                        return true;
                    }
                }
                Log.d(TAG, "Successful add new chat");
            } catch (Exception e) {
                Log.e(TAG, "Failed to fetch HalChat API", e);
            }
        }
        return false;
    }

    //Добавление нового чата
    protected CompletableFuture<Void> addChat(long uid,String password) {
        CompletableFuture<Void> future= new CompletableFuture<>();
        TaskExecutorManager.getInstance().submitChatSync("addChat:"+uid,()->{
            addChatWithoutThread(uid,password);
            future.complete(null);
            return null;
        });
        return future;
    }

    //Запрос пароля на чат
    protected boolean requestPasswordChat(long uid) {
        Log.d(TAG,"Start request password");
        /*if (hasPasswordChat(uid)){return true;}
        try {
            RSACipher rsa= new RSACipher(uid);

            JSONObject postData=new JSONObject();
            postData.put("chatId",uid);
            postData.put("publicKey",Base64.encodeToString(Objects.requireNonNull(rsa.getPublicKey("pkcs8-pem")).getBytes(StandardCharsets.UTF_8), Base64.NO_WRAP));
            JSONObject jsonObject=hc.chatWS.apiReq("requestPasswordChat",postData).get();

            if (jsonObject.getInt("errorCode") == 0) {
                Log.d(TAG,"Successful requestPasswordChat");
                return true;
            } else {
                Log.d(TAG, "Error requestPasswordChat: " + jsonObject.getInt("errorCode") + ";" + jsonObject.getString("error"));
            }
        } catch (Exception e) {
            Log.e(TAG,"Failed to fetch HalChat API",e);
        }*/
        return false;
    }


    private int getTimeFromChat(long chatCreated,HCMessage msg) {
        long out=chatCreated;

        if (msg!=null) {
            out=msg.time;
        }

        out=System.currentTimeMillis()-out*1000;
        out/=1000;
        return (int)out;
    }

    protected boolean hasLastMessageChat(long uid) {
        Cursor cursor=db.rawQuery("SELECT * FROM `groupChatsMessages` WHERE `chatId`=? AND `isDelete`=0",new String[]{String.valueOf(uid)});
        if (cursor.moveToFirst()) {
            cursor.close();
            return true;
        }
        cursor.close();
        return false;
    }

    public List<ChatInfoList> getChatInfoList() {
        List<ChatInfoList> chats = new ArrayList<>();
        List<HCChat> chatList = new ArrayList<>();
        StringBuilder chatIds = new StringBuilder();

        // 1. Получаем все чаты
        Cursor chatCur = db.rawQuery("SELECT * FROM `groupChats` WHERE `isDelete`=0", null);
        if (chatCur.moveToFirst()) {
            do {
                HCChat chat = getChatFromCursor(chatCur);
                chatList.add(chat);

                // Собираем ID чатов для второго запроса
                if (chatIds.length() > 0) chatIds.append(",");
                chatIds.append(chat.chatUID);
            } while (chatCur.moveToNext());
        }
        chatCur.close();

        // Если чатов нет, сразу возвращаем пустой список
        if (chatList.isEmpty()) return chats;

        // 2. Получаем последние сообщения для всех этих чатов
        @SuppressLint("UseSparseArrays")
        HashMap<Long, HCMessage> lastMessages = new HashMap<>();

        String msgQuery = "SELECT m.* FROM `groupChatsMessages` m " +
                "INNER JOIN (SELECT chatId, MAX(msgId) as maxMsgId FROM `groupChatsMessages` " +
                "WHERE `isDelete`=0 AND `commentMsg`=-1 AND `chatId` IN (" + chatIds.toString() + ") GROUP BY chatId) max_m " +
                "ON m.chatId = max_m.chatId AND m.msgId = max_m.maxMsgId";

        Cursor msgCur = db.rawQuery(msgQuery, null);
        if (msgCur.moveToFirst()) {
            do {
                HCMessage msg = getMessageFromCursor(msgCur, userId);
                lastMessages.put(msg.chatId, msg);
            } while (msgCur.moveToNext());
        }
        msgCur.close();

        // 3. Собираем всё вместе в памяти
        for (HCChat chat : chatList) {
            HCEncryptedMessage lmsg;

            boolean hasPassword = chat.password != null && !chat.password.equals("-1");

            if (!hasPassword) {
                lmsg = new HCEncryptedMessage("Введите пароль чтобы прочитать", false, null);
            } else {
                HCMessage msg = lastMessages.get(chat.chatUID);
                if (msg == null) {
                    lmsg = new HCEncryptedMessage("Создан новый чат", false, null);
                } else {
                    if (msg.message == null || msg.message.isEmpty()) {
                        lmsg = new HCEncryptedMessage("Прикреплён файл", false, msg);
                    } else {
                        //TODO REPLACE EMOJI
                        lmsg = new HCEncryptedMessage("", true, msg);
                    }
                }
            }

            int time = getTimeFromChat(chat.created, lmsg.msg);
            chats.add(new ChatInfoList(
                    chat.chatUID,
                    chat.name,
                    lmsg,
                    chat.icon,
                    getTimeFromSeconds(time),
                    time
            ));
        }

        return chats;
    }

    public HCChat getChatInfo(long chatId) {
        Cursor chatCur=db.rawQuery("SELECT * FROM `groupChats` WHERE `chatUID`=? AND `isDelete`=0",new String[]{String.valueOf(chatId)});
        if(chatCur.moveToFirst()) {
            HCChat chat=getChatFromCursor(chatCur);
            chatCur.close();
            return chat;
        }
        chatCur.close();
        return null;
    }

    protected HCEncryptedMessage getChatLastMessage(Cursor cursorChat) {
        if (!hasPasswordChat(cursorChat.getLong(1))) {
            return new HCEncryptedMessage("Введите пароль чтобы прочитать",false,null);
        }
        Cursor cursor=db.rawQuery("SELECT * FROM `groupChatsMessages` WHERE `chatId`=? AND `isDelete`=0 ORDER BY `msgId` DESC LIMIT 1",new String[]{String.valueOf(cursorChat.getLong(1))});

        if(cursor.moveToFirst()) {
            HCMessage msg=getMessageFromCursor(cursor,userId);
            cursor.close();
            if (msg.message.isEmpty()) {
                return new HCEncryptedMessage("Прикреплён файл",false,msg);
            }

            //TODO REPLACE EMOJI
            return new HCEncryptedMessage("",true,msg);
        }
        cursor.close();
        return new HCEncryptedMessage("Создан новый чат",false,null);
    }

    protected boolean hasChatInDB(long uid) {
        Cursor chatCur=db.rawQuery("SELECT * FROM `groupChats` WHERE `chatUID`=?",new String[]{String.valueOf(uid)});
        if (chatCur.moveToFirst()) {
            chatCur.close();
            return true;
        }
        chatCur.close();
        return false;
    }

    protected boolean hasPasswordChat(long uid) {
        return getPasswordChat(uid)!=null;
    }

    protected String getPasswordChat(long uid) {
        Cursor chatCur=db.rawQuery("SELECT * FROM `groupChats` WHERE `chatUID`=?",new String[]{String.valueOf(uid)});
        if (chatCur.moveToFirst()) {
            String password=chatCur.getString(12);
            chatCur.close();
            return password.equals("-1")?null:password;
        }
        chatCur.close();
        return null;
    }

    //Заход в новый чат по паролю
    protected CompletableFuture<Boolean> enterNewChatWithoutThread(long chatId,String origPassword,String password) {

        try {
            JSONObject postData=new JSONObject();
            postData.put("chatId",chatId);
            postData.put("password",password);

            CompletableFuture<Boolean> future=new CompletableFuture<>();

            hc.hcapi.apiReq("joinChat",postData).thenAccept(jsonObject -> {
                try {
                    if (jsonObject.getInt("errorCode") == 0) {
                        Log.d(TAG, "Successful enter chat");
                        addNewChat(chatId).thenAccept(v->{
                            db.execSQL("UPDATE `groupChats` SET `password`=? WHERE `chatUID`=?", new String[]{origPassword, String.valueOf(chatId)});
                            HCChat hcchat = getChatInfo(chatId);
                            TaskExecutorManager.getInstance().submitDecrypt("chat"+chatId, () -> {
                                while (loadLastMessagesWithoutThread(hcchat).get()) ;
                                HCMessage lmsg=getLastMessage(hcchat);
                                lmsg=decryptMessage(lmsg,getPasswordChat(chatId));
                                hc.chatListI.onNewMessage(hcchat, lmsg);
                                loadChatUsers(chatId);
                                return null;
                            });

                            future.complete(true);
                        });

                    } else {
                        Log.e(TAG,"joinChat Error: "+jsonObject.getInt("errorCode")+";"+jsonObject.getString("error"));
                        future.complete(false);
                    }
                } catch (Exception e) {
                    Log.e(TAG,"joinChat",e);
                }
            });

            return future;
        } catch (Exception e) {
            Log.e(TAG,"Error in enterChatWithoutThread",e);
        }
        return CompletableFuture.completedFuture(false);
    }

    //Заход в чат по паролю
    protected CompletableFuture<Boolean> enterChatWithoutThread(long uid,String origPassword,String password) {
        Cursor chatCur=db.rawQuery("SELECT * FROM `groupChats` WHERE `chatUID`=?",new String[]{String.valueOf(uid)});
        if (chatCur.moveToFirst()) {
            if (!chatCur.getString(12).equals("-1")){chatCur.close();return CompletableFuture.completedFuture(true);}
            try {
                long chatId=chatCur.getLong(1);

                JSONObject postData=new JSONObject();
                postData.put("chatId",chatId);
                postData.put("password",password);
                chatCur.close();

                CompletableFuture<Boolean> future=new CompletableFuture<>();

                hc.hcapi.apiReq("joinChat",postData).thenAccept(jsonObject -> {
                    try {
                        if (jsonObject.getInt("errorCode") == 0) {
                            Log.d(TAG, "Successful enter chat");
                            db.execSQL("UPDATE `groupChats` SET `password`=? WHERE `chatUID`=?", new String[]{origPassword, String.valueOf(uid)});
                            HCChat hcchat = getChatInfo(chatId);
                            TaskExecutorManager.getInstance().submitDecrypt("chat"+uid, () -> {
                                while (loadLastMessagesWithoutThread(hcchat).get()) ;
                                HCMessage lmsg=getLastMessage(hcchat);
                                lmsg=decryptMessage(lmsg,getPasswordChat(chatId));
                                hc.chatListI.onNewMessage(hcchat, lmsg);
                                loadChatUsers(chatId);
                                return null;
                            });
                            future.complete(true);

                        } else {
                            Log.e(TAG,"joinChat Error: "+jsonObject.getInt("errorCode")+";"+jsonObject.getString("error"));
                            future.complete(false);
                        }
                    } catch (Exception e) {
                        Log.e(TAG,"joinChat",e);
                    }
                });

                return future;
            } catch (Exception e) {
                Log.e(TAG,"Error in enterChatWithoutThread",e);
            }
        }
        chatCur.close();
        Log.d(TAG,"Unsuccessful enter chat");
        return CompletableFuture.completedFuture(false);
    }

    protected Cursor getChatCursorById(long id) {
        Cursor chatCur=db.rawQuery("SELECT * FROM `groupChats` WHERE `chatUID`=?",new String[]{String.valueOf(id)});
        if(chatCur.moveToFirst()) {
            return chatCur;
        }
        chatCur.close();
        return null;
    }

    //Загрузка сообщений которые были пропущены во время выключения или пропущены приложением
    public void loadMessages() {
        //Scanning and loading messages from not start messages loaded chats
        Log.e(TAG,"Start loading start messages");

        TaskExecutorManager.getInstance().submitChatSync("loadMessages",()->{
            Cursor chatCur=db.rawQuery("SELECT * FROM `groupChats` WHERE `password`!='-1'",null);
            if(chatCur.moveToFirst()) {
                do {
                    HCChat chat=HalChatGroupChats.getChatFromCursor(chatCur);
                    try {
                        //TODO FIX LOAD MESSAGES
                        HCMessage lmsg=getLastMessage(chat);
                        long lastMessageId=lmsg==null?-1:lmsg.msgId;
                        syncChatNewMessagesAsync(chat,lastMessageId);
                    } catch (Exception e) {
                        Log.e(TAG,"loadMessages",e);
                    }
                } while (chatCur.moveToNext());
            }
            chatCur.close();
            return null;
        });
    }

    private void syncChatNewMessagesAsync(HCChat chat, long lastMessageId) throws JSONException {
        JSONObject postData = new JSONObject();
        postData.put("chatId", chat.chatUID);
        postData.put("limit", 100);
        postData.put("start", lastMessageId);
        postData.put("isNew",1);

        hc.hcapi.apiReq("getMessages", postData).thenAccept(res -> {
            try {
                if (res.getInt("errorCode") == 0) {
                    JSONArray msgs = res.getJSONArray("messages");

                    insertMessagesBatch(msgs,chat,true);

                    if (msgs.length() == 100) {
                        long newLastId = msgs.getJSONObject(0).getLong("uid");
                        syncChatNewMessagesAsync(chat, newLastId);
                    }
                }
            } catch (Exception e) {
                Log.e(TAG, "Sync error", e);
            }
        });
    }

    protected void insertMessagesBatch(JSONArray msgs, HCChat chat,boolean isNew) {
        db.beginTransaction();
        try {
            for (int i = 0; i < msgs.length(); i++) {
                HCMessage msg = jsonMsgToHCMSG(msgs.getJSONObject(i));
                if(hasMessageById(msg.msgId))continue;

                if(hasPasswordChat(msg.chatId)) {
                    deencryptMessage(msg, getPasswordChat(msg.chatId));
                }

                addMessageToChat(msg,isNew);


                if(hc.chatInterface != null) {
                    if(isNew){hc.chatInterface.onNewMessage(msg);}
                    else{hc.chatInterface.onLoadMessage(msg);}
                }
            }
            db.setTransactionSuccessful();

        } catch (Exception e) {
            Log.e(TAG, "Batch insert error", e);
        } finally {
            db.endTransaction();
        }
    }

    //Загрузка старых сообщений в чатах которые не были загружены до вступления в чат
    public void loadOldMessages() {
        Log.d(TAG,"Start loading old messages");
        TaskExecutorManager.getInstance().submitChatSync("loadOldMessages",()->{
            Cursor chatCur=db.rawQuery("SELECT * FROM `groupChats` WHERE `password`!='-1' AND `isEnd`=0",null);
            if(chatCur.moveToFirst()) {
                do {
                    HCChat chat=HalChatGroupChats.getChatFromCursor(chatCur);
                    try {
                        while(loadOldMessagesChat(chat).get()){}
                    } catch (Exception e) {
                        Log.e(TAG,"loadOldMessages",e);
                    }
                } while (chatCur.moveToNext());
            }
            chatCur.close();
            return null;
        });
    }

    //Загрузка старых сообщений
    protected CompletableFuture<Boolean> loadOldMessagesWithoutThread(HCChat chat) {
        CompletableFuture<Boolean> future=new CompletableFuture<>();
        JSONObject postData=new JSONObject();

        try {
            HCMessage oldMsg=getFirstMessage(chat);

            postData.put("chatId",chat.chatUID);
            postData.put("limit", 100);

            if(oldMsg!=null) {
                postData.put("start",oldMsg.msgId);
            }
            hc.hcapi.apiReq("getMessages", postData).thenAccept(res->{
                try {
                    if(res.getInt("errorCode")==0) {
                        JSONArray msgs = res.getJSONArray("messages");
                        Log.d(TAG, "Successfully load messages: "+jsonMsgToHCMSG(msgs.getJSONObject(msgs.length()-1)).msgId+";"+oldMsg.msgId);

                        for(int i=0;i<msgs.length();i++) {
                            HCMessage msg=jsonMsgToHCMSG(msgs.getJSONObject(i));
                            addMessageToChat(msg,false);
                        }

                        if(msgs.length()==100) {
                            future.complete(true);
                            return;
                        }
                        future.complete(false);
                    } else {
                        Log.e(TAG,"getMessages: "+ res);
                        future.complete(false);
                    }
                } catch (Exception e) {
                    Log.e(TAG,"getMessages",e);
                    future.complete(false);
                }
            });
        } catch (Exception e) {
            Log.e(TAG,"loadLastMessages",e);
            future.complete(false);
        }
        return future;
    }

    //Загрузка последнего сообщения чата
    protected CompletableFuture<Boolean> loadLastMessagesWithoutThread(HCChat chat) {
        CompletableFuture<Boolean> future=new CompletableFuture<>();
        JSONObject postData=new JSONObject();

        try {
            postData.put("chatId",chat.chatUID);
            postData.put("limit", 100);
            postData.put("isNew","1");

            if(lastChatIds.containsKey(chat.chatUID)&&lastChatIds.get(chat.chatUID)!=-1) {
                postData.put("start",lastChatIds.get(chat.chatUID));
            }

            hc.hcapi.apiReq("getMessages", postData).thenAccept(res->{
                try {
                    if(res.getInt("errorCode")==0) {
                        Log.d(TAG, "Successfully load messages");
                        JSONArray msgs = res.getJSONArray("messages");

                        for(int i=0;i<msgs.length();i++) {
                            HCMessage msg=jsonMsgToHCMSG(msgs.getJSONObject(i));
                            addMessageToChat(msg,false);
                        }

                        if(msgs.length()==100) {
                            future.complete(true);
                            return;
                        }
                        future.complete(false);
                    } else {
                        Log.e(TAG,"getMessages: "+ res);
                        future.complete(false);
                    }
                } catch (Exception e) {
                    Log.e(TAG,"getMessages",e);
                    future.complete(false);
                }
            });
        } catch (Exception e) {
            Log.e(TAG,"loadLastMessages",e);
            future.complete(false);
        }
        return future;
    }

    protected long getCountMessages(long chatId) {
        Cursor cursor=db.rawQuery("SELECT COUNT(*) FROM `groupChatsMessages` WHERE `chatId`=?",new String[]{String.valueOf(chatId)});
        long count = 0;
        if (cursor.moveToFirst()) {
            count = cursor.getLong(0);
        }
        cursor.close();
        return count;
    }

    protected CompletableFuture<Long> getCountMessagesServer(long chatId) {
        CompletableFuture<Long> future=new CompletableFuture<>();
        try {
            JSONObject postData=new JSONObject();
            postData.put("chatId",chatId);
            hc.hcapi.apiReq("getCountMessages",postData).thenAccept(res->{
                try {
                    if(res.getInt("errorCode")==0) {
                        future.complete(res.getLong("count"));
                        return;
                    }
                    future.complete(0L);
                } catch (Exception e) {
                    Log.e(TAG,"getCountMessages",e);
                    future.completeExceptionally(e);
                }
            });
        } catch (Exception e) {
            future.completeExceptionally(e);
        }
        return future;
    }

    public void loadMessage(long chatId,long msgId) {
        Log.e(TAG,"Start loading message");
        TaskExecutorManager.getInstance().submitChatSync("loadMessage:msgId:"+msgId,()->{
            try {
                JSONObject postData=new JSONObject();
                postData.put("chatId",chatId);
                postData.put("msgId",msgId);
                JSONObject res= hc.hcapi.apiReq("getMessage",postData).get();
                if (res.getInt("errorCode") == 0) {
                    Log.e(TAG,"Successfully load message");
                    HCMessage msg=jsonMsgToHCMSG(res.getJSONObject("msg"));
                    addMessageToChat(msg,false);
                }
            } catch (Exception e) {
                Log.e(TAG,"Error in loadMessage",e);
            }
            return null;
        });
    }

    public void updateMessage(long chatId,long msgId) {
        Log.e(TAG,"Update Message");
        TaskExecutorManager.getInstance().submitChatSync("updateMessage:msgId:"+msgId,()->{
            try {
                JSONObject postData=new JSONObject();
                postData.put("chatId",chatId);
                postData.put("msgId",msgId);
                JSONObject res= hc.hcapi.apiReq("getMessage",postData).get();
                if (res.getInt("errorCode") == 0) {
                    Log.e(TAG,"Successfully update message");
                    HCMessage msg=jsonMsgToHCMSG(res.getJSONObject("msg"));
                    editMessage(msg);
                }
            } catch (Exception e) {
                Log.e(TAG,"Error in updateMessage",e);
            }
            return null;
        });
    }

    protected HCMessage jsonMsgToHCMSG(JSONObject msg) throws JSONException {
        return new HCMessage(
                -1,msg.getLong("uid"),msg.getLong("fromChat"),msg.getLong("fromId"),
                msg.getLong("time"),msg.getLong("answerMsg"),msg.getLong("commentMsg"),
                msg.getString("message"),msg.getJSONArray("attachments"),msg.getString("soundMsg"),
                msg.getString("dataBot"),msg.getString("recordMic"), HalChatFunctionsLib.hexStringToByteArray(msg.getString("encryptId")),
                false,true,msg.getBoolean("isReaded"),msg.getInt("type"),msg.getLong("fromId")==userId,true,
                msg.isNull("data") ? null:msg.getJSONObject("data"),0,msg.getLong("pixelId"),msg.getBoolean("isPinned"),msg.getLong("v")
        );
    }

    protected Cursor getCursorMessageById(long id) {
        Cursor msgCur=db.rawQuery("SELECT * FROM `groupChatsMessages` WHERE `msgId`=? AND `isDelete`=0",new String[]{String.valueOf(id)});
        if(msgCur.moveToLast()) {
            return msgCur;
        }
        msgCur.close();
        return null;
    }

    protected boolean hasMessageById(long id) {
        Cursor msgCur=db.rawQuery("SELECT * FROM `groupChatsMessages` WHERE `msgId`=? AND `isDelete`=0",new String[]{String.valueOf(id)});
        if(msgCur.moveToLast()) {
            msgCur.close();
            return true;
        }
        msgCur.close();
        return false;
    }

    protected CompletableFuture<Boolean> sendDeleteMessage(HCMessage message) {
        if(message.isDelete){deleteMessageById(message.msgId);return CompletableFuture.completedFuture(true);}

        CompletableFuture<Boolean> future=new CompletableFuture<>();
        try {
            JSONObject postData=new JSONObject();
            postData.put("chatId",message.chatId);
            postData.put("msgId",message.msgId);
            hc.hcapi.apiReq("deleteMessage", postData).thenAccept(res->{
                if(res.optInt("errorCode",-1)==0) {
                    deleteMessageById(message.msgId);
                    future.complete(true);
                } else {
                    future.complete(false);
                }
            });
        } catch (Exception e) {
            Log.e(TAG,"sendDeleteMessage",e);
            future.completeExceptionally(e);
        }
        return future;
    }

    protected void deleteMessageById(long id) {
        db.execSQL("UPDATE `groupChatsMessages` SET `isDelete`=1 WHERE `msgId`=? AND `isDelete`=0",new String[]{
                String.valueOf(id)
        });

        if(hc.chatInterface != null) {
            hc.chatInterface.onDeleteMessage(id);
        }
    }

    protected CompletableFuture<Boolean> sendEditMessage(HCMessage msg) {
        if(msg.isDelete){deleteMessageById(msg.msgId);return CompletableFuture.completedFuture(false);}

        CompletableFuture<Boolean> future=new CompletableFuture<>();
        try {
            String encryptId = generateEncryptId(System.currentTimeMillis(), msg.chatId);
            msg.encryptId = HalChatFunctionsLib.hexStringToByteArray(encryptId);
            msg.message = encryptMessage(msg.decryptedMessage, getPasswordChat(msg.chatId), msg.encryptId);
            try {
                JSONObject postData=new JSONObject();
                postData.put("msgId",msg.msgId);
                postData.put("message",msg.message);
                postData.put("attachments",msg.attachments.toString());
                postData.put("encryptId",encryptId);
                postData.put("chatId",msg.chatId);
                postData.put("soundMsg",-1);
                if(!msg.recordMic.equals("-1")) {
                    postData.put("recordMic",msg.recordMic);
                }
                if(msg.answerMsg!=-1) {
                    postData.put("answerMsg",msg.answerMsg);
                }

                Log.e(TAG,"PostData: "+ postData);

                hc.hcapi.apiReq("editMessage",postData).thenAccept(res->{
                    if(res.optInt("errorCode",-1)==0) {
                        future.complete(true);
                    } else {
                        future.complete(false);
                    }
                });
            } catch (Exception e) {
                Log.e(TAG, "Error in sendMessage", e);
            }
        } catch (Exception e) {
            Log.e(TAG,"sendDeleteMessage",e);
            future.completeExceptionally(e);
        }
        return future;
    }

    protected HCMessage editMessage(HCMessage message) {
        try {
            if(!hasMessageById(message.msgId)) {
                addMessageToChat(message,false);
                return message;
            }
            //deleteMessageById(message.msgId);
            //addMessageToChat(message);
            message=deencryptMessage(message,getPasswordChat(message.chatId));
            db.execSQL("UPDATE `groupChatsMessages` SET `message`=?, `encryptId`=?, `attachments`=?, `time`=?, `answerMsg`=?, `commentMsg`=?, `type`=?, `soundMsg`=?, `recordMic`=?, `isHalEnc`=? WHERE `msgId`=? AND `isDelete`=0",new String[]{
                    message.message, HalChatFunctionsLib.bytesToHex(message.encryptId), message.attachments.toString(), String.valueOf(message.time), String.valueOf(message.answerMsg), String.valueOf(message.commentMsg),
                    String.valueOf(message.type), message.soundMsg, message.recordMic, message.isHalEnc?"1":"0",String.valueOf(message.msgId)
            });

            if(hc.chatInterface!=null) {
                hc.chatInterface.onEditMessage(message);
            }

            return message;
        } catch (JSONException e) {
            Log.e(TAG,"editMessage",e);
        }
        return message;
    }

    protected HCMessage addMessageToChat(HCMessage msg,boolean isNew) throws JSONException {
        if(!hasMessageById(msg.msgId)) {
            if(!lastChatIds.containsKey(msg.chatId)||msg.msgId>lastChatIds.get(msg.chatId)) {
                lastChatIds.put(msg.chatId,msg.msgId);
            }
            if(msg.attachments.length()>0) {
                int i;
                for(i=0;i<msg.attachments.length();i++) {
                    hc.hd.addHalDriveFile(msg.attachments.getString(i));
                }
            }
            if(!msg.recordMic.equals("-1"))hc.hd.addHalDriveFile((msg.recordMic));
            if(hasPasswordChat(msg.chatId)) {
                deencryptMessage(msg, getPasswordChat(msg.chatId));
            }
            db.execSQL("INSERT INTO `groupChatsMessages` (`msgId`, `chatId`, `fromId`, `message`, `encryptId`, `attachments`, `time`, `isDelete`, `answerMsg`," +
                    " `commentMsg`, `type`, `soundMsg`, `dataBot`, `recordMic`, `isSended`, `isReceived`, `isHalEnc`, `data`, `shareId`, `pixelId`, `isPinned`, `v`) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
                    new String[]{String.valueOf(msg.msgId),String.valueOf(msg.chatId),String.valueOf(msg.fromId),msg.message,
                            HalChatFunctionsLib.bytesToHex(msg.encryptId),msg.attachments.toString(),String.valueOf(msg.time),msg.isDelete?"1":"0",
                            String.valueOf(msg.answerMsg),String.valueOf(msg.commentMsg),String.valueOf(msg.type),msg.soundMsg,
                    msg.dataBot,msg.recordMic,msg.isSended?"1":"0",msg.isReceived?"1":"0",msg.isHalEnc?"1":"0",msg.data==null?null:msg.data.toString(),
                            String.valueOf(msg.shareId),String.valueOf(msg.pixelId),msg.isPinned?"1":"0",String.valueOf(msg.v)});
            if(!isNew&&hc.chatInterface!=null){hc.chatInterface.onLoadMessage(msg);}
        }
        return msg;
    }

    @SuppressLint("Range")
    protected HCMessage getMessageFromCursor(Cursor msgCur, long userId) {
        long uid,msgId,chatId,fromId,time,answerMsg,commentMsg,pixelId,v;
        String message,soundMsg,dataBot,recordMic;
        JSONArray attachments;
        byte[] encryptId;
        boolean isDelete,isSended,isReceived,isHalEnc,isPinned;
        int type;
        JSONObject data;
        uid=msgCur.getLong(msgCur.getColumnIndex("uid"));
        msgId=msgCur.getLong(msgCur.getColumnIndex("msgId"));
        chatId=msgCur.getLong(msgCur.getColumnIndex("chatId"));
        fromId=msgCur.getLong(msgCur.getColumnIndex("fromId"));
        message=msgCur.getString(msgCur.getColumnIndex("message"));
        encryptId=HalChatFunctionsLib.hexStringToByteArray(msgCur.getString(msgCur.getColumnIndex("encryptId")));
        try {
            attachments=new JSONArray(msgCur.getString(msgCur.getColumnIndex("attachments")));
        } catch (JSONException e) {
            throw new RuntimeException(e);
        }
        time=msgCur.getLong(msgCur.getColumnIndex("time"));
        isDelete=msgCur.getInt(msgCur.getColumnIndex("isDelete"))==1;
        answerMsg=msgCur.getLong(msgCur.getColumnIndex("answerMsg"));
        commentMsg=msgCur.getLong(msgCur.getColumnIndex("commentMsg"));
        type=msgCur.getInt(msgCur.getColumnIndex("type"));
        soundMsg=msgCur.getString(msgCur.getColumnIndex("soundMsg"));
        dataBot=msgCur.getString(msgCur.getColumnIndex("dataBot"));
        recordMic=msgCur.getString(msgCur.getColumnIndex("recordMic"));
        isSended=msgCur.getInt(msgCur.getColumnIndex("isSended"))==1;
        isReceived=msgCur.getInt(msgCur.getColumnIndex("isReceived"))==1;
        isHalEnc=msgCur.getInt(msgCur.getColumnIndex("isHalEnc"))==1;
        data=null;
        if(!msgCur.isNull(msgCur.getColumnIndex("data"))) {
            try {
                data = new JSONObject(msgCur.getString(msgCur.getColumnIndex("data")));
            } catch (JSONException e) {
                Log.e(TAG, "Parse Data", e);
            }
        }
        pixelId=msgCur.getLong(msgCur.getColumnIndex("pixelId"));
        v=msgCur.getLong(msgCur.getColumnIndex("v"));
        isPinned=msgCur.getInt(msgCur.getColumnIndex("isPinned"))==1;

        if(attachments.length()>0) {
            int i;
            for(i=0;i<attachments.length();i++) {
                try {
                    String fileId = attachments.getString(i);
                    if (!hc.hd.isFileExists(fileId)) {
                        hc.hd.addHalDriveFile(fileId);
                    }
                } catch(JSONException e) {
                    Log.e(TAG,"getMessageFromCursor",e);
                }
            }
        }
        if(!recordMic.equals("-1")){hc.hd.addHalDriveFile(recordMic);}
        return new HCMessage(uid,msgId,chatId,fromId,time,answerMsg,commentMsg,message,attachments,soundMsg,dataBot,recordMic,encryptId,isDelete,isSended,isReceived,type,userId==fromId,isHalEnc,
                data,0,pixelId,isPinned,v);
    }

    public HCMessage getLastMessage(HCChat chat) throws JSONException {
        Cursor msgCur=db.rawQuery("SELECT * FROM `groupChatsMessages` WHERE `chatId`=? AND `isDelete`=0 ORDER BY `msgId` DESC LIMIT 1",new String[]{String.valueOf(chat.chatUID)});
        if(msgCur.moveToLast()) {
            return getMessageFromCursor(msgCur,userId);
        }
        msgCur.close();
        return null;
    }

    public HCMessage getFirstMessage(HCChat chat) throws JSONException {
        Cursor msgCur=db.rawQuery("SELECT * FROM `groupChatsMessages` WHERE `chatId`=? AND `isDelete`=0 ORDER BY `msgId` ASC LIMIT 1",new String[]{String.valueOf(chat.chatUID)});
        if(msgCur.moveToLast()) {
            return getMessageFromCursor(msgCur,userId);
        }
        msgCur.close();
        return null;
    }

    protected static HCChat getChatFromCursor(Cursor chatCur) {
        long uid,chatUID,created;
        String id,name,icon,password,description;
        int publicType,chatType;
        boolean fromMe,isAllowMessages,isDelete,isAllowComments;
        uid=chatCur.getLong(chatCur.getColumnIndexOrThrow("uid"));
        chatUID=chatCur.getLong(chatCur.getColumnIndexOrThrow("chatUID"));
        id=chatCur.getString(chatCur.getColumnIndexOrThrow("id"));
        name=chatCur.getString(chatCur.getColumnIndexOrThrow("name"));
        icon=chatCur.getString(chatCur.getColumnIndexOrThrow("icon"));
        fromMe=chatCur.getInt(chatCur.getColumnIndexOrThrow("fromMe"))==1;
        created=chatCur.getLong(chatCur.getColumnIndexOrThrow("created"));
        publicType=chatCur.getInt(chatCur.getColumnIndexOrThrow("publicType"));
        isAllowMessages=chatCur.getInt(chatCur.getColumnIndexOrThrow("isAllowMessages"))==1;
        isDelete=chatCur.getInt(chatCur.getColumnIndexOrThrow("isDelete"))==1;
        chatType=chatCur.getInt(chatCur.getColumnIndexOrThrow("chatType"));
        isAllowComments=chatCur.getInt(chatCur.getColumnIndexOrThrow("isAllowComments"))==1;
        password=chatCur.getString(chatCur.getColumnIndexOrThrow("password"));
        description=chatCur.getString(chatCur.getColumnIndexOrThrow("description"));
        return new HCChat(uid,chatUID,created,publicType,chatType,id,name,icon,password,fromMe,isAllowMessages,isDelete,isAllowComments,description);
    }

    protected static HCChat getChatFromJSON(JSONObject data) {
        try {
            long uid,chatUID,created;
            String id,name,icon,password,description;
            int publicType,chatType;
            boolean fromMe,isAllowMessages,isAllowComments;

            uid=-1;
            chatUID=data.getLong("uid");
            created=data.getLong("created");
            id=data.getString("id");
            name=data.getString("name");
            icon=data.getString("icon");
            password="";
            publicType=data.getInt("publicType");
            chatType=data.getInt("chatType");
            fromMe=data.getBoolean("fromMe");
            isAllowMessages=data.getInt("isAllowMessages")==1;
            isAllowComments=data.getInt("isAllowComments")==1;
            description=data.getString("description");
            return new HCChat(uid,chatUID,created,publicType,chatType,id,name,icon,password,fromMe,isAllowMessages, false,isAllowComments,description);
        } catch (Exception e) {
            Log.e(TAG,"getChatFromJSON",e);
        }
        return null;
    }

    protected ArrayList<HCMessage> getChatLastMessages(long chatId,long commentMsg,int count,boolean is_decrypt) {
        //FUA Проверку есть ли именно этот пользователь в чате
        Cursor msgCur=db.rawQuery("SELECT * FROM `groupChatsMessages` WHERE `chatId`=? AND `commentMsg`=? AND `isDelete`=0 ORDER BY `msgId` DESC LIMIT ?",new String[]{String.valueOf(chatId),String.valueOf(commentMsg),String.valueOf(count)});
        ArrayList<HCMessage> out=new ArrayList<>();
        String password=getPasswordChat(chatId);
        if(msgCur.moveToFirst()) {
            do {
                HCMessage msg=getMessageFromCursor(msgCur,userId);
                if(is_decrypt) {
                    msg=decryptMessage(msg,password);
                }
                if(msg.isHalEnc) {
                    msg=deencryptMessage(msg,password);
                }
                out.add(msg);
            } while (msgCur.moveToNext());
        }
        msgCur.close();
        return out;
    }

    protected ArrayList<HCMessage> getChatLastMessages(long chatId,long commentMsg,int count,boolean is_decrypt,long msgId) {
        //FUA Проверку есть ли именно этот пользователь в чате
        Cursor msgCur=db.rawQuery("SELECT * FROM `groupChatsMessages` WHERE `chatId`=? AND `commentMsg`=? AND `isDelete`=0 AND `msgId`<? ORDER BY `msgId` DESC LIMIT ?",new String[]{String.valueOf(chatId),String.valueOf(commentMsg),String.valueOf(msgId),String.valueOf(count)});
        ArrayList<HCMessage> out=new ArrayList<>();
        String password=getPasswordChat(chatId);
        if(msgCur.moveToFirst()) {
            do {
                HCMessage msg=getMessageFromCursor(msgCur,userId);
                if(is_decrypt) {
                    msg=decryptMessage(msg,password);
                }
                if(msg.isHalEnc) {
                    msg=deencryptMessage(msg,password);
                }
                out.add(msg);
            } while (msgCur.moveToNext());
        }
        msgCur.close();
        return out;
    }

    protected void decryptMessagesAndAdd(Activity activity, long chatId, List<HCMessage> messages, HalChatAddMessage am) {
        String password=getPasswordChat(chatId);
        if(password==null)return;
        Log.e(TAG,"DECRYPT MESSAGES");

        TaskExecutorManager.getInstance().submitDecryptChatActivity("decryptMessagesAndAdd:chatId:"+chatId,()->{
            Log.e(TAG,"Начинаем параллеьную расшифровку "+messages.size());
            long startTime = System.currentTimeMillis();

            List<HCMessage> out=new ArrayList<>();

            for(int i=0;i<messages.size();i++) {
                HCMessage msg=messages.get(i);

                if(msg.isHalEnc) {
                    msg=deencryptMessage(msg,password);
                }

                msg=decryptMessage(msg,password);

                out.add(msg);
            }

            Log.e(TAG, "Расшифровка завершена за " + (System.currentTimeMillis() - startTime) + " мс");

            activity.runOnUiThread(()->am.addMessages(out));
            return null;
        });
    }

    protected HCMessage decryptMessage(HCMessage msg,String password) {
        if(!hasPasswordChat(msg.chatId))return msg;
        if(msg.message.isEmpty())return msg;
        if(msg.isDecrypted)return msg;
        boolean hasPoll=msg.type==4&&msg.data!=null&&msg.data.has("variants");
        if(msg.isHalEnc) {
            msg.setDecryptedMessage(new String(hc.he.decodeByHash(HalChatFunctionsLib.hexStringToByteArray(msg.message), password + HalChatFunctionsLib.bytesToHex(msg.encryptId), 10, 100000, ""), StandardCharsets.UTF_8));

            if(hasPoll) {
                try {
                    JSONArray oldVariants = msg.data.getJSONArray("variants");
                    JSONArray newVariants = new JSONArray();

                    for(int i=0;i<oldVariants.length();i++) {
                        String vr=new String(hc.he.decodeByHash(HalChatFunctionsLib.hexStringToByteArray(oldVariants.getString(i)), password + HalChatFunctionsLib.bytesToHex(msg.encryptId), 10, 100000, ""), StandardCharsets.UTF_8);
                        newVariants.put(vr);
                    }

                    msg.data.put("variants",newVariants);
                } catch (Exception e) {
                    Log.e(TAG,"Failed to decrypt poll:",e);
                }
            }
            return msg;
        }
        try {
            msg.setDecryptedMessage(AESGCMHelper.decrypt(HalChatFunctionsLib.hexStringToByteArray(msg.message),msg.chatId));

            if(hasPoll) {
                Log.e(TAG,"DEcrypt variants");
                try {
                    JSONArray oldVariants = msg.data.getJSONArray("variants");
                    JSONArray newVariants = new JSONArray();

                    for(int i=0;i<oldVariants.length();i++) {
                        String vr=AESGCMHelper.decrypt(HalChatFunctionsLib.hexStringToByteArray(oldVariants.getString(i)),msg.chatId);
                        newVariants.put(vr);
                    }

                    msg.data.put("variants",newVariants);
                } catch (Exception e) {
                    Log.e(TAG,"Failed to decrypt poll:",e);
                }
            }

            return msg;
        } catch (Exception e) {
            //Log.e(TAG,"Error in decryptMessage",e);
        }
        msg.setDecryptedMessage(new String(hc.he.decodeByHash(HalChatFunctionsLib.hexStringToByteArray(msg.message), password + HalChatFunctionsLib.bytesToHex(msg.encryptId), 10, 100000, ""), StandardCharsets.UTF_8));

        if(hasPoll) {
            try {
                JSONArray oldVariants = msg.data.getJSONArray("variants");
                JSONArray newVariants = new JSONArray();

                for(int i=0;i<oldVariants.length();i++) {
                    String vr=new String(hc.he.decodeByHash(HalChatFunctionsLib.hexStringToByteArray(oldVariants.getString(i)), password + HalChatFunctionsLib.bytesToHex(msg.encryptId), 10, 100000, ""), StandardCharsets.UTF_8);
                    newVariants.put(vr);
                }

                msg.data.put("variants",newVariants);
            } catch (Exception e) {
                Log.e(TAG,"Failed to decrypt poll:",e);
            }
        }

        return msg;
    }

    protected String encryptMessage(String msg,String password,byte[] encryptId) {
        if(msg=="")return "";
        byte[] enc=hc.he.encodeByHash(msg.getBytes(StandardCharsets.UTF_8),password+HalChatFunctionsLib.bytesToHex(encryptId),10,100000,"");
        StringBuilder out=new StringBuilder();
        for(int i=0;i<enc.length;i++) {
            out.append(String.format("%02x", enc[i]));
        }
        return out.toString();
    }

    //Load group chats users from network
    protected void loadChatUsers(long chatId) {
        long lastId=-1;
        int countLoad=100;
        while (true) {
            try {
                JSONObject postData=new JSONObject();
                postData.put("chatId",chatId);
                postData.put("count",countLoad);
                postData.put("lastId",lastId);
                JSONObject jsonObject=hc.hcapi.apiReq("getChatUsers",postData).get();

                if (jsonObject.getInt("errorCode") == 0) {
                    Log.e(TAG, "Successful load users");
                    JSONArray users = jsonObject.getJSONArray("users");
                    int i;
                    JSONObject user;
                    HNUser hnuser;
                    HCUser hcuser;
                    if (users.length() > 0) {
                        lastId = users.getJSONObject(users.length() - 1).getLong("uid");
                    }
                    for (i = 0; i < users.length(); i++) {
                        user = users.getJSONObject(i);
                        hnuser = new HNUser(-1, user.getLong("id"), user.getString("nickname"), user.getString("icon"), user.getInt("isBot") == 1, hc.hd.getFileById(user.getString("icon")).get().getAbsolutePath());
                        hcuser = new HCUser(-1, user.getLong("uid"), chatId, user.getLong("id"), (byte) user.getInt("permissions"), user.getInt("isJoin") == 1, hnuser);
                        hc.hnUsers.addUser(hnuser);
                        hc.chatUsers.addUser(hcuser);
                        Log.e(TAG, "Add user: " + hcuser.chatId + ":" + hcuser.toId + ":" + hcuser.id + ":" + hnuser.nickname);
                    }
                    Log.e(TAG, "LAST_ID: " + lastId);
                    if(users.length()<countLoad) {
                        break;
                    }
                }
            } catch (Exception e) {
                Log.e(TAG, "Error in loadChatUser", e);
                break;
            }
        }
    }

    protected HCChat updateLocalChatSetting(HCChat chat,UpdateChatSetting updateChatSetting) {
        switch (updateChatSetting.name) {
            case "name":
                chat.name=updateChatSetting.valueStr;
                db.execSQL("UPDATE `groupChats` SET `name`=? WHERE `uid`=?",new String[]{chat.name,String.valueOf(chat.uid)});
                break;
            case "id":
                chat.id=updateChatSetting.valueStr;
                db.execSQL("UPDATE `groupChats` SET `id`=? WHERE `uid`=?",new String[]{chat.id,String.valueOf(chat.uid)});
                break;
            case "public":
                chat.publicType=updateChatSetting.valueInt;
                db.execSQL("UPDATE `groupChats` SET `publicType`=? WHERE `uid`=?",new String[]{String.valueOf(chat.publicType),String.valueOf(chat.uid)});
                break;
            case "type":
                chat.chatType=updateChatSetting.valueInt;
                db.execSQL("UPDATE `groupChats` SET `chatType`=? WHERE `uid`=?",new String[]{String.valueOf(chat.chatType),String.valueOf(chat.uid)});
                break;
            default:
                break;
        }
        return chat;
    }

    protected void updateChatSettings(HCChat chat,List<UpdateChatSetting> updateChatSettingList, Function<HCChat,Void> endFunc) {
        TaskExecutorManager.getInstance().submitSend("updateSettingsChat:chatId:"+chat.chatUID,()->{
            HCChat nc=chat;
            for(UpdateChatSetting updateChatSetting:updateChatSettingList) {
                try {
                    String apiReq="";
                    JSONObject postData=new JSONObject();
                    switch (updateChatSetting.name) {
                        case "name":
                            apiReq="changeName";
                            postData.put("name",updateChatSetting.valueStr);
                            break;
                        case "id":
                            apiReq="changeId";
                            postData.put("newId",updateChatSetting.valueStr);
                            break;
                        case "public":
                            apiReq="changePublic";
                            postData.put("public",updateChatSetting.valueInt==1?"true":"false");
                            break;
                        case "type":
                            apiReq="changeType";
                            postData.put("type",updateChatSetting.valueInt);
                            break;
                        default:
                            break;
                    }

                    postData.put("chatId",nc.chatUID);
                    JSONObject jsonObject=hc.hcapi.apiReq(apiReq,postData).get();

                    if (jsonObject.getInt("errorCode") == 0) {
                        Log.e(TAG, "Successful update setting");
                        nc=updateLocalChatSetting(nc,updateChatSetting);
                    }
                } catch (Exception e) {
                    Log.e(TAG,"Error in updateChatSettings",e);
                    endFunc.apply(nc);
                    return null;
                }
            }
            endFunc.apply(nc);
            return null;
        });
    }

    protected HCMessage getMessageById(long chatId,long id) {
        Cursor cursor=getCursorMessageById(id);
        if(cursor==null) {
            loadMessage(chatId,id);
            return null;
        }
        HCMessage out=getMessageFromCursor(cursor,hc.idUser);
        cursor.close();
        return out;
    }

    protected void loadComments(long chatId,long msgId) {
        TaskExecutorManager.getInstance().submitChatSync("loadComments:msgId:"+msgId,()->{
            return null;
        });
    }

    protected int loadCountComments(long chatId,long msgId) {
        Cursor cursor=db.rawQuery("SELECT * FROM `groupChatsMessages` WHERE `chatId`=? AND `isDelete`=0 AND `commentMsg`=?",new String[]{String.valueOf(chatId),String.valueOf(msgId)});
        loadComments(chatId,msgId);

        int count=cursor.getCount();

        if(cursor.moveToFirst()) {
            count=cursor.getCount();
        }
        cursor.close();
        return count;
    }

    protected String generateEncryptId(long a,long b) {
        String out=Long.toHexString(a+b);
        if(out.length()<16){return "0".repeat(16-out.length())+out;}
        return out.substring(out.length()-16,out.length());
    }

    protected void setSendMessageEvent(HCGCSendMessage sendMessageEvent) {
        this.sendMessageEvent=sendMessageEvent;
    }

    protected CompletableFuture<JSONObject> sendMessage(HCMessage msg, List<String> variants) {
        String encryptId;
        if(!msg.isHalEnc) {
            encryptId = generateEncryptId(System.currentTimeMillis(), msg.chatId);
            msg.encryptId = HalChatFunctionsLib.hexStringToByteArray(encryptId);
            msg.message = encryptMessage(msg.decryptedMessage, getPasswordChat(msg.chatId), msg.encryptId);
        } else {
            encryptId=HalChatFunctionsLib.bytesToHex(msg.encryptId);
        }

        if(variants!=null&&variants.size()>1&&variants.size()<13) {

            try {
                JSONArray variantsJSON = new JSONArray();

                for (String vr : variants) {
                    variantsJSON.put(encryptMessage(vr, getPasswordChat(msg.chatId), msg.encryptId));
                }

                JSONObject postData = new JSONObject();
                postData.put("message", msg.message);
                postData.put("encryptId", encryptId);
                postData.put("chatId", msg.chatId);
                postData.put("variants",variantsJSON);

                msg.type=4;

                return hc.hcapi.apiReq("sendPoll", postData);
            } catch (Exception e) {
                Log.e(TAG,"Failed to send poll: ",e);
                return CompletableFuture.completedFuture(null);
            }
        }

        try {
            JSONObject postData=new JSONObject();
            postData.put("message",msg.message);
            postData.put("attachments",msg.attachments.toString());
            postData.put("encryptId",encryptId);
            postData.put("chatId",msg.chatId);
            postData.put("commentMsg",msg.commentMsg);
            postData.put("soundMsg",-1);
            if(!msg.recordMic.equals("-1")) {
                postData.put("recordMic",msg.recordMic);
            }

            if(msg.answerMsg!=-1) {
                postData.put("answerMsg",msg.answerMsg);
            }

            Log.e(TAG,"PostData: "+ postData);

            return hc.hcapi.apiReq("sendMessage",postData);
        } catch (Exception e) {
            Log.e(TAG, "Error in sendMessage", e);
        }
        return CompletableFuture.completedFuture(null);
    }

    protected void addToSendMessage(HCMessage msg) {
        String encryptId = generateEncryptId(System.currentTimeMillis(), msg.chatId);
        msg.encryptId = HalChatFunctionsLib.hexStringToByteArray(encryptId);
        msg.message = encryptMessage(msg.decryptedMessage, getPasswordChat(msg.chatId), msg.encryptId);

        db.execSQL("INSERT INTO `sendMessages` (`chatId`, `fromId`, `answerMsg`, `commentMsg`, `message`, `soundMsg`, `dataBot`, `recordMic`, `encryptId`," +
                "`type`, `data`, `shareId`, `pixelId`, `v`) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",new String[]{String.valueOf(msg.chatId),String.valueOf(msg.fromId),
        String.valueOf(msg.answerMsg),String.valueOf(msg.commentMsg),msg.message,String.valueOf(msg.soundMsg),msg.dataBot,msg.recordMic,HalChatFunctionsLib.bytesToHex(msg.encryptId),
        String.valueOf(msg.type),msg.data==null?"NULL":msg.data.toString(),String.valueOf(msg.shareId),String.valueOf(msg.pixelId),String.valueOf(msg.v)});
    }

    protected CompletableFuture<JSONObject> sendPixel(Pixel pixel, long chatId, long commentMsg) {
        try {
            JSONObject postData=new JSONObject();
            postData.put("pixelId",pixel.pixelId);
            postData.put("commentMsg",commentMsg);
            postData.put("chatId",chatId);

            Log.e(TAG,"PostData: "+ postData);

            return hc.chatWS.apiReq("sendPixel",postData);
        } catch (Exception e) {
            Log.e(TAG, "Error in sendMessage", e);
        }
        return CompletableFuture.completedFuture(null);
    }

    protected CompletableFuture<HCFile> uploadChatFile(File file, String contentType, long chatId, int uidf) {
        CompletableFuture<HCFile> future=new CompletableFuture<>();
        TaskExecutorManager.getInstance().submitDecryptChatActivity("downloadFile:chatId" + chatId + ":" + uidf, () -> {
            try {
                HttpUrl url = HttpUrl.parse("https://haldrive.halwarsing.net/api").newBuilder()
                        .addQueryParameter("req", "uploadHalChatFile")
                        .addQueryParameter("chatId", String.valueOf(chatId))
                        .build();

                HCFile hcFileProgress = new HCFile("-1", hc.hd.getFileIcon(hc.hd.getFormatType(contentType)), file.getName());

                ProgressRequestBody fileBody = new ProgressRequestBody(file, contentType, percentage -> {
                    fileUploadEvent.onProgress(hcFileProgress, uidf, percentage);
                });

                MultipartBody requestBody = new MultipartBody.Builder()
                        .setType(MultipartBody.FORM)
                        .addFormDataPart("file", file.getName(), fileBody)
                        .build();

                Request request = new Request.Builder()
                        .url(url)
                        .addHeader("Cookie", "uid=" + codeUser)
                        .post(requestBody)
                        .build();

                CompletableFuture<JSONObject> jfut=new CompletableFuture<>();
                hc.hdapi.executeRequest(request,jfut);

                jfut.thenAccept(jsonObject->{
                    try {
                        if (jsonObject.getInt("errorCode") == 0) {
                            JSONObject data = jsonObject.getJSONObject("data");
                            HCFile hcf = new HCFile(
                                    data.getString("id"),
                                    hc.hd.getFileIcon(hc.hd.getFormatType(contentType)),
                                    data.getString("name")
                            );

                            hc.hd.addLocalFile(data.getString("path"),data.getString("name"),hc.idUser,data.getLong("updated"),file,data.getString("id"),
                                    data.getString("mimeType"),data.getString("fileType"),data.getString("imageData"));

                            fileUploadEvent.onUpload(hcf, uidf);

                            future.complete(hcf);
                        } else {
                            Log.e(TAG, "API Error: " + jsonObject.getString("error"));
                            HCFile hcf=new HCFile("-1",-1,"");
                            hcf.disabled=true;
                            fileUploadEvent.onUpload(hcf, uidf);
                        }
                    } catch (JSONException e) {
                        HCFile hcf=new HCFile("-1",-1,"");
                        hcf.disabled=true;
                        fileUploadEvent.onUpload(hcf, uidf);
                        Log.e(TAG,"jfut",e);
                    }
                });
            } catch (Exception e) {
                Log.e(TAG, "Failed to fetch HalDrive API", e);
            }
            return null;
        });

        return future;
    }

    protected void setOnUploadFile(HCFileUploadEvent fileUploadEvent) {
        this.fileUploadEvent=fileUploadEvent;
    }

    protected HCMessage deencryptMessage(HCMessage msg,String password) {
        if(!msg.isHalEnc){
            return msg;}
        if(msg.message.isEmpty()){
            msg.isHalEnc=false;
            if(msg.uid!=-1) {
                db.execSQL("UPDATE `groupChatsMessages` SET `isHalEnc`=0 WHERE `uid`=?", new String[]{String.valueOf(msg.uid)});
            }
            return msg;
        }
        if(!msg.isDecrypted) {
            msg=decryptMessage(msg, password);
        }

        try {
            String newencryptmsg= AESGCMHelper.encrypt(msg.decryptedMessage,msg.chatId);
            msg.message=newencryptmsg;

            if(msg.type==4&&msg.data!=null&&msg.data.has("variants")) {
                try {
                    JSONArray oldVariants = msg.data.getJSONArray("variants");
                    JSONArray newVariants = new JSONArray();
                    for (int i=0;i<oldVariants.length();i++) {
                        String vr=oldVariants.getString(i);
                        Log.e(TAG,vr);
                        newVariants.put(AESGCMHelper.encrypt(vr,msg.chatId));
                    }
                    msg.data.put("variants",newVariants);

                    if(msg.uid!=-1) {
                        db.execSQL("UPDATE `groupChatsMessages` SET `data`=? WHERE `uid`=?", new String[]{msg.data.toString(), String.valueOf(msg.uid)});
                    }
                } catch (JSONException e) {
                    Log.e(TAG,"Failed to deencrypt poll:",e);
                }
            }

            msg.isHalEnc=false;
            if(msg.uid!=-1) {
                db.execSQL("UPDATE `groupChatsMessages` SET `message`=?, `isHalEnc`=0 WHERE `uid`=?", new String[]{newencryptmsg, String.valueOf(msg.uid)});
            }
        } catch (Exception e) {
            Log.e(TAG,"Error in deencryptMessage",e);
        }
        return msg;
    }

    protected void addRequestPassword(long chatId,String password) {
        if(getRequestPassword(chatId)!=null) {
            db.execSQL("UPDATE `requestsPassword` SET `psw`=? WHERE `chatId`=?", new String[]{
                    password,
                    String.valueOf(chatId)
            });
            return;
        }
        db.execSQL("INSERT INTO `requestsPassword` (`chatId`, `psw`) VALUES (?, ?)", new String[]{
                String.valueOf(chatId),
                password
        });
    }

    protected String getRequestPassword(long chatId) {
        String out=null;
        Cursor cursor=db.rawQuery("SELECT * FROM `requestsPassword` WHERE `chatId`=?",new String[]{String.valueOf(chatId)});
        if (cursor.moveToFirst()) {
            out=cursor.getString(2);
        }
        cursor.close();
        return out;
    }

    protected void removeRequestPassword(long chatId) {
        db.execSQL("DELETE FROM `requestsPassword` WHERE `chatId`=?",new String[]{String.valueOf(chatId)});
    }

    protected void deleteChat(long chatId) {
        db.execSQL("DELETE FROM `groupChats` WHERE `chatUID`=?", new String[]{String.valueOf(chatId)});
        db.execSQL("DELETE FROM `groupChatsUsers` WHERE `chatId`=?",new String[]{String.valueOf(chatId)});
        db.execSQL("DELETE FROM `groupChatsMessages` WHERE `chatId`=?",new String[]{String.valueOf(chatId)});
    }

    protected CompletableFuture<Boolean> exitChat(long chatId) {
        if(!hasChatInDB(chatId)) {
            return CompletableFuture.completedFuture(false);
        }
        try {
            JSONObject postData = new JSONObject();
            postData.put("chatId", chatId);

            return hc.hcapi.apiReq("exitChat",postData).thenCompose(data -> {
                try {
                    if (data.getInt("errorCode") != 0) {
                        Log.e(TAG, "exitChat: " + data.getString("error") + ";" + data.getInt("errorCode"));
                        return CompletableFuture.completedFuture(false);
                    }
                    deleteChat(chatId);
                    //NAF: deleteMessages
                    return CompletableFuture.completedFuture(true);
                } catch (Exception e) {
                    Log.e(TAG, "exitChat", e);
                }
                return CompletableFuture.completedFuture(false);
            });
        } catch (Exception e) {
            Log.e(TAG,"Error in exitChat",e);
        }
        return CompletableFuture.completedFuture(false);
    }

    //Polls
    protected long getVotesPollVariant(long chatId, long msgId, long variant) {
        Cursor cursor=db.rawQuery("SELECT * FROM `groupChatsPollVotes` WHERE `chatId`=? AND `msgId`=? AND `variant`=?",new String[]{String.valueOf(chatId),
        String.valueOf(msgId),String.valueOf(variant)});
        long out=0;
        if(cursor.moveToFirst()) {
            out=cursor.getLong(cursor.getColumnIndexOrThrow("votes"));
        }
        cursor.close();
        return out;
    }

    protected void updateVotesPollVariant(long chatId, long msgId, long variant, long votes) {
        Cursor cursor=db.rawQuery("SELECT * FROM `groupChatsPollVotes` WHERE `chatId`=? AND `msgId`=? AND `variant`=?",new String[]{String.valueOf(chatId),String.valueOf(msgId),String.valueOf(variant)});
        if(!cursor.moveToFirst()) {
            cursor.close();
            db.execSQL("INSERT INTO `groupChatsPollVotes` (`chatId`, `msgId`, `variant`, `votes`) VALUES (?, ?, ? ,?)",new String[]{String.valueOf(chatId),String.valueOf(msgId),
            String.valueOf(variant),String.valueOf(votes)});

            return;
        }
        cursor.close();

        db.execSQL("UPDATE `groupChatsPollVotes` SET `votes`=? WHERE `chatId`=? AND `msgId`=? AND `variant`=?",new String[]{String.valueOf(votes),String.valueOf(chatId),
        String.valueOf(msgId),String.valueOf(variant)});
    }

    protected CompletableFuture<Boolean> updateAllPollsChat(long chatId) {
        CompletableFuture<Boolean> future=new CompletableFuture<>();
        TaskExecutorManager.getInstance().submitChatSync("getResultPoll:chatId:"+chatId,()->{
            Cursor cursor=db.rawQuery("SELECT * FROM `groupChatsMessages` WHERE `chatId`=? AND `type`=4 AND `isDelete`=0",new String[]{String.valueOf(chatId)});
            if(cursor.moveToFirst()) {
                do {
                    try {
                        JSONObject postData = new JSONObject();
                        final long msgId=cursor.getLong(cursor.getColumnIndexOrThrow("msgId"));
                        postData.put("chatId", chatId);
                        postData.put("msgId", msgId);
                        JSONObject data=hc.hcapi.apiReq("getResultPoll",postData).get();
                        if(data.getLong("errorCode")==0) {
                            JSONArray result=data.getJSONArray("result");
                            for(int i=0;i<result.length();i++) {
                                updateVotesPollVariant(chatId,msgId,i,result.getLong(i));
                            }
                        } else {
                            Log.e(TAG,"Error api getResultPoll: "+data.getString("errorCode")+";"+data.getLong("errorCode"));
                        }
                    } catch (Exception e) {
                        Log.e(TAG,"Error json:",e);
                    }
                } while (cursor.moveToNext());
            }
            cursor.close();
            future.complete(true);
            return null;
        });

        return future;
    }

    protected CompletableFuture<Boolean> updatePoll(long chatId, long msgId) {
        CompletableFuture<Boolean> future=new CompletableFuture<>();

        try {
            JSONObject postData=new JSONObject();
            postData.put("chatId",chatId);
            postData.put("msgId",msgId);
            hc.chatWS.apiReq("getResultPoll",postData).thenAccept(data->{
                try {
                    if(data.getLong("errorCode")==0) {
                        JSONArray result=data.getJSONArray("result");
                        for(int i=0;i<result.length();i++) {
                            updateVotesPollVariant(chatId,msgId,i,result.getLong(i));
                        }

                        future.complete(true);

                        return;
                    } else {
                        Log.e(TAG,"Error api getResultPoll: "+data.getString("error")+";"+data.getLong("errorCode"));
                    }
                } catch (JSONException e) {
                    Log.e(TAG,"getResultPoll:",e);
                }
                future.complete(false);
            });
        } catch (JSONException e) {
            Log.e(TAG,"updatePoll:",e);
            future.complete(false);
        }

        return future;
    }

    protected CompletableFuture<Boolean> votePollVariant(long chatId, long msgId, long variant) {
        CompletableFuture<Boolean> future=new CompletableFuture<>();

        try {
            JSONObject postData = new JSONObject();
            postData.put("chatId", chatId);
            postData.put("msgId", msgId);
            postData.put("variant", variant);

            hc.chatWS.apiReq("votePoll", postData).thenAccept(data -> {
                try {
                    if (data.getLong("errorCode")==0) {
                        updatePoll(chatId,msgId).thenAccept(b->{
                            future.complete(true);
                        });
                        return;
                    } else if(data.getLong("errorCode")==4) {
                        String err=data.getString("error");
                        new Handler(Looper.getMainLooper()).post(() -> {
                            Toast.makeText(hc.context, err, Toast.LENGTH_SHORT).show();
                        });
                    } else {
                        Log.e(TAG,"Error api votePoll: "+data.getString("error")+";"+data.getLong("errorCode"));
                    }
                } catch (JSONException e) {
                    Log.e(TAG,"votePoll:",e);
                }
                future.complete(false);
            });
        } catch (Exception e) {
            Log.e(TAG,"votePolLVariant:",e);
            future.complete(false);
        }

        return future;
    }

    public interface HalChatAddMessage {
        void addMessages(List<HCMessage> messages);
    }

    public interface HCGCSendMessage {
        void onSended(HCMessage msg);
    }

    public interface HalChatLoadChats {
        void onLoadChat(ChatInfoList chatInfoList);
    }

    public static class HCEncryptedMessage {
        public String text;
        public boolean isEncrypted;
        public HCMessage msg;

        public HCEncryptedMessage(String text, boolean isEncrypted,HCMessage msg) {
            this.text=text;
            this.isEncrypted=isEncrypted;
            this.msg=msg;
        }

        public String getMessage(HalChat hc) {
            if(isEncrypted) {
                if(!msg.isDecrypted) {
                    msg=hc.chatGroupChats.decryptMessage(msg, hc.chatGroupChats.getPasswordChat(msg.chatId));
                }
                return msg.decryptedMessage;
            }

            return text;
        }
    }
}
