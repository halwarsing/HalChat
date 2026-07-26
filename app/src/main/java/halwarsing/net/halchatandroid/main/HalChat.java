package halwarsing.net.halchatandroid.main;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.graphics.BitmapFactory;
import android.media.AudioAttributes;
import android.media.SoundPool;
import android.net.ConnectivityManager;
import android.net.Network;
import android.util.Log;
import android.webkit.CookieManager;
import android.webkit.CookieSyncManager;

import androidx.annotation.NonNull;

import org.json.JSONObject;

import java.io.IOException;
import java.lang.ref.WeakReference;
import java.net.HttpURLConnection;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.TimeUnit;

import javax.net.ssl.HttpsURLConnection;

import halwarsing.net.halchatandroid.type.ChatListI;
import halwarsing.net.halchatandroid.type.HCAction;
import halwarsing.net.halchatandroid.type.HCChat;
import halwarsing.net.halchatandroid.type.HCMessage;
import halwarsing.net.halchatandroid.type.HalChatI;
import okhttp3.OkHttpClient;

//Main class for communicate with online and offline functions in automatic mode.
//Главный класс для взаимодействия между онлайн и оффлайн функциями в автоматическом режиме. То есть это отдельный как бы процесс,
//а все остальные с ним взаимодействуют для обновления или получения информации
public class HalChat {
    private static final String TAG="HCAHC";
    protected HalHash hh;
    protected HalEncryption he;
    protected SQLiteDatabase db;
    protected HalDrive hd;
    protected HalNetUsers hnUsers;
    protected HalChatUsers chatUsers;
    protected HalChatGroupChats chatGroupChats;
    protected HalChatGroupChatsMessages chatGroupChatsMessages;
    protected long idUser;
    protected String nickname;
    protected String icon;
    protected String codeUser;
    protected Context context;
    protected boolean isLogIn=false;
    protected HalChatI chatInterface=null;
    protected HalChatWS chatWS=null;
    protected ChatListI chatListI=null;
    protected PasswordSync passwordSync=null;
    protected HalChatSettingsApp chatSettingsApp=null;
    protected HalChatUIDSystem uidSystem;
    protected HalChatActions hcActions;
    public EmojiPixelSystem EPSystem;

    private final Map<String, List<WeakReference<HalChatEvent>>> events;

    private final HalChatEvent onReceivePasswordEvent=this::onReceivePassword;
    private final HalChatEvent onCheckPasswordEvent=this::onCheckPassword;
    private final HalChatEvent onAuthEvent=this::onAuth;
    private final HalChatEvent onNewActionEvent=this::onNewAction;
    private final HalChatEvent onNewMessageEvent=this::onNewMessage;

    //REST API
    private final OkHttpClient sharedClient;
    protected HalChatAPI hcapi;
    protected HalNetAPI hnapi;
    protected HalDriveAPI hdapi;

    //Sound
    //Play and load
    private SoundPool soundPool;
    //Map loaded sounds
    private HashMap<String, SoundID> soundMap=new HashMap<>();
    //List of sounds
    private String[] soundFiles={"resources/audio/receive_message.wav","resources/audio/send_message.wav"};

    //Class for sound
    private static class SoundID {
        protected Integer id;
        protected CompletableFuture<Void> future;

        public SoundID(Integer id) {
            this.id=id;
            this.future=new CompletableFuture<>();
        }
    }

    //Documents
    protected static final String ID_PRIVACY_DOC="5nX0HYaxaXSsACFgokRWDj9b0RQmf07e2mMbb7O47BU2EJwpFE4kny76YASxh5uOEIevUe7d87yfETKOR8bBIvH8ob4vYvrY5aIX";
    protected static final String ID_USER_AGREEMENT_DOC="x4Rm7pIV4JddZ13mDgYh1GmWPkVsrk0KHVCdQ6iCrklLX3eq3BZV5oP23Dz0hr6awQeJQDvnt7IfL9pXWfmhsKx1L0vkAodUQTkC";


    protected HalChat() {
        hh=new HalHash();
        he=new HalEncryption();

        sharedClient=new OkHttpClient.Builder()
                .retryOnConnectionFailure(true)
                .connectTimeout(15, TimeUnit.SECONDS)
                .readTimeout(30,TimeUnit.SECONDS)
                .build();

        this.events=new HashMap<>();
        this.events.put("onAuth",new ArrayList<>());
        this.events.put("onNewAction",new ArrayList<>());
        this.events.put("onNewMessage",new ArrayList<>());
        this.events.put("onNewVoiceAction",new ArrayList<>());
        this.events.put("onClose",new ArrayList<>());
        this.events.put("onReceivePassword",new ArrayList<>());
        this.events.put("onCheckPassword",new ArrayList<>());
        this.events.put("onNewChat",new ArrayList<>());

        addEventListener("onNewAction",onNewActionEvent);
        addEventListener("onNewMessage",onNewMessageEvent);
    }

    protected void addEventListener(String name, HalChatEvent event) {
        if(events.containsKey(name)) {
            events.get(name).add(new WeakReference<>(event));

            if(name.equals("onAuth")&&chatWS!=null&&chatWS.isAuth) {
                event.onEvent(new JSONObject());
            }
        }
    }

    public void removeEventListener(String name, HalChatEvent event) {
        if (events.containsKey(name)) {
            List<WeakReference<HalChatEvent>> listEv = events.get(name);
            Iterator<WeakReference<HalChatEvent>> iterator = listEv.iterator();
            while (iterator.hasNext()) {
                WeakReference<HalChatEvent> ref = iterator.next();
                HalChatEvent ev = ref.get();
                if (ev == null || ev == event) {
                    iterator.remove();
                }
            }
        }
    }

    protected void runEvent(String name,Object... args) {
        if(events.containsKey(name)) {
            List<WeakReference<HalChatEvent>> listEv=events.get(name);

            Iterator<WeakReference<HalChatEvent>> iterator=listEv.iterator();
            while(iterator.hasNext()) {
                WeakReference<HalChatEvent> ref=iterator.next();
                HalChatEvent ev=ref.get();

                if(ev!=null) {
                    ev.onEvent(args);
                } else {
                    iterator.remove();
                }
            }
        }
    }

    //Авторизация и инициализация баз данных и всех классов для взаимодействия с данными
    boolean init(Context context) {
        this.context=context;
        HalChatDatabaseHelper dbHelper=new HalChatDatabaseHelper(context);
        db=dbHelper.getWritableDatabase();
        Cursor sessionCur=db.rawQuery("SELECT * FROM sessions WHERE selected=1",new String[0]);
        if (sessionCur.moveToFirst()) {
            Log.d(TAG, "Successful find session");
            Cursor userCur=db.rawQuery("SELECT * FROM users WHERE id=?",new String[]{String.valueOf(sessionCur.getInt(2))});
            if (userCur.moveToFirst()) {
                Log.d(TAG, "Successful find user");
                idUser=userCur.getLong(1);
                nickname=userCur.getString(2);
                icon=userCur.getString(3);
                codeUser=sessionCur.getString(1);
                hd=new HalDrive(context,db,codeUser);
                hnUsers=new HalNetUsers(this,db,hd);
                chatUsers=new HalChatUsers(db,this);
                chatGroupChats=new HalChatGroupChats(db,codeUser,idUser,this);
                chatGroupChatsMessages=new HalChatGroupChatsMessages(db);
                passwordSync=new PasswordSync(this);
                chatSettingsApp=new HalChatSettingsApp(this);
                uidSystem=new HalChatUIDSystem(db);
                hcActions=new HalChatActions(db,codeUser,idUser,this);
                EPSystem=new EmojiPixelSystem(db,this);

                //API
                hcapi=new HalChatAPI(sharedClient,codeUser);
                hnapi=new HalNetAPI(sharedClient,codeUser);
                hdapi=new HalDriveAPI(sharedClient,codeUser);

                this.isLogIn=true;

                //Internet connect status
                ConnectivityManager.NetworkCallback networkCallback = new ConnectivityManager.NetworkCallback() {
                    @Override
                    public void onAvailable(@NonNull Network network) {
                        super.onAvailable(network);
                        chatGroupChats.checkSendMessages();
                        chatGroupChats.checkChats();
                        hcActions.syncActions();
                    }

                    @Override
                    public void onLost(@NonNull Network network) {
                        super.onLost(network);
                    }
                };

                ConnectivityManager connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
                connectivityManager.registerDefaultNetworkCallback(networkCallback);


                //check downloaded documents
                if(!hd.isFileExists(ID_USER_AGREEMENT_DOC)) {
                    hd.addHalDriveFile(ID_USER_AGREEMENT_DOC);
                }

                if(!hd.isFileExists(ID_PRIVACY_DOC)) {
                    hd.addHalDriveFile(ID_PRIVACY_DOC);
                }

                //Events
                addEventListener("onReceivePassword", onReceivePasswordEvent);
                addEventListener("onCheckPassword", onCheckPasswordEvent);


                addEventListener("onAuth", onAuthEvent);

                //Sound
                soundPool=new SoundPool.Builder()
                        .setMaxStreams(5)
                        .setAudioAttributes(
                                new AudioAttributes.Builder()
                                        .setUsage(AudioAttributes.USAGE_MEDIA)
                                        .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                                        .build()
                        ).build();
                preloadSounds();

                checkAllDatas();
                hd.checkUsersIconsIsDownloaded();
                sessionCur.close();
                userCur.close();

                return true;
            } else {
                Log.d(TAG,"Error find user");
                sessionCur.close();
                userCur.close();
            }
        } else {
            Log.d(TAG,"No sessions");
            sessionCur.close();
        }
        this.isLogIn=false;
        return false;
    }

    private void onAuth(Object... data) {
        TaskExecutorManager.getInstance().cancelTasks("checknrequestpasswords");

        //TODO FIX FRIZES
        chatGroupChats.checkSendMessages();
        chatGroupChats.checkChats();
        hcActions.syncActions();

        TaskExecutorManager.getInstance().submitPasswordT("checknrequestpasswords",
                ()->{
                    passwordSync.checkAllAvailablePasswords();
                    passwordSync.requestMissingPasswords();
                    passwordSync.checkRequestsPasswords();
                    return null;
                }
        );
    }

    private void onReceivePassword(Object... data) {
        passwordSync.onReceivePassword((JSONObject) data[0]);
    }

    private void onCheckPassword(Object... data) {
        passwordSync.onCheckPassword((JSONObject) data[0]);
    }

    //Sound

    private void preloadSounds() {
        AssetManager assetManager= context.getAssets();

        for(String filename:soundFiles) {
            try {
                AssetFileDescriptor fd=assetManager.openFd(filename);
                int soundId=soundPool.load(fd,1);
                soundMap.put(filename, new SoundID(soundId));
            } catch (IOException e) {
                Log.e(TAG,"Error in preloadSounds",e);
            }
        }

        soundPool.setOnLoadCompleteListener((pool,sampleId,status)->{
            Collection<SoundID> soundIDS=soundMap.values();
            for(SoundID soundID:soundIDS) {
                if(soundID.id==sampleId) {
                    soundID.future.complete(null);
                }
            }
        });
    }

    public void playSound(String filename) {
        SoundID soundID=soundMap.get(filename);
        if(soundID!=null) {
            soundID.future.thenAccept(v->{
                soundPool.play(soundID.id,1f,1f,1,0,1f);
            });
        }
    }

    //Data

    private void checkAllDatas() {
        //Проверка на новые данные онлайн
        /*Intent serviceIntent=new Intent(context, HalChatOnlineSync.class);
        Log.e(TAG,"start foreground service");
        context.startForegroundService(serviceIntent);*/
        EPSystem.sync();
        hd.downloadFiles();
        //chatGroupChats.checkChats();
        //chatGroupChats.loadMessages();
    }

    private void exitAccountLocal() {
        //NAF: add delete all data and show login
        db.execSQL("DELETE FROM `sessions` WHERE `fromId`=?",new String[]{String.valueOf(idUser)});
        db.execSQL("DELETE FROM `groupChats`");
        System.exit(0);
    }

    protected void exitAccount(Activity activity,Context c) {
        TaskExecutorManager.getInstance().submitSend("exit",()->{
            try {
                HttpsURLConnection connection = HalChatFunctionsLib.getHTTPSRequest("https://halwarsing.net/api/api?req=exit", codeUser);
                connection.connect();
                if (connection.getResponseCode() != HttpURLConnection.HTTP_OK) {
                    Log.e(TAG, "Server returned HTTP " + connection.getResponseCode() + " " + connection.getResponseMessage());
                    return null;
                }
                Log.e(TAG,"Successful exit account");
            } catch (Exception e) {
                Log.e(TAG,"Error in exitAccount",e);
            }

            activity.runOnUiThread(() -> {
                CookieSyncManager.createInstance(c);
                CookieManager cookieManager = CookieManager.getInstance();
                cookieManager.removeAllCookies(value -> exitAccountLocal());
            });
            return null;
        });
    }

    //CHAT INTERFACE
    protected void setChatInterface(HalChatI chatInterface) {
        this.chatInterface=chatInterface;
    }

    protected void onNewAction(Object... args) {
        HCAction action=(HCAction)args[1];
        try {
            int type=action.type;
            if(type==0)  {
                chatGroupChats.deleteMessageById(action.fromMsg);
            } else if((type==1||type==17)&&action.jdata.has("newmsg")) {
                action.newmsg=chatGroupChats.editMessage(chatGroupChats.jsonMsgToHCMSG(action.jdata.getJSONObject("newmsg")));
            } else if(type==3) {
                JSONObject pdata=new JSONObject();
                pdata.put("chatId",action.fromChat);
                hcapi.apiReq("joinChatById",pdata);

                if(chatGroupChats.hasChatInDB(action.fromChat)) {
                    passwordSync.requestMissingPassword(action.fromChat);
                    runEvent("onNewChat",chatGroupChats.getChatInfo(action.fromChat));
                } else {
                    chatGroupChats.addNewChat(action.fromChat).thenAccept(v->{
                        passwordSync.requestMissingPassword(action.fromChat);
                        runEvent("onNewChat",chatGroupChats.getChatInfo(action.fromChat));
                    });
                }

            }
            hcActions.processAction(action);
            //if(chatInterface==null)return;
            //chatInterface.onNewAction(action);
        } catch (Exception e) {
            Log.e(TAG,"onNewAction",e);
        }
    }

    protected void onNewMessage(Object... args) {
        try {
            //HCMessage msg=chatGroupChats.jsonMsgToHCMSG(data);
            HCMessage msg=(HCMessage)args[1];
            if(!chatGroupChats.addMessageToChatIfAbsent(msg)) {
                return;
            }
            msg=chatGroupChats.decryptMessage(msg,chatGroupChats.getPasswordChat(msg.chatId));

            if (chatListI != null) {
                chatListI.onNewMessage(chatGroupChats.getChatInfo(msg.chatId),msg);
            }

            if(msg.fromId!=idUser) {
                if(chatInterface!=null){
                    if(!chatInterface.onNewMessage(msg)) {
                        notifNewMessage(msg);
                    }
                } else {
                    notifNewMessage(msg);
                }
            } else if(chatInterface!=null){
                chatInterface.onNewMessage(msg);
            }
        } catch (Exception e) {
            Log.e(TAG,"onNewMessage",e);
        }
    }

    protected void notifNewMessage(HCMessage msg) {
        //if(msg.fromId==idUser)return;
        if(chatSettingsApp.getBoolean(HalChatSettingsApp.KEY_MUTE_NOTIFICATIONS))return;
        Log.e(TAG,"NotifNewMsg");
        try {
            HCChat chat=chatGroupChats.getChatInfo(msg.chatId);
            if(chat.chatType==2) {
                hd.getFileById(chat.icon).thenAccept(file->{
                    NotificationHelper.showChatNotification(context,msg,chat,file.getAbsolutePath());
                });
            } else {
                hnUsers.getUserByUserId(msg.fromId,true).thenAccept(fromUser->{
                    hd.getFileById(fromUser.icon).thenAccept(file->{
                        fromUser.loadIcon(BitmapFactory.decodeFile(file.getAbsolutePath()));
                        hd.getFileById(chat.icon).thenAccept(file2->{
                            NotificationHelper.showNotification(context,msg,chat,fromUser,file2.getAbsolutePath());
                        });
                    });
                });
            }
        } catch (Exception e) {
            Log.e(TAG,"notifNewMessage",e);
        }
    }

    //CHAT LIST INTERFACE
    protected void setChatListInterface(ChatListI chatListI) {
        this.chatListI=chatListI;
    }

    protected void onNewChat(HCChat chat) {
        try {
            if (chatListI == null) {return;}
            chatListI.onNewChat(chat);
        } catch (Exception e) {
            Log.e(TAG,"onNewChat",e);
        }
    }

    protected void onDeleteChat(HCChat chat) {
        try {
            if (chatListI == null) {return;}
            chatListI.onDeleteChat(chat);
        } catch (Exception e) {
            Log.e(TAG,"onDeleteChat",e);
        }
    }

    protected void destroy() {
        soundPool.release();
        soundPool=null;
    }

    @FunctionalInterface
    public interface HalChatEvent {
        void onEvent(Object... args);
    }
}
