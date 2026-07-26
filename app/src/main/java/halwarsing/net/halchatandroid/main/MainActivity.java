package halwarsing.net.halchatandroid.main;

import android.Manifest;
import android.annotation.SuppressLint;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ShortcutInfo;
import android.content.pm.ShortcutManager;
import android.graphics.drawable.Icon;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.PowerManager;
import android.provider.Settings;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.PopupWindow;

import androidx.activity.EdgeToEdge;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.recyclerview.widget.DividerItemDecoration;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;


import org.json.JSONException;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.type.ChatListI;
import halwarsing.net.halchatandroid.type.HCChat;
import halwarsing.net.halchatandroid.type.HCMessage;
import halwarsing.net.halchatandroid.type.ShareData;
import halwarsing.net.halchatandroid.type.ShareDataFiles;
import halwarsing.net.halchatandroid.type.ShareDataText;

//Главная страница со списком всех чатов
public class MainActivity extends AppCompatActivity {
    private static final int REQUEST_CODE_PERMISSIONS=322;
    private static final int REQUEST_CODE_PERMISSIONS_NEW=345;
    private static final int REQUEST_CODE_MANAGE_EXTERNAL_STORAGE=321;
    private static final String TAG="HCAMAIN";
    protected HalChat hc=null;
    private RecyclerView chatRecyclerView;
    private ChatListI chatListI;
    private ChatListAdapter chatListAdapter=null;
    private ShareData shareData=null;
    private final HalChat.HalChatEvent onNewChatEvent=this::onNewChat;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);

        //startActivity(new Intent(MainActivity.this, OprosActivity.class));
        //return;

        //Toast.makeText(this, "Отключите оптимизацию батареи для стабильной работы HalChat!", Toast.LENGTH_LONG).show();

        //Intent dintent = new Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
        //dintent.setData(Uri.parse("package:" + getPackageName()));
        //startActivity(dintent);

        /*int i;
        long startTime=System.nanoTime();
        BigInteger num0=new BigInteger("14588");
        BigInteger num1=new BigInteger("532452334534512");
        BigInteger result=num0;
        for(i=0;i<10000;i++) {
            result = result.divide(num1);
        }
        //String resultStr=result.toString();
        long endTime=System.nanoTime();
        String resultStr="-1";
        Log.e(TAG,"BigInteger: "+resultStr+";"+((endTime-startTime))+"ns");

        HalHash hh=new HalHash();
        BigInteger[] test=hh.getHalfInt(new BigInteger("14588"));
        Log.e(TAG,test[0].toString()+":"+test[1].toString());*/

        /*if (Build.VERSION.SDK_INT>=Build.VERSION_CODES.TIRAMISU) {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_MEDIA_AUDIO)
                    != PackageManager.PERMISSION_GRANTED||ContextCompat.checkSelfPermission(this, Manifest.permission.READ_MEDIA_VIDEO)
                    != PackageManager.PERMISSION_GRANTED||ContextCompat.checkSelfPermission(this, Manifest.permission.READ_MEDIA_IMAGES)
                    != PackageManager.PERMISSION_GRANTED||ContextCompat.checkSelfPermission(this, Manifest.permission.POST_NOTIFICATIONS)
                    != PackageManager.PERMISSION_GRANTED||ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                    != PackageManager.PERMISSION_GRANTED||ContextCompat.checkSelfPermission(this,Manifest.permission.READ_EXTERNAL_STORAGE)
                    !=PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this,new String[]{
                        Manifest.permission.READ_MEDIA_AUDIO,
                        Manifest.permission.READ_MEDIA_VIDEO,
                        Manifest.permission.READ_MEDIA_IMAGES,
                        Manifest.permission.POST_NOTIFICATIONS,
                        Manifest.permission.WRITE_EXTERNAL_STORAGE,
                        Manifest.permission.READ_EXTERNAL_STORAGE
                },REQUEST_CODE_PERMISSIONS_NEW);
            } else {
                start();
            }
        } else {
            if (ContextCompat.checkSelfPermission(this, android.Manifest.permission.WRITE_EXTERNAL_STORAGE)
                    != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this,
                        new String[]{android.Manifest.permission.WRITE_EXTERNAL_STORAGE},
                        REQUEST_CODE_PERMISSIONS);
            } else {
                start();
            }
        }*/


        start();
    }

    private void checkPermissions() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {  // Android 13 и выше
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_MEDIA_AUDIO) != PackageManager.PERMISSION_GRANTED ||
                    ContextCompat.checkSelfPermission(this, Manifest.permission.READ_MEDIA_VIDEO) != PackageManager.PERMISSION_GRANTED ||
                    ContextCompat.checkSelfPermission(this, Manifest.permission.READ_MEDIA_IMAGES) != PackageManager.PERMISSION_GRANTED ||
                    ContextCompat.checkSelfPermission(this, Manifest.permission.POST_NOTIFICATIONS) != PackageManager.PERMISSION_GRANTED) {

                ActivityCompat.requestPermissions(this, new String[]{
                        Manifest.permission.READ_MEDIA_AUDIO,
                        Manifest.permission.READ_MEDIA_VIDEO,
                        Manifest.permission.READ_MEDIA_IMAGES,
                        Manifest.permission.POST_NOTIFICATIONS
                }, REQUEST_CODE_PERMISSIONS_NEW);
            }
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {  // Android 11 и выше
            if (!Environment.isExternalStorageManager()) {
                Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION, Uri.parse("package:" + getPackageName()));
                startActivityForResult(intent, REQUEST_CODE_MANAGE_EXTERNAL_STORAGE);
            }
        } else {  // Android 10 и ниже
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED ||
                    ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {

                ActivityCompat.requestPermissions(this, new String[]{
                        Manifest.permission.WRITE_EXTERNAL_STORAGE,
                        Manifest.permission.READ_EXTERNAL_STORAGE
                }, REQUEST_CODE_PERMISSIONS);
            }
        }

    }

    private void checkBatteryOptimization() {
        // Проверка оптимизации батареи
        PowerManager pm = (PowerManager) getSystemService(POWER_SERVICE);
        if (!pm.isIgnoringBatteryOptimizations(getPackageName())) {
            Intent tintent = new Intent(Settings.ACTION_REQUEST_IGNORE_BATTERY_OPTIMIZATIONS);
            tintent.setAction(Settings.ACTION_REQUEST_IGNORE_BATTERY_OPTIMIZATIONS);
            tintent.setData(Uri.parse("package:" + getPackageName()));
            startActivity(tintent);
        }
    }

    @SuppressLint("ScheduleExactAlarm")
    private void start() {
        HalChatApp app=(HalChatApp)getApplicationContext();
        hc=app.getHalChat();

        if(!hc.isLogIn) {
            startActivity(new Intent(MainActivity.this, RegActivity.class));
            return;
        }

        hc.chatGroupChats.checkSendMessages();
        hc.chatGroupChats.checkChats();

        checkPermissions();
        checkBatteryOptimization();

        Intent serviceIntent = new Intent(this, WebSocketService.class);
        startService(serviceIntent);

        //Intent serviceIntent=new Intent(this,HalLPService.class);
        //startService(serviceIntent);

        hc.setChatListInterface(new ChatListI() {

            @Override
            public void onNewChat(HCChat chat) {
                if(chatListAdapter!=null) {
                    //HCChat chat=hc.chatGroupChats.getChatInfo(data.getLong("fromChat"));
                    if(chat==null)return;
                    runOnUiThread(() -> {
                        try {
                            chatListAdapter.addNewChat(chat,hc.chatGroupChats.getLastMessage(chat));
                        } catch (JSONException e) {
                            Log.e(TAG,"onNewChat",e);
                        }
                    });

                }
            }

            @Override
            public void onNewMessage(HCChat chat, HCMessage message) {
                if(chatListAdapter!=null&&message!=null&&message.commentMsg==-1) {
                    runOnUiThread(() -> chatListAdapter.onNewMessage(chat,message));
                }
            }

            @Override
            public void onDeleteChat(HCChat chat) {
                if(chatListAdapter!=null) {
                    runOnUiThread(() -> chatListAdapter.deleteChat(chat));
                }
            }

            @Override
            public void onEnterChat(HCChat chat) {
                updateChatListItem(chat);
            }

            @Override
            public void onChatUpdated(HCChat chat) {
                updateChatListItem(chat);
            }
        });

        /*Intent intent = new Intent();
        String packageName = getPackageName();
        PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
        if (!pm.isIgnoringBatteryOptimizations(packageName)) {
            intent.setAction(android.provider.Settings.ACTION_REQUEST_IGNORE_BATTERY_OPTIMIZATIONS);
            intent.setData(Uri.parse("package:" + packageName));
            startActivity(intent);
        }*/

        startMainView();
        //context.startActivity(new Intent(MainActivity.this, LogInActivity.class));
    }

    private void updateChatListItem(HCChat chat) {
        if(chatListAdapter==null || chat==null) {
            return;
        }

        TaskExecutorManager.getInstance().submitDecryptChatActivity(
                "updateChatList:chatId:" + chat.chatUID,
                () -> {
                    try {
                        HCMessage lastMsg=hc.chatGroupChats.getLastMessage(chat);
                        if(lastMsg!=null && !lastMsg.isDecrypted) {
                            lastMsg=hc.chatGroupChats.deencryptMessage(
                                    lastMsg,
                                    hc.chatGroupChats.getPasswordChat(chat.chatUID)
                            );
                        }

                        HCMessage finalLastMsg = lastMsg;
                        runOnUiThread(() -> {
                            if(chatListAdapter!=null) {
                                chatListAdapter.updateChat(chat, finalLastMsg);
                            }
                        });
                    } catch (JSONException e) {
                        Log.e(TAG,"updateChatListItem",e);
                    }
                    return null;
                }
        );
    }

    protected void openChat(long uid,String name) {
        Intent intent;
        if (hc.chatGroupChats.hasPasswordChat(uid)) {
            intent=new Intent(this,ChatActivity.class);
            intent.putExtra("uid",uid);

            //ShareData
            if(shareData!=null) {
                if(shareData instanceof ShareDataText) {
                    intent.putExtra("shareDataText",((ShareDataText) shareData).getData());
                } else if(shareData instanceof ShareDataFiles) {
                    ArrayList<File> files=((ShareDataFiles) shareData).getFiles();
                    String[] filesStr=new String[files.size()];
                    for(int i=0;i<filesStr.length;i++) {
                        filesStr[i]=files.get(i).getAbsolutePath();
                    }
                    intent.putExtra("shareDataFiles",filesStr);
                }
                shareData=null;
            }

            startActivity(intent);
            return;
        }

        intent=new Intent(this, JoinChatActivity.class);
        intent.putExtra("uid",uid);
        intent.putExtra("codeUser",hc.codeUser);
        intent.putExtra("name",name);

        startActivity(intent);
        overridePendingTransition(R.anim.slide_in_right, R.anim.slide_out_left);
    }

    private void startMainView() {
        handleSharedData(getIntent());

        setContentView(R.layout.activity_main);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            int topPadding = insets.getInsets(WindowInsetsCompat.Type.systemBars()).top;
            v.setPadding(v.getPaddingLeft(), topPadding, v.getPaddingRight(), v.getPaddingBottom());
            return insets;
        });

        //Shortcut
        ShortcutManager shortcutManager = getSystemService(ShortcutManager.class);
        ShortcutInfo shortcut = new ShortcutInfo.Builder(this, "share_to_halchat")
                .setShortLabel("Отправить в HalChat")
                .setLongLabel("Отправить в HalChat")
                .setIcon(Icon.createWithResource(this, R.mipmap.ic_launcher))
                .setIntent(new Intent(Intent.ACTION_SEND)
                        .setType("text/plain")
                        .setClass(this, ShareReceiverActivity.class))
                .build();
        shortcutManager.setDynamicShortcuts(Collections.singletonList(shortcut));

        //Initialize variables
        ImageButton addChatImgBtn=findViewById(R.id.addChatImgBtn);
        ImageButton buttonMenu=findViewById(R.id.toolbar_menu);
        Toolbar toolbar=findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        getSupportActionBar().setDisplayShowTitleEnabled(false);

        //Set event on click buttons
        addChatImgBtn.setOnClickListener(v -> {
            startActivity(new Intent(MainActivity.this, AddChatActivity.class));
        });

        buttonMenu.setOnClickListener((View v0)->{
            View popupView = LayoutInflater.from(this).inflate(R.layout.main_menu, null);

            PopupWindow popupWindow = new PopupWindow(popupView,
                    LinearLayout.LayoutParams.WRAP_CONTENT,
                    LinearLayout.LayoutParams.WRAP_CONTENT,
                    true);

            LinearLayout menuSettings = popupView.findViewById(R.id.menuSettings);
            LinearLayout menuSearch=popupView.findViewById(R.id.menuSearch);

            menuSettings.setOnClickListener(v -> {
                Intent intent=new Intent(MainActivity.this,SettingsAppActivity.class);
                startActivity(intent);
                overridePendingTransition(android.R.anim.slide_in_left,android.R.anim.slide_out_right);
                popupWindow.dismiss();
            });

            menuSearch.setOnClickListener(v->{
                Intent intent=new Intent(MainActivity.this,SearchPeopleActivity.class);
                startActivity(intent);
                overridePendingTransition(android.R.anim.slide_in_left,android.R.anim.slide_out_right);
                popupWindow.dismiss();
            });

            popupWindow.showAsDropDown(buttonMenu, -buttonMenu.getWidth(), 20, Gravity.END);
        });

        chatRecyclerView=findViewById(R.id.chatRecyclerView);
        chatRecyclerView.setLayoutManager(new LinearLayoutManager(this));
        DividerItemDecoration dividerItemDecoration=new DividerItemDecoration(chatRecyclerView.getContext(), LinearLayout.VERTICAL);
        chatRecyclerView.addItemDecoration(dividerItemDecoration);
        Log.e(TAG,"START LOAD CHATS");

        chatListAdapter=new ChatListAdapter(hc.chatGroupChats.getChatInfoList(),this,hc);
        chatRecyclerView.setAdapter(chatListAdapter);
    }

    @Override
    protected void onResume() {
        super.onResume();

        hc.addEventListener("onNewChat",onNewChatEvent);
    }

    private void onNewChat(Object... args) {
        try {
            if(chatListAdapter!=null) {
                //HCChat chat=hc.chatGroupChats.getChatInfo(data.getLong("fromChat"));
                HCChat chat=(HCChat) args[0];
                if(chat==null)return;
                runOnUiThread(() -> {
                    try {
                        chatListAdapter.addNewChat(chat,hc.chatGroupChats.getLastMessage(chat));
                    } catch (JSONException e) {
                        Log.e(TAG,"onNewChat",e);
                    }
                });

            }
        } catch (Exception e) {
            Log.e(TAG,"onNewChat",e);
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        return false;
    }

    @Override
    public boolean onOptionsItemSelected(@NonNull MenuItem item) {
        return true;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_CODE_PERMISSIONS) {
            if (grantResults.length>0&&grantResults[0]==PackageManager.PERMISSION_GRANTED) {
                start();
            } else {
                finishAndRemoveTask();
            }
        } else if (requestCode==REQUEST_CODE_PERMISSIONS_NEW) {
            if(grantResults.length>3&&grantResults[0]==PackageManager.PERMISSION_GRANTED&&
            grantResults[1]==PackageManager.PERMISSION_GRANTED&&grantResults[2]==PackageManager.PERMISSION_GRANTED&&
            grantResults[3]==PackageManager.PERMISSION_GRANTED) {
                start();
            } else {
                finishAndRemoveTask();
            }
        }
    }

    //Share data

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        handleSharedData(intent);
    }

    //NAF SHARE CHAT

    private File saveTempFileFromUri(Uri fileUri) {
        try {
            //Загрузка изображения
            InputStream inputStream=getContentResolver().openInputStream(fileUri);
            String fileName = HalChatFunctionsLib.getFileNameFromUri(getContentResolver(), fileUri);
            if (fileName == null) {
                fileName = new File(fileUri.getPath()).getName(); // Резервное имя, если имя файла не удалось получить
            }
            File tempFile=new File(hc.hd.directory,fileName);

            try (OutputStream outputStream= Files.newOutputStream(tempFile.toPath())) {
                byte[] buffer=new byte[8192];
                int bytesRead;

                while((bytesRead=inputStream.read(buffer))!=-1) {
                    outputStream.write(buffer,0,bytesRead);
                }
                outputStream.flush();
            } finally {
                if(inputStream!=null) {
                    inputStream.close();
                }
            }
            return tempFile;
        }catch (IOException e) {
            Log.e(TAG,"saveTempFIleFromUri",e);
        }
        return null;
    }

    private void handleSharedData(Intent intent) {
        if(hc==null)return;

        String action=intent.getAction();
        Uri data=intent.getData();

        if(Intent.ACTION_VIEW.equals(action) && data!=null) {
            String host=data.getHost();
            String path=data.getPath();
            if(host.equals("join")) {
                //TODO: PROVERKA
                List<String> segments=data.getPathSegments();
                if(segments.size()!=2)return;

                Intent newintent=new Intent(MainActivity.this, JoinChatByLinkActivity.class);
                newintent.putExtra("chatId",segments.get(0));
                newintent.putExtra("psw",segments.get(1));
                startActivity(newintent);
            }
        } else if (intent.hasExtra("shared_text")) {
            String text = intent.getStringExtra("shared_text");
            // Отправь в чат
            shareData=new ShareDataText(text);
        } else if (intent.hasExtra("shared_uri")) {
            Uri uri = Uri.parse(intent.getStringExtra("shared_uri"));
            // Загрузка файла или предпросмотр

            if(uri!=null) {
                ArrayList<File> files = new ArrayList<>();
                files.add(saveTempFileFromUri(uri));
                shareData = new ShareDataFiles(files);
            }
        } else if (intent.hasExtra("shared_uris")) {
            ArrayList<Uri> uris = intent.getParcelableArrayListExtra("shared_uris");
            if(uris!=null) {
                // Загрузка нескольких файлов
                ArrayList<File> files = new ArrayList<>();
                for (int i = 0; i < uris.size(); i++) {
                    Uri uri = uris.get(i);
                    files.add(saveTempFileFromUri(uri));
                }
                shareData=new ShareDataFiles(files);
            }
        }
    }
}
