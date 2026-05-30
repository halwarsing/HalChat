package halwarsing.net.halchatandroid.main;

import static androidx.recyclerview.widget.RecyclerView.SCROLL_STATE_IDLE;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.Drawable;
import android.media.AudioManager;
import android.media.MediaRecorder;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.SystemClock;
import android.text.Editable;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.SpannableStringBuilder;
import android.text.TextWatcher;
import android.text.style.ImageSpan;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.GridView;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.content.res.AppCompatResources;
import androidx.core.app.ActivityCompat;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.bumptech.glide.Glide;
import com.bumptech.glide.request.target.CustomTarget;
import com.bumptech.glide.request.transition.Transition;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Objects;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.type.Emoji;
import halwarsing.net.halchatandroid.type.HCAction;
import halwarsing.net.halchatandroid.type.HCChat;
import halwarsing.net.halchatandroid.type.HCFile;
import halwarsing.net.halchatandroid.type.HCMessage;
import halwarsing.net.halchatandroid.type.HalChatI;
import halwarsing.net.halchatandroid.type.Pixel;

//Страница чата
public class ChatActivity extends AppCompatActivity {
    private HalChat hc;
    private RecyclerView messagesRecyclerView;
    private MessageAdapter messageAdapter;
    private List<HCMessage> messageList;
    private EditText messageInput;
    private long chatId;
    private long commentId;
    private static final String TAG="HCChatA";
    protected HCChat chat;
    private List<HCFile> selectedFiles=new ArrayList<>();
    private LinearLayout attachmentsSend;
    private PopupWindow emojiPopup=null;
    private boolean isEmojiMenuVisible = false;
    private LinearLayout emojiMenu,messageInputDiv;
    private ImageButton sendButton;

    //Record Audio
    private MediaRecorder mediaRecorder;
    private String audioFilePath;
    private boolean isRecording = false;
    private Handler timerHandler = new Handler();
    private long startTime = 0;
    private Runnable timerRunnable;
    private LinearLayout recordingLayout;
    private TextView timerText;
    private boolean isSendedAudio=false;
    private ImageButton pauseButton, deleteButton, sendRecordButton;
    private static final int REQUEST_RECORD_AUDIO_PERMISSION = 200;
    private boolean permissionToRecordAccepted = false;
    private final String[] permissions = {Manifest.permission.RECORD_AUDIO};

    //Answer / Edit Message
    private LinearLayout answerDiv;
    private TextView answerDivNickname,answerDivText;
    private ImageButton closeAnswerDiv;
    private int answerType=0;
    private HCMessage answerMsg=null;

    //History load
    private boolean isLoadingHistory=false;
    private boolean hasMoreHistory=true;
    private static final int VISIBLE_THRESHOLD=50; //За сколько сообщений до конца загружать историю

    //Emoji / Pixel
    private Button emojisSelectBtn, pixelsSelectBtn;
    private GridView emojiGrid, pixelGrid;

    private final HalChat.HalChatEvent onNewActionEvent=this::onNewAction;
    private final ActivityResultLauncher<String> selectMultipleFilesLauncher = registerForActivityResult(
            new ActivityResultContracts.GetMultipleContents(),
            uris -> {
                if (uris != null && !uris.isEmpty()) {
                    for (Uri uri : uris) {
                        processSelectedFile(uri);
                    }
                }
            }
    );

    private int countUploadFiles=0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        TextView nameChatText;
        ImageView iconChatView;
        ImageButton buttonBack,buttonMenu;
        LinearLayout panelChat;
        ImageButton menuButton;

        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_chat);
        /*ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars() | WindowInsetsCompat.Type.displayCutout());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });*/
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars() | WindowInsetsCompat.Type.displayCutout());
            Insets imeInsets = insets.getInsets(WindowInsetsCompat.Type.ime());

            // Если клавиатура отображается, добавляем её нижний отступ
            int bottomPadding = Math.max(systemBars.bottom, imeInsets.bottom);

            v.setPadding(systemBars.left, systemBars.top, systemBars.right, bottomPadding);
            return insets;
        });

        HalChatApp app=(HalChatApp)getApplicationContext();
        hc=app.getHalChat();
        chatId=getIntent().getLongExtra("uid",-1);
        commentId=getIntent().getLongExtra("commentId",-1);

        Log.e(TAG,"CommentID: "+commentId);

        hc.chatGroupChats.setSendMessageEvent(msg -> Log.e(TAG,"Test: "+msg.msgId));

        NotificationHelper.clearChatNotifications(this,chatId);

        //Variables
        chat=hc.chatGroupChats.getChatInfo(chatId);
        nameChatText=findViewById(R.id.toolbar_chat_name);
        iconChatView=findViewById(R.id.toolbar_chat_icon);
        buttonBack=findViewById(R.id.toolbar_back);
        buttonMenu=findViewById(R.id.toolbar_menu);
        panelChat=findViewById(R.id.toolbar_panel_chat);
        sendButton=findViewById(R.id.send_button);
        menuButton=findViewById(R.id.menu_button);
        messageInput=findViewById(R.id.message_input);
        attachmentsSend=findViewById(R.id.attachmentsSend);
        recordingLayout = findViewById(R.id.recording_layout);
        timerText = findViewById(R.id.timer_text);
        pauseButton = findViewById(R.id.pause_button);
        deleteButton = findViewById(R.id.delete_button);
        sendRecordButton = findViewById(R.id.send_record_button);
        messageInputDiv=findViewById(R.id.message_input_div);

        //Answer / Edit Message
        answerDiv=findViewById(R.id.answerDiv);
        answerDivNickname=findViewById(R.id.answerDivNickname);
        answerDivText=findViewById(R.id.answerDivText);
        closeAnswerDiv=findViewById(R.id.closeAnswerDiv);

        //Emoji / Pixels
        emojiMenu = findViewById(R.id.emoji_menu);
        emojisSelectBtn=findViewById(R.id.emojisSelectBtn);
        pixelsSelectBtn=findViewById(R.id.pixelsSelectBtn);
        emojiGrid=emojiMenu.findViewById(R.id.emoji_grid);
        pixelGrid=emojiMenu.findViewById(R.id.pixel_grid);
        setupEmojiMenu();

        //Init
        messagesRecyclerView=findViewById(R.id.messages_recycler_view);
        messageInput=findViewById(R.id.message_input);
        messageAdapter=new MessageAdapter(new ArrayList<>(),this,hc,chat.password);

        //reverse messages
        LinearLayoutManager layoutManager=new LinearLayoutManager(this);
        //layoutManager.setReverseLayout(true);
        //layoutManager.setStackFromEnd(true);
        messagesRecyclerView.setLayoutManager(layoutManager);
        messagesRecyclerView.setAdapter(messageAdapter);

        //ChatInterface
        hc.setChatInterface(new HalChatI() {

            @Override
            public boolean onNewMessage(HCMessage data) {
                if(data.chatId==chatId) {
                    runOnUiThread(() -> ChatActivity.this.onNewMessage(data));
                    return true;
                }
                return false;
            }

            @Override
            public void onLoadMessage(HCMessage message) {
                if(message.chatId==chatId) {
                    ChatActivity.this.onLoadMessage(message);
                }
            }

            @Override
            public void onDeleteMessage(long id) {
                ChatActivity.this.deleteMessageById(id);
            }

            @Override
            public void onEditMessage(HCMessage message) {
                ChatActivity.this.updateMessage(message);
            }
        });



        //Set name and icon chat
        nameChatText.setText(chat.name);
        hc.hd.getFileById(chat.icon).thenAccept(file -> {
            iconChatView.setImageBitmap(BitmapFactory.decodeFile(file.getAbsolutePath()));
        });

        //check Share Data
        checkShareData();

        //Set clickable buttons and panel
        hc.chatGroupChats.setOnUploadFile(new HalChatGroupChats.HCFileUploadEvent() {
            @Override
            public void onUpload(HCFile file,int uid) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        addFileToSend(file,uid);
                    }
                });
            }

            @Override
            public void onProgress(HCFile file, int uid, int percent) {
                setProgressFile(uid,percent);
            }
        });

        buttonBack.setOnClickListener((View v)->{
            finish();
        });
        buttonMenu.setOnClickListener((View v0)->{
            View popupView = LayoutInflater.from(this).inflate(R.layout.chat_menu, null);

            PopupWindow popupWindow = new PopupWindow(popupView,
                    LinearLayout.LayoutParams.WRAP_CONTENT,
                    LinearLayout.LayoutParams.WRAP_CONTENT,
                    true);

            LinearLayout menuChatSettings = popupView.findViewById(R.id.menuChatSettings);
            LinearLayout menuChatUsers = popupView.findViewById(R.id.menuChatUsers);
            LinearLayout menuExitChat = popupView.findViewById(R.id.menuExitChat);

            menuChatUsers.setOnClickListener(v->{
                Intent intent=new Intent(ChatActivity.this,UsersChatActivity.class);
                intent.putExtra("uid",chatId);
                startActivity(intent);
                overridePendingTransition(R.anim.slide_in_down,R.anim.stay);
                popupWindow.dismiss();
            });

            menuChatSettings.setOnClickListener(v -> {
                Intent intent=new Intent(ChatActivity.this,SettingsChatActivity.class);
                intent.putExtra("uid",chatId);
                startActivity(intent);
                overridePendingTransition(R.anim.slide_in_down,R.anim.stay);
                popupWindow.dismiss();
            });

            menuExitChat.setOnClickListener(v -> {
                //Toast.makeText(this, "Выход из чата", Toast.LENGTH_SHORT).show();
                hc.chatGroupChats.exitChat(chatId).thenAccept(d->{
                    if(d) {
                        startActivity(new Intent(ChatActivity.this,MainActivity.class));
                        finish();
                    } else {
                        Toast.makeText(ChatActivity.this,"Не получилось выйти из чата",Toast.LENGTH_SHORT).show();
                    }
                });
                popupWindow.dismiss();
            });

            popupWindow.showAsDropDown(buttonMenu, -buttonMenu.getWidth(), 20, Gravity.END);
        });

        panelChat.setOnClickListener((View v)->{
            Intent intent=new Intent(ChatActivity.this,UsersChatActivity.class);
            intent.putExtra("uid",chatId);
            startActivity(intent);
            overridePendingTransition(R.anim.slide_in_down,R.anim.stay);
        });

        messageInput.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {

            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                if (s.toString().trim().isEmpty()) {
                    sendButton.setImageResource(R.drawable.ic_microphone);
                } else if(selectedFiles.isEmpty()) {
                    sendButton.setImageResource(R.drawable.ic_send);
                }
            }

            @Override
            public void afterTextChanged(Editable s) {

            }
        });

        //Pixel / Emoji
        pixelsSelectBtn.setOnClickListener((v)->{
            emojiGrid.setVisibility(View.GONE);
            pixelGrid.setVisibility(View.VISIBLE);
        });

        emojisSelectBtn.setOnClickListener((v)->{
            pixelGrid.setVisibility(View.GONE);
            emojiGrid.setVisibility(View.VISIBLE);
        });

        // Добавляем обработчик нажатия на кнопку
        sendButton.setOnClickListener(this::sendMessage);

        menuButton.setOnClickListener(this::showPopupMenu);

        //Chat Sync
        TaskExecutorManager.getInstance().submitDecryptChatActivity("main:chatId:"+chat.chatUID,()->{

            //Messages list
            messageList=hc.chatGroupChats.getChatLastMessages(chatId,commentId,100,false);

            hc.chatGroupChats.decryptMessagesAndAdd(this,chatId, messageList, new HalChatGroupChats.HalChatAddMessage() {
                @Override
                public void addMessages(List<HCMessage> messages) {
                    /*int i;
                    for(i=0;i<messages.size();i++) {
                        HCMessage msg=messages.get(i);
                        hc.hnUsers.getUserByUserId(msg.fromId).thenAccept(hnUser -> {
                            hc.hd.getFileById(hnUser.icon).thenAccept(file->{
                                msg.setFromIcon(BitmapFactory.decodeFile(file.getAbsolutePath()));
                                //messageAdapter.addMessageToTop(msg);
                                messageAdapter.addMessageLoaded(msg);
                            });
                        });
                    }*/

                    messageAdapter.addMessagesBatch(messages);
                    messagesRecyclerView.scrollToPosition(messageAdapter.messageList.size() - 1);
                    hc.chatGroupChats.checkChat(chatId);
                }
            });
            return null;
        });


        //History
        messagesRecyclerView.addOnScrollListener(new RecyclerView.OnScrollListener() {
            @Override
            public void onScrolled(@NonNull RecyclerView rv, int dx, int dy) {
                super.onScrolled(rv, dx, dy);
                checkLoadMore();
            }

            @Override
            public void onScrollStateChanged(@NonNull RecyclerView recyclerView, int newState) {
                super.onScrollStateChanged(recyclerView, newState);
                if (newState == SCROLL_STATE_IDLE && !recyclerView.canScrollVertically(-1)) {
                    checkLoadMore();
                }
            }
        });


        //Click Events
        pauseButton.setOnClickListener(v -> pauseRecording());
        deleteButton.setOnClickListener(v -> stopRecording(true));
        sendRecordButton.setOnClickListener(v -> stopRecording(false));

        closeAnswerDiv.setOnClickListener(v-> {answerType=0;answerMsg=null;answerDiv.setVisibility(View.GONE);});
        answerDiv.setOnClickListener(v->{
            if(answerType!=0&&answerMsg!=null) {
                goToMessage(answerMsg.msgId);
            }
        });
    }

    private void checkLoadMore() {
        if (isLoadingHistory || !hasMoreHistory) return;

        LinearLayoutManager lm = (LinearLayoutManager) messagesRecyclerView.getLayoutManager();
        if (lm == null) {return;}

        int firstVisible = lm.findFirstVisibleItemPosition();
        if (firstVisible <= VISIBLE_THRESHOLD) {
            loadMoreMessages();
        }
    }

    private void loadMoreMessages() {
        Log.e(TAG,"Load more history");
        if(isLoadingHistory||!hasMoreHistory)return;
        isLoadingHistory=true;

        TaskExecutorManager.getInstance().submitDecryptChatActivity("history:chatId:"+chat.chatUID,()-> {
            List<HCMessage> messagesL;

            if (messageAdapter.messageList.isEmpty()) {
                messagesL = hc.chatGroupChats.getChatLastMessages(chatId, commentId, 100, false);
            } else {
                HCMessage lastMsg = messageAdapter.messageList.get(0);
                messagesL = hc.chatGroupChats.getChatLastMessages(lastMsg.chatId, commentId, 100, false, lastMsg.msgId);
            }

            if (messagesL.size() < 100) {
                hasMoreHistory = false;
            }

            if (messagesL.isEmpty()) return null;

            hc.chatGroupChats.decryptMessagesAndAdd(this,chatId, messagesL, new HalChatGroupChats.HalChatAddMessage() {
                @Override
                public void addMessages(List<HCMessage> messages) {
                    /*int i;

                    try {
                        for (i = 0; i < messages.size(); i++) {
                            HCMessage msg = messages.get(i);
                            HNUser hnUser = hc.hnUsers.getUserByUserId(msg.fromId, true).get();
                            File file = hc.hd.getFileById(hnUser.icon).get();
                            msg.setFromIcon(BitmapFactory.decodeFile(file.getAbsolutePath()));
                            Log.e(TAG, "END LOADING MSG");
                            //messageAdapter.addMessageToTop(msg);
                            runOnUiThread(() -> messageAdapter.addMessageLoaded(msg));
                        }
                    } catch (Exception e) {
                        Log.e(TAG, "addMessages", e);
                    }*/

                    messageAdapter.addMessagesBatch(messages);
                    isLoadingHistory = false;
                }
            });

            return null;
        });
    }

    private String getInputText() {
        Spannable spannable = messageInput.getText();

        SpannableStringBuilder result = new SpannableStringBuilder(spannable);
        ImageSpan[] spans = spannable.getSpans(0, spannable.length(), ImageSpan.class);

        for (ImageSpan span : spans) {
            int start = spannable.getSpanStart(span);
            int end = spannable.getSpanEnd(span);

            String emojiTag = spannable.subSequence(start, end).toString();
            result.replace(start, end, emojiTag);
        }

        return result.toString();
    }

    private void sendMessageS(String recordMic) {
        long t=System.currentTimeMillis();
        long amsg=-1;

        if(answerType==1&&answerMsg!=null) {
            amsg=answerMsg.msgId;
        }

        HCMessage message=new HCMessage(-1,-1,chatId,hc.idUser,t/1000L,amsg,commentId,getInputText() ,new JSONArray(),"-1","-1",recordMic,new byte[0],false,false,false,0,true,false,null,0,0,false,1);

        JSONArray attachments=new JSONArray();

        for(HCFile hcf: selectedFiles) {
            if(!hcf.disabled&&!hcf.id.equals("-1")) {
                attachments.put(hcf.id);
            }
        }

        message.attachments=attachments;
        message.setDecryptedMessage(message.message);
        selectedFiles.clear();
        attachmentsSend.removeAllViews();

        messageInput.setText("");
        answerDiv.setVisibility(View.GONE);

        if(answerType==2&&answerMsg!=null) {
            message.msgId=answerMsg.msgId;
            hc.chatGroupChats.sendEditMessage(message).thenAccept(res->{
                if(res) {
                    hc.hnUsers.getUserByUserId(message.fromId).thenAccept(hnUser -> {
                        runOnUiThread(()-> {
                            message.setFromIcon(hnUser.icon);
                            messageAdapter.editMessage(message);
                        });
                    });
                } else {

                }
            });
            answerType=0;
            answerMsg=null;
            return;
        }

        answerType=0;
        answerMsg=null;

        hc.chatGroupChats.sendMessage(message).thenAccept(res->{
            if(res==null){
                return;
            }

            //Sound
            hc.playSound("resources/audio/send_message.wav");
            try {
                if (res.getInt("errorCode") == 0) {
                    Log.e(TAG, "Successful send message");
                    message.msgId =res.getLong("uid");
                    message.time = res.getLong("time");
                    hc.chatGroupChats.addMessageToChat(message,true);
                    runOnUiThread(() -> {
                        messageAdapter.addMessageLoaded(message);
                        //messageAdapter.addNewMessage(message);
                        messagesRecyclerView.scrollToPosition(messageAdapter.messageList.size()-1);
                    });
                    //addMessage.addMessage(msg);
                } else {
                    Log.e(TAG,"Error to sendMessage: "+ res);
                }
            } catch (Exception e) {
                Log.e(TAG,"sendMessage",e);
            }
        }).exceptionally(thr->{
            Log.e(TAG,"sendMessageS Expect: ",thr);

            //Скорее всего не доступен сервер, значит запоминаем отправленное сообщение
            hc.chatGroupChats.addToSendMessage(message);
            return null;
        });
        /*hc.hd.getFileById(hc.hnUsers.getIconFromCursor(hc.hnUsers.getUserCursorByUserId(message.fromId))).thenAccept(file -> {
            message.setFromIcon(BitmapFactory.decodeFile(file.getAbsolutePath()));
        });*/

        //messageAdapter.addNewMessage(message);
    }

    private void sendMessage(View v) {
        emojiMenu.animate()
                .translationY(emojiMenu.getHeight())
                .alpha(0f)
                .setDuration(300)
                .withEndAction(() -> emojiMenu.setVisibility(View.GONE));
        isEmojiMenuVisible=false;
        if (!messageInput.getText().toString().trim().isEmpty()||!selectedFiles.isEmpty()) {
            sendMessageS("-1");
        } else {
            if(!isRecording) {
                ActivityCompat.requestPermissions(this,permissions,REQUEST_RECORD_AUDIO_PERMISSION);
            }
        }
    }

    private void showPopupMenu(View anchorView) {
        View popupView = LayoutInflater.from(this).inflate(R.layout.menu_send_message, null);

        PopupWindow popupWindow = new PopupWindow(popupView,
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT,
                true);

        ImageButton btnAttachFile = popupView.findViewById(R.id.btn_attach_file);
        ImageButton btnSelectEmoji = popupView.findViewById(R.id.btn_select_emoji);

        btnAttachFile.setOnClickListener(v -> {
            selectFile();
            popupWindow.dismiss();
        });

        btnSelectEmoji.setOnClickListener(v -> {
            toggleEmojiMenu();
            popupWindow.dismiss();
        });

        popupWindow.setElevation(10);
        popupWindow.showAsDropDown(anchorView, 0, -anchorView.getHeight() - 320);
    }

    protected void openComments(long msgId) {
        Intent intent=new Intent(ChatActivity.this,ChatActivity.class);
        intent.putExtra("uid",chatId);
        intent.putExtra("commentId",msgId);
        startActivity(intent);
        overridePendingTransition(R.anim.slide_in_down,R.anim.stay);
    }

    private void selectFile() {
        selectMultipleFilesLauncher.launch("*/*");
    }

    protected void addFileToSend(HCFile file,int uid) {
        if(isSendedAudio) {
            isSendedAudio=false;
            sendMessageS(file.id);
            return;
        }

        if(file.disabled) {
            removeUploadFile(file,uid);
            return;
        }

        HCFile f=selectedFiles.get(uid);
        if(f==null || f.disabled) {
            return;
        }

        Log.e(TAG,"Add file to send");
        selectedFiles.set(uid,file);

        countUploadFiles--;

        if(countUploadFiles==0) {
            sendButton.setImageResource(R.drawable.ic_send);
        }

        View fileView=LayoutInflater.from(this).inflate(R.layout.file_item,attachmentsSend,false);
        TextView nameFile=fileView.findViewById(R.id.filename);
        nameFile.setText(file.name);
        ImageView iconFile=fileView.findViewById(R.id.fileicon);
        iconFile.setImageDrawable(AppCompatResources.getDrawable(this,file.icon));
        attachmentsSend.removeViewAt(uid);
        attachmentsSend.addView(fileView,uid);

        fileView.setOnClickListener(v->removeUploadFile(file,uid));
    }

    private void setMessageInput(String text) {
        if(text.isEmpty()) {
            messageInput.setText("");
            sendButton.setImageResource(R.drawable.ic_microphone);
            return;
        }
        messageInput.setText(text);
        sendButton.setImageResource(R.drawable.ic_send);
    }

    protected int addTempFile(HCFile file) {
        selectedFiles.add(file);

        sendButton.setImageResource(R.drawable.ic_microphone);

        countUploadFiles++;
        View fileView=LayoutInflater.from(this).inflate(R.layout.upload_file_item,attachmentsSend,false);
        TextView nameFile=fileView.findViewById(R.id.filename);
        nameFile.setText(file.name);
        attachmentsSend.addView(fileView);

        int uid=selectedFiles.size()-1;

        fileView.setOnClickListener(v->removeUploadFile(file,uid));
        return uid;
    }

    private void removeUploadFile(HCFile file, int uid) {
        attachmentsSend.getChildAt(uid).setVisibility(View.GONE);
        selectedFiles.get(uid).disabled=true;

        if(selectedFiles.get(uid).id.equals("-1")) {
            countUploadFiles--;
        }

        if(countUploadFiles==0) {
            sendButton.setImageResource(R.drawable.ic_send);
        }
    }

    protected void setProgressFile(int uid,int percent) {
        if(uid==-1)return;
        View fileView=attachmentsSend.getChildAt(uid);
        ProgressBar progressBar=fileView.findViewById(R.id.progressBar);
        progressBar.setProgress(percent);
    }

    protected void showMessageContextMenu(HCMessage message,View anchorView) {
        LayoutInflater inflater = (LayoutInflater) getSystemService(LAYOUT_INFLATER_SERVICE);
        View menuView = inflater.inflate(R.layout.context_menu_message, null);

        PopupWindow popupWindow = new PopupWindow(menuView,
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT,
                true);

        TextView editMessage = menuView.findViewById(R.id.edit_message);
        TextView deleteMessage = menuView.findViewById(R.id.delete_message);
        TextView answerMessage=menuView.findViewById(R.id.answer_message);

        editMessage.setOnClickListener(v -> {
            popupWindow.dismiss();
            editMessage(message);
        });

        deleteMessage.setOnClickListener(v -> {
            popupWindow.dismiss();
            deleteMessage(message);
        });

        answerMessage.setOnClickListener(v->{
            popupWindow.dismiss();
            answerMessage(message);
        });

        popupWindow.showAsDropDown(anchorView, 50, -anchorView.getHeight() / 2);
    }

    private void updateMessage(HCMessage message) {
        hc.hnUsers.getUserByUserId(message.fromId).thenAccept(hnUser -> {
            runOnUiThread(()-> {
                message.setFromIcon(hnUser.icon);
                messageAdapter.editMessage(message);
            });
        });
    }

    private void editMessage(HCMessage message) {
        hc.hnUsers.getUserByUserId(message.fromId).thenAccept(user->{
            answerType=2;
            answerMsg=message;

            setMessageInput(message.decryptedMessage);

            answerDivNickname.setText(user.nickname);
            answerDivText.setText(message.decryptedMessage);
            answerDiv.setVisibility(View.VISIBLE);
        });
    }

    private void deleteMessage(HCMessage message) {
        hc.chatGroupChats.sendDeleteMessage(message).thenAccept(f->{
            if(f) {
                messageAdapter.deleteMessage(message.msgId);
            }
        });
    }

    private void deleteMessageById(long id) {
        messageAdapter.deleteMessage(id);
    }


    private void answerMessage(HCMessage message) {
        hc.hnUsers.getUserByUserId(message.fromId).thenAccept(user->{
            answerType=1;
            answerMsg=message;

            answerDivNickname.setText(user.nickname);
            answerDivText.setText(message.decryptedMessage);
            answerDiv.setVisibility(View.VISIBLE);
        });

    }

    @Override
    public void finish() {
        super.finish();
        overridePendingTransition(android.R.anim.slide_in_left, android.R.anim.slide_out_right);
    }

    private void processSelectedFile(Uri fileUri) {
        try {
            //Загрузка файлов
            String fileName = HalChatFunctionsLib.getFileNameFromUri(getContentResolver(), fileUri);
            if (fileName == null) {
                fileName = new File(fileUri.getPath()).getName(); // Резервное имя, если имя файла не удалось получить
            }
            File tempFile=new File(hc.hd.directory,fileName);
            int uidf=addTempFile(new HCFile("-1",R.drawable.unknown_file,tempFile.getName()));

            try (InputStream inputStream = getContentResolver().openInputStream(fileUri);
                 OutputStream outputStream = new FileOutputStream(tempFile)) {
                byte[] buffer=new byte[8192];
                int bytesRead;

                while((bytesRead=inputStream.read(buffer))!=-1) {
                    outputStream.write(buffer,0,bytesRead);
                }
                outputStream.flush();
            }

            hc.chatGroupChats.uploadChatFile(tempFile, HalChatFunctionsLib.getContentType(tempFile), chatId,uidf);
        } catch (IOException e) {
            Log.e(TAG,"processSelectedFile",e);
            Toast.makeText(this,"Ошибка загрузки файла",Toast.LENGTH_SHORT).show();
        }
    }

    private void showEmojiMenu() {
        View emojiView = emojiMenu;

        EmojiAdapter emojiAdapter = new EmojiAdapter(this, hc.EPSystem.getListEmoji(), this::addEmojiToInput,hc);
        emojiGrid.setAdapter(emojiAdapter);

        //Pixels

        PixelAdapter pixelAdapter=new PixelAdapter(this, hc.EPSystem.getListPixel(), this::sendPixel, hc);
        pixelGrid.setAdapter(pixelAdapter);
    }

    // Показ / Скрытие меню эмодзи
    private void toggleEmojiMenu() {
        if (isEmojiMenuVisible) {
            emojiMenu.animate()
                    .translationY(emojiMenu.getHeight())
                    .alpha(0f)
                    .setDuration(300)
                    .withEndAction(() -> emojiMenu.setVisibility(View.GONE));
        } else {
            emojiGrid.setVisibility(View.VISIBLE);
            pixelGrid.setVisibility(View.GONE);

            emojiMenu.setVisibility(View.VISIBLE);
            emojiMenu.setAlpha(0f);
            emojiMenu.animate()
                    .translationY(0)
                    .alpha(1f)
                    .setDuration(300);
        }
        isEmojiMenuVisible = !isEmojiMenuVisible;
    }

    // Настройка эмодзи в меню
    private void setupEmojiMenu() {
        /*GridView emojiGrid = findViewById(R.id.emoji_grid);

        List<Integer> emojiList = List.of(
                R.drawable.smile, R.drawable.grinning,
                R.drawable.rolling_on_the_floor_laughing, R.drawable.cringe);

        EmojiAdapter emojiAdapter = new EmojiAdapter(this, emojiList, this::addEmojiToInput);
        emojiGrid.setAdapter(emojiAdapter);*/
        showEmojiMenu();
    }

    private void sendPixel(Pixel pixel) {
        Log.e(TAG,"SEND PIXEL");
        emojiMenu.animate()
                .translationY(emojiMenu.getHeight())
                .alpha(0f)
                .setDuration(300)
                .withEndAction(() -> emojiMenu.setVisibility(View.GONE));

        long t=System.currentTimeMillis();

        HCMessage message=new HCMessage(-1,-1,chatId,hc.idUser,t/1000L,-1,commentId,"" ,new JSONArray(),"-1","-1","",new byte[0],false,false,false,0,true,false,null,0,pixel.pixelId,false,1);
        message.setDecryptedMessage("");

        hc.playSound("resources/audio/send_message.wav");

        hc.chatGroupChats.sendPixel(pixel,chatId,commentId).thenAccept(res->{
            if(res==null)return;
            try {
                if (res.getInt("errorCode") == 0) {
                    Log.e(TAG, "Successful send message");
                    JSONObject msg=res.getJSONObject("message");
                    message.msgId = msg.getLong("uid");
                    message.time = msg.getLong("time");
                    hc.chatGroupChats.addMessageToChat(message,true);
                    runOnUiThread(() -> {
                        messageAdapter.addMessageLoaded(message);
                        //messageAdapter.addNewMessage(message);
                        messagesRecyclerView.scrollToPosition(messageAdapter.messageList.size()-1);
                    });
                    //addMessage.addMessage(msg);
                } else {
                    Log.e(TAG,"Error to sendMessage: "+ res);
                }
            } catch (Exception e) {
                Log.e(TAG,"sendMessage",e);
            }
        });
    }

    private void addEmojiToInput(Emoji emoji) {
        int size = (int) ((int) messageInput.getTextSize() * 1.5);  // Размер эмодзи пропорционально тексту

        Drawable placeholderDrawable = new ColorDrawable(Color.TRANSPARENT);
        placeholderDrawable.setBounds(0, 0, size, size);

        final ImageSpan placeholderSpan = new ImageSpan(placeholderDrawable, ImageSpan.ALIGN_BOTTOM);

        SpannableString spannableString = new SpannableString("[emoji-" + emoji.emojiId + "]");
        spannableString.setSpan(placeholderSpan, 0, spannableString.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

        int cursorPosition = Math.max(messageInput.getSelectionStart(), 0);
        messageInput.getText().insert(cursorPosition, spannableString);

        hc.hd.getFileById(emoji.image).thenAccept(fileIcon->{
            new Handler(Looper.getMainLooper()).post(() -> {
                Glide.with(this)
                        .asDrawable()
                        .load(fileIcon)
                        .override(size, size)
                        .into(new CustomTarget<Drawable>() {
                            @Override
                            public void onResourceReady(@NonNull Drawable resource, @Nullable Transition<? super Drawable> transition) {
                                resource.setBounds(0, 0, size, size);
                                ImageSpan readyImageSpan = new ImageSpan(resource, ImageSpan.ALIGN_BOTTOM);

                                Editable editable = messageInput.getText();
                                int start = editable.getSpanStart(placeholderSpan);
                                int end = editable.getSpanEnd(placeholderSpan);

                                if (start != -1 && end != -1) {
                                    editable.removeSpan(placeholderSpan);
                                    editable.setSpan(readyImageSpan, start, end, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
                                }
                            }

                            @Override
                            public void onLoadCleared(@Nullable Drawable placeholder) {

                            }

                            @Override
                            public void onLoadFailed(@Nullable Drawable errorDrawable) {

                            }
                        });
            });
        });
    }


    //CHAT INTERFACE
    public void onNewAction(HCAction action) {
        try {
            long cid=action.fromChat;
            int type=action.type;
            if(cid!=chatId)return;
            if(type==0) {
                messageAdapter.deleteMessage(action.fromMsg);
            } else if(type==1) {
                HCMessage message=action.newmsg;
                if(message==null)return;

                hc.hnUsers.getUserByUserId(message.fromId).thenAccept(hnUser -> {
                    message.setFromIcon(hnUser.icon);
                    messageAdapter.editMessage(message);
                });



            }
        } catch (Exception e) {
            Log.e(TAG,"onNewAction",e);
        }
    }

    public void onNewMessage(HCMessage message) {
        LinearLayoutManager layoutManager = (LinearLayoutManager) messagesRecyclerView.getLayoutManager();
        boolean isAtBottom = layoutManager == null||layoutManager.findLastVisibleItemPosition() > messageAdapter.getItemCount()-12;

        hc.hnUsers.getUserByUserId(message.fromId).thenAccept(hnUser -> {
                message.setFromIcon(hnUser.icon);
                messageAdapter.addMessageLoaded(message);
                //messageAdapter.addNewMessage(message);
                if (isAtBottom) {
                    messagesRecyclerView.scrollToPosition(messageAdapter.getItemCount()-1);
                }

                //Sound
                if(message.fromId!=hc.idUser) {
                    hc.playSound("resources/audio/receive_message.wav");
                }
            }
        );
    }

    public void onLoadMessage(HCMessage message) {
        try {
            hc.hnUsers.getUserByUserId(message.fromId,true).thenAccept(hnUser->{
                message.setFromIcon(hnUser.icon);
                runOnUiThread(()->messageAdapter.addMessageLoaded(message));
            });
        } catch (Exception e) {
            Log.e(TAG,"onLoadMessage",e);
        }

    }

    //Record Audio
    private void startRecording() {
        try {
            File audioFile = new File(hc.hd.directory, "recording_" + new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date()) + ".wav");
            audioFilePath = audioFile.getAbsolutePath();

            mediaRecorder = new MediaRecorder();
            mediaRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
            mediaRecorder.setOutputFormat(MediaRecorder.OutputFormat.MPEG_4);
            mediaRecorder.setAudioEncodingBitRate(128000);
            mediaRecorder.setAudioSamplingRate(44100);
            mediaRecorder.setOutputFile(audioFilePath);
            mediaRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.AAC);

            mediaRecorder.prepare();
            mediaRecorder.start();
            isRecording = true;

            AudioManager audioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
            audioManager.setParameters("noise_suppression=off");
            audioManager.setParameters("voice_volume_boost=on");

            messageInputDiv.setVisibility(View.GONE);
            recordingLayout.setVisibility(View.VISIBLE);

            startTime = SystemClock.elapsedRealtime();
            timerRunnable = new Runnable() {
                @Override
                public void run() {
                    long elapsedMillis = SystemClock.elapsedRealtime() - startTime;
                    int minutes = (int) (elapsedMillis / 60000);
                    int seconds = (int) (elapsedMillis / 1000) % 60;
                    timerText.setText(String.format("%02d:%02d", minutes, seconds));
                    timerHandler.postDelayed(this, 1000);
                }
            };
            timerHandler.post(timerRunnable);

        } catch (IOException e) {
            Toast.makeText(this, "Ошибка при записи", Toast.LENGTH_SHORT).show();
        }
    }

    private void pauseRecording() {
        if (mediaRecorder != null) {
            try {
                mediaRecorder.pause();
            } catch (Exception e) {
                Toast.makeText(this, "Ошибка при паузе", Toast.LENGTH_SHORT).show();
            }
        }
    }

    private void stopRecording(boolean deleteFile) {
        if (mediaRecorder != null) {
            try {
                mediaRecorder.stop();
                mediaRecorder.release();
                mediaRecorder = null;
                isRecording = false;

                messageInputDiv.setVisibility(View.VISIBLE);
                recordingLayout.setVisibility(View.GONE);
                timerHandler.removeCallbacks(timerRunnable);

                if (!deleteFile) {
                    uploadAudioFile(audioFilePath);
                } else {
                    File file = new File(audioFilePath);
                    if (file.exists()) file.delete();
                }

            } catch (Exception e) {
                Toast.makeText(this, "Ошибка при завершении записи", Toast.LENGTH_SHORT).show();
            }
        }
    }

    private void uploadAudioFile(String filePath) {
        try {
            File audioFile = new File(filePath);
            //int uidf=addTempFile(new HCFile("-1",R.drawable.unknown_file,audioFile.getName()));
            int uidf=-1;
            isSendedAudio=true;
            hc.chatGroupChats.uploadChatFile(audioFile,HalChatFunctionsLib.getContentType(audioFile),chatId,uidf);
            Toast.makeText(this, "Файл успешно отправлен", Toast.LENGTH_SHORT).show();

        } catch (Exception e) {
            Toast.makeText(this, "Ошибка при отправке", Toast.LENGTH_SHORT).show();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if(requestCode==REQUEST_RECORD_AUDIO_PERMISSION) {
            if (grantResults[0]== PackageManager.PERMISSION_GRANTED) {
                permissionToRecordAccepted = true;
                startRecording();
            } else {
                Toast.makeText(this,"Вы не предоставили разрешение на микрофон",Toast.LENGTH_SHORT).show();
                stopRecording(true);
            }

        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        hc.addEventListener("onNewAction",onNewActionEvent);
    }

    private void onNewAction(Object... data) {
        HCAction action=(HCAction)data[1];
        if(action.fromChat==chatId) {
            runOnUiThread(() -> ChatActivity.this.onNewAction(action));
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        TaskExecutorManager.getInstance().cancelTasks("chat"+chatId);
    }

    //Share Data

    private void checkShareData() {
        Intent intent=getIntent();
        if(intent.hasExtra("shareDataText")) {
            //messageInput.setText(intent.getStringExtra("shareDataText"));
            setMessageInput(Objects.requireNonNull(intent.getStringExtra("shareDataText")));
        }
        if(intent.hasExtra("shareDataFiles")) {
            String[] filesStr=intent.getStringArrayExtra("shareDataFiles");
            if(filesStr!=null) {
                for (String s : filesStr) {
                    File tempFile = new File(s);
                    int uidf = addTempFile(new HCFile("-1", R.drawable.unknown_file, tempFile.getName()));
                    hc.chatGroupChats.uploadChatFile(tempFile, HalChatFunctionsLib.getContentType(tempFile), chatId, uidf);
                }
                //sendButton.setImageResource(R.drawable.ic_send);
            }
        }
    }

    protected void goToMessage(long msgId) {
        int position = messageAdapter.getPositionById(msgId);
        if (position != -1) {
            messagesRecyclerView.smoothScrollToPosition(position);
        }
    }
}