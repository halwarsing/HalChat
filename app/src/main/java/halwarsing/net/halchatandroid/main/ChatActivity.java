package halwarsing.net.halchatandroid.main;

import static androidx.recyclerview.widget.RecyclerView.SCROLL_STATE_IDLE;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.Rect;
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
import android.view.inputmethod.InputMethodManager;
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
import java.util.Collections;
import java.util.Date;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Future;
import java.util.concurrent.atomic.AtomicLong;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.type.Emoji;
import halwarsing.net.halchatandroid.type.HCAction;
import halwarsing.net.halchatandroid.type.HCChat;
import halwarsing.net.halchatandroid.type.HCFile;
import halwarsing.net.halchatandroid.type.HCMessage;
import halwarsing.net.halchatandroid.type.HalChatI;
import halwarsing.net.halchatandroid.type.MessageReaction;
import halwarsing.net.halchatandroid.type.Pixel;

//Страница чата
public class ChatActivity extends AppCompatActivity {
    private HalChat hc;
    private RecyclerView messagesRecyclerView;
    private ImageButton scrollToBottomButton;
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
    private boolean hasMoreOldHistory=true;
    private boolean hasMoreNewHistory=false;
    private boolean isUserHistoryScroll=false;
    private static final int BOTTOM_BUTTON_THRESHOLD=10;
    private static final int VISIBLE_THRESHOLD=50; //За сколько сообщений до конца загружать историю

    //Emoji / Pixel
    private Button emojisSelectBtn, pixelsSelectBtn;
    private GridView emojiGrid, pixelGrid;
    private final Set<Long> pendingReactionMessages=ConcurrentHashMap.newKeySet();
    private final Set<Long> pendingPinnedMessages=ConcurrentHashMap.newKeySet();

    //Polls
    private LinearLayout pollMenu,pollMenuVariants;
    private Button pollAddBtn,pollRemoveBtn;
    private boolean isPollMenuVisible=false;
    private int pollVariants=2;

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

    //Pinned messages
    private ImageButton pinnedMsgBtn,btnPinnedUp,btnPinnedDown;
    private TextView pinnedMsgNickname, pinnedMsgText;
    private LinearLayout pinnedMsgDiv, pinnedMsg;
    private List<HCMessage> pinnedMsgs=new ArrayList<>();
    private int currentPinnedMsg=0;

    private final AtomicLong historyJumpRequestId = new AtomicLong();
    private final AtomicLong pinnedLoadRequestId = new AtomicLong();
    private Future<?> historyJumpFuture;
    private Future<?> historyLoadFuture;
    private Future<?> initialHistoryFuture;
    private Future<?> pinnedLoadFuture;
    private int countUploadFiles=0;

    //Message search
    private static final long MESSAGE_SEARCH_JUMP_DELAY_MS=150;
    private final Handler messageSearchHandler=new Handler(Looper.getMainLooper());
    private final List<Long> messageSearchResults=new ArrayList<>();
    private final Map<Long,String> messageSearchIndex=new LinkedHashMap<>();
    private LinearLayout messageSearchPanel;
    private EditText messageSearchInput;
    private TextView messageSearchCounter;
    private ImageButton messageSearchPrevious,messageSearchNext,messageSearchClose;
    private Runnable pendingMessageSearch;
    private Future<?> messageSearchIndexFuture;
    private String normalizedMessageSearchQuery="";
    private boolean isMessageSearchIndexBuilding=false;
    private boolean isMessageSearchIndexReady=false;
    private int currentMessageSearchResult=-1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        TextView nameChatText;
        ImageView iconChatView;
        ImageButton buttonBack,buttonMenu,buttonSearch;
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
        buttonSearch=findViewById(R.id.toolbar_search);
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

        //Polls
        pollMenu=findViewById(R.id.poll_menu);
        pollMenuVariants=findViewById(R.id.poll_menu_variants);
        pollAddBtn=findViewById(R.id.btn_poll_menu_add);
        pollRemoveBtn=findViewById(R.id.btn_poll_menu_remove);

        //Pinned messages
        pinnedMsgBtn=findViewById(R.id.pinnedMsgBtn);
        btnPinnedUp=findViewById(R.id.btnPinnedUp);
        btnPinnedDown=findViewById(R.id.btnPinnedDown);
        pinnedMsgNickname=findViewById(R.id.pinnedMsgNickname);
        pinnedMsgText=findViewById(R.id.pinnedMsgText);
        pinnedMsgDiv=findViewById(R.id.pinnedMsgDiv);
        pinnedMsg=findViewById(R.id.pinnedMsg);

        //Message search
        messageSearchPanel=findViewById(R.id.messageSearchPanel);
        messageSearchInput=findViewById(R.id.messageSearchInput);
        messageSearchCounter=findViewById(R.id.messageSearchCounter);
        messageSearchPrevious=findViewById(R.id.messageSearchPrevious);
        messageSearchNext=findViewById(R.id.messageSearchNext);
        messageSearchClose=findViewById(R.id.messageSearchClose);
        setupMessageSearch(buttonSearch);

        //Init
        messagesRecyclerView=findViewById(R.id.messages_recycler_view);
        scrollToBottomButton=findViewById(R.id.scrollToBottomButton);
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
                runOnUiThread(()->ChatActivity.this.deleteMessageById(id));
            }

            @Override
            public void onEditMessage(HCMessage message) {
                ChatActivity.this.updateMessage(message);
            }
        });

        //Pinned messages
        pinnedMsgDiv.setVisibility(View.GONE);
        loadPinnedMessages();

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
            if(isMessageSearchOpen()) {
                closeMessageSearch();
            } else {
                finish();
            }
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



        pinnedMsgBtn.setOnClickListener(this::goToPinnedMsg);
        pinnedMsg.setOnClickListener(this::goToPinnedMsg);

        btnPinnedUp.setOnClickListener(this::upPinMsg);

        btnPinnedDown.setOnClickListener(this::downPinMsg);
        scrollToBottomButton.setOnClickListener(v->goToLatestMessages());

        //Chat Sync
        long initialHistoryRequestId=historyJumpRequestId.get();
        initialHistoryFuture=TaskExecutorManager.getInstance().submitDecryptChatActivity("main:chatId:"+chat.chatUID,()->{
            messageList=hc.chatGroupChats.getChatLastMessages(chatId,commentId,100,false);
            List<HCMessage> decryptedMessages=hc.chatGroupChats.decryptMessages(chatId,messageList);

            runOnUiThread(()->{
                if(initialHistoryRequestId!=historyJumpRequestId.get() || isFinishing() || isDestroyed()) {
                    return;
                }
                messageAdapter.clearMessages(decryptedMessages);
                messagesRecyclerView.post(()->{
                    messagesRecyclerView.scrollToPosition(messageAdapter.messageList.size()-1);
                    messagesRecyclerView.post(this::updateScrollToBottomButton);
                });
            });

            hc.chatGroupChats.checkChat(chatId);
            return null;
        });

        hc.chatGroupChats.updateAllPollsChat(chatId).thenAccept(updated->
                runOnUiThread(messageAdapter::updatePollResults)
        );


        //History
        messagesRecyclerView.addOnScrollListener(new RecyclerView.OnScrollListener() {
            @Override
            public void onScrolled(@NonNull RecyclerView rv, int dx, int dy) {
                super.onScrolled(rv, dx, dy);
                updateScrollToBottomButton();
                if(isUserHistoryScroll) {
                    checkLoadMore(dy);
                }
            }

            @Override
            public void onScrollStateChanged(@NonNull RecyclerView recyclerView, int newState) {
                super.onScrollStateChanged(recyclerView, newState);
                if(newState==RecyclerView.SCROLL_STATE_DRAGGING) {
                    isUserHistoryScroll=true;
                } else if(newState==SCROLL_STATE_IDLE && isUserHistoryScroll) {
                    checkLoadMore(0);
                    isUserHistoryScroll=false;
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

        pollRemoveBtn.setOnClickListener(this::removePollMenu);
        pollAddBtn.setOnClickListener(this::addPollMenu);
    }

    private void setupMessageSearch(ImageButton buttonSearch) {
        updateMessageSearchControls();

        buttonSearch.setOnClickListener(v->openMessageSearch());
        messageSearchClose.setOnClickListener(v->closeMessageSearch());
        messageSearchPrevious.setOnClickListener(v->showPreviousMessageSearchResult());
        messageSearchNext.setOnClickListener(v->showNextMessageSearchResult());

        messageSearchInput.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s,int start,int count,int after) {
            }

            @Override
            public void onTextChanged(CharSequence s,int start,int before,int count) {
                if(isMessageSearchOpen()) {
                    updateMessageSearchQuery(s.toString());
                }
            }

            @Override
            public void afterTextChanged(Editable s) {
            }
        });
    }

    private boolean isMessageSearchOpen() {
        return messageSearchPanel.getVisibility()==View.VISIBLE;
    }

    private void openMessageSearch() {
        if(isMessageSearchOpen()) {
            messageSearchInput.requestFocus();
            return;
        }

        messageSearchPanel.setVisibility(View.VISIBLE);
        refreshVisibleMessageSearchIndex();
        startMessageSearchIndexIfNeeded();
        rebuildMessageSearchResults(false,true);
        messageSearchInput.requestFocus();
        messageSearchInput.post(()->{
            InputMethodManager inputMethodManager=
                    (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
            if(inputMethodManager!=null) {
                inputMethodManager.showSoftInput(messageSearchInput,InputMethodManager.SHOW_IMPLICIT);
            }
        });
    }

    private void closeMessageSearch() {
        messageSearchHandler.removeCallbacksAndMessages(null);
        pendingMessageSearch=null;

        messageSearchPanel.setVisibility(View.GONE);
        messageSearchInput.setText("");
        normalizedMessageSearchQuery="";
        clearMessageSearchResults();

        InputMethodManager inputMethodManager=
                (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
        if(inputMethodManager!=null) {
            inputMethodManager.hideSoftInputFromWindow(messageSearchInput.getWindowToken(),0);
        }
        messageSearchInput.clearFocus();
    }

    private void updateMessageSearchQuery(String query) {
        messageSearchHandler.removeCallbacksAndMessages(null);
        pendingMessageSearch=null;
        normalizedMessageSearchQuery=MessageSearch.normalizeQuery(query);
        refreshVisibleMessageSearchIndex();
        rebuildMessageSearchResults(false,false);

        if(normalizedMessageSearchQuery.isEmpty() || messageSearchResults.isEmpty()) {
            pendingMessageSearch=null;
            return;
        }

        long selectedMessageId=messageSearchResults.get(currentMessageSearchResult);
        pendingMessageSearch=()->{
            pendingMessageSearch=null;
            if(isMessageSearchOpen()
                    && currentMessageSearchResult>=0
                    && messageSearchResults.contains(selectedMessageId)) {
                goToMessage(selectedMessageId);
            }
        };
        messageSearchHandler.postDelayed(
                pendingMessageSearch,
                MESSAGE_SEARCH_JUMP_DELAY_MS
        );
    }

    private void startMessageSearchIndexIfNeeded() {
        if(isMessageSearchIndexReady || isMessageSearchIndexBuilding) {
            return;
        }

        isMessageSearchIndexBuilding=true;
        updateMessageSearchControls();
        messageSearchIndexFuture=TaskExecutorManager.getInstance().submitDecrypt(
                "message_search_index:"+chatId+":"+commentId,
                ()->{
                    try {
                        hc.chatGroupChats.buildLocalMessageSearchIndex(
                                chatId,
                                commentId,
                                batch->runOnUiThread(
                                        ()->applyMessageSearchIndexBatch(batch)
                                )
                        );
                        runOnUiThread(this::finishBuiltMessageSearchIndex);
                    } catch(Exception error) {
                        Log.e(TAG,"Unable to build message search index",error);
                        runOnUiThread(this::finishFailedMessageSearchIndex);
                    }
                    return null;
                }
        );
    }

    private void applyMessageSearchIndexBatch(Map<Long,String> batch) {
        if(isFinishing() || isDestroyed()) {
            return;
        }

        boolean hadResults=!messageSearchResults.isEmpty();

        messageSearchIndex.putAll(batch);
        refreshVisibleMessageSearchIndex();

        if(isMessageSearchOpen() && !normalizedMessageSearchQuery.isEmpty()) {
            MessageSearch.appendMatchingIds(messageSearchResults, batch, normalizedMessageSearchQuery);
            if(!hadResults && !messageSearchResults.isEmpty()) {
                currentMessageSearchResult=0;
            }
            updateMessageSearchControls();
            if(!hadResults && !messageSearchResults.isEmpty()) {
                goToMessage(messageSearchResults.get(currentMessageSearchResult));
            }
        }
    }

    private void finishBuiltMessageSearchIndex() {
        if(isFinishing() || isDestroyed()) {
            return;
        }
        isMessageSearchIndexBuilding=false;
        isMessageSearchIndexReady=true;
        messageSearchIndexFuture=null;
        updateMessageSearchControls();
    }

    private void finishFailedMessageSearchIndex() {
        if(isFinishing() || isDestroyed()) {
            return;
        }
        isMessageSearchIndexBuilding=false;
        isMessageSearchIndexReady=true;
        messageSearchIndexFuture=null;
        updateMessageSearchControls();
    }

    private void refreshVisibleMessageSearchIndex() {
        if(messageAdapter==null) {
            return;
        }
        for(HCMessage message:messageAdapter.messageList) {
            if(message==null || message.type==-1 || message.isDelete || !message.isDecrypted) {
                continue;
            }
            messageSearchIndex.put(message.msgId, MessageSearch.normalizeText(message.decryptedMessage));
        }
    }

    private void cacheMessageForSearch(HCMessage message) {
        if(message==null || message.type==-1 || message.isDelete || !message.isDecrypted) {
            return;
        }
        messageSearchIndex.put(message.msgId, MessageSearch.normalizeText(message.decryptedMessage));
        if(isMessageSearchOpen() && !normalizedMessageSearchQuery.isEmpty()) {
            rebuildMessageSearchResults(false,true);
        }
    }

    private void removeMessageFromSearchIndex(long msgId) {
        messageSearchIndex.remove(msgId);
        if(isMessageSearchOpen() && !normalizedMessageSearchQuery.isEmpty()) {
            rebuildMessageSearchResults(false,true);
        }
    }

    private void rebuildMessageSearchResults(boolean jumpToResult,boolean keepSelectedResult) {
        long selectedMessageId=keepSelectedResult ? getSelectedMessageSearchResultId() : -1;
        messageSearchResults.clear();

        if(!normalizedMessageSearchQuery.isEmpty()) {
            for(Map.Entry<Long,String> entry:messageSearchIndex.entrySet()) {
                if(MessageSearch.normalizedTextMatches(
                        entry.getValue(),
                        normalizedMessageSearchQuery
                )) {
                    messageSearchResults.add(entry.getKey());
                }
            }
            messageSearchResults.sort(Collections.reverseOrder());
        }

        currentMessageSearchResult=messageSearchResults.isEmpty() ? -1 : 0;
        if(selectedMessageId!=-1) {
            restoreSelectedMessageSearchResult(selectedMessageId);
        }
        updateMessageSearchControls();

        if(jumpToResult && currentMessageSearchResult!=-1) {
            goToMessage(messageSearchResults.get(currentMessageSearchResult));
        }
    }

    private long getSelectedMessageSearchResultId() {
        if(currentMessageSearchResult<0 || currentMessageSearchResult>=messageSearchResults.size()) {
            return -1;
        }
        return messageSearchResults.get(currentMessageSearchResult);
    }

    private void restoreSelectedMessageSearchResult(long msgId) {
        if(msgId==-1) {
            return;
        }
        int position=messageSearchResults.indexOf(msgId);
        if(position>=0) {
            currentMessageSearchResult=position;
        }
    }

    private void showPreviousMessageSearchResult() {
        if(currentMessageSearchResult<0 || currentMessageSearchResult>=messageSearchResults.size()-1) {
            return;
        }
        cancelPendingMessageSearchJump();
        currentMessageSearchResult++;
        updateMessageSearchControls();
        goToMessage(messageSearchResults.get(currentMessageSearchResult));
    }

    private void showNextMessageSearchResult() {
        if(currentMessageSearchResult<=0) {
            return;
        }
        cancelPendingMessageSearchJump();
        currentMessageSearchResult--;
        updateMessageSearchControls();
        goToMessage(messageSearchResults.get(currentMessageSearchResult));
    }

    private void cancelPendingMessageSearchJump() {
        if(pendingMessageSearch!=null) {
            messageSearchHandler.removeCallbacks(pendingMessageSearch);
            pendingMessageSearch=null;
        }
    }

    private void clearMessageSearchResults() {
        messageSearchResults.clear();
        currentMessageSearchResult=-1;
        updateMessageSearchControls();
    }

    private void updateMessageSearchControls() {
        boolean hasPrevious=currentMessageSearchResult>=0
                && currentMessageSearchResult<messageSearchResults.size()-1;
        boolean hasNext=currentMessageSearchResult>0;
        messageSearchPrevious.setEnabled(hasPrevious);
        messageSearchPrevious.setAlpha(hasPrevious?1f:0.35f);
        messageSearchNext.setEnabled(hasNext);
        messageSearchNext.setAlpha(hasNext?1f:0.35f);

        if(normalizedMessageSearchQuery.isEmpty()) {
            messageSearchCounter.setText(R.string.message_search_empty);
        } else if(currentMessageSearchResult==-1 && isMessageSearchIndexBuilding) {
            messageSearchCounter.setText(R.string.message_search_loading);
        } else if(currentMessageSearchResult==-1) {
            messageSearchCounter.setText(R.string.message_search_empty);
        } else {
            messageSearchCounter.setText(getString(
                    R.string.message_search_results,
                    currentMessageSearchResult+1,
                    messageSearchResults.size()
            ));
        }
    }

    private void checkLoadMore(int direction) {
        if (isLoadingHistory) return;

        LinearLayoutManager lm = (LinearLayoutManager) messagesRecyclerView.getLayoutManager();
        if (lm == null) {return;}

        int totalItemCount = lm.getItemCount();
        int firstVisible = lm.findFirstVisibleItemPosition();
        int lastVisible = lm.findLastVisibleItemPosition();
        if (direction<=0 && hasMoreOldHistory && firstVisible <= VISIBLE_THRESHOLD) {
            loadMoreMessages();
        } else if (direction>=0 && hasMoreNewHistory && (totalItemCount - lastVisible) <= VISIBLE_THRESHOLD) {
            loadNewMessages();
        }
    }

    private void loadMoreMessages() {
        Log.e(TAG,"Load more history");
        if(isLoadingHistory||!hasMoreOldHistory)return;
        isUserHistoryScroll=false;
        isLoadingHistory=true;
        long requestId=historyJumpRequestId.get();
        HistoryAnchor anchor=captureHistoryAnchor();
        HCMessage oldestMessage=messageAdapter.getOldestMessage();

        historyLoadFuture=TaskExecutorManager.getInstance().submitDecryptChatActivity("history:chatId:"+chat.chatUID,()-> {
            try {
                List<HCMessage> messagesL=oldestMessage==null
                        ? hc.chatGroupChats.getChatLastMessages(chatId,commentId,100,false)
                        : hc.chatGroupChats.getChatLastMessages(chatId,commentId,100,false,oldestMessage.msgId);
                boolean hasMore=messagesL.size()>=100 || !hc.chatGroupChats.isChatHistoryEnd(chatId);
                List<HCMessage> decryptedMessages=hc.chatGroupChats.decryptMessages(chatId,messagesL);

                runOnUiThread(()->{
                    if(requestId!=historyJumpRequestId.get()) {
                        return;
                    }
                    hasMoreOldHistory=hasMore;
                    messageAdapter.addMessagesBatch(decryptedMessages);
                    restoreHistoryAnchor(anchor);
                    updateScrollToBottomButton();
                    finishHistoryLoad(requestId);
                });
            } catch (Exception error) {
                Log.e(TAG,"Unable to load old history",error);
                runOnUiThread(()->finishHistoryLoad(requestId));
            }
            return null;
        });
    }

    private void loadNewMessages() {
        Log.e(TAG,"Load new history");
        if(isLoadingHistory||!hasMoreNewHistory)return;
        isUserHistoryScroll=false;
        isLoadingHistory=true;
        long requestId=historyJumpRequestId.get();
        HistoryAnchor anchor=captureHistoryAnchor();
        HCMessage newestMessage=messageAdapter.getNewestMessage();

        historyLoadFuture=TaskExecutorManager.getInstance().submitDecryptChatActivity("historyNew:chatId:"+chat.chatUID,()-> {
            try {
                List<HCMessage> messagesL=newestMessage==null
                        ? hc.chatGroupChats.getChatNextMessages(chatId,commentId,100,false,0)
                        : hc.chatGroupChats.getChatNextMessages(chatId,commentId,100,false,newestMessage.msgId);
                boolean hasMore=messagesL.size()>=100;
                List<HCMessage> decryptedMessages=hc.chatGroupChats.decryptMessages(chatId,messagesL);

                runOnUiThread(()->{
                    if(requestId!=historyJumpRequestId.get()) {
                        return;
                    }
                    hasMoreNewHistory=hasMore;
                    messageAdapter.addMessagesBatch(decryptedMessages);
                    restoreHistoryAnchor(anchor);
                    updateScrollToBottomButton();
                    finishHistoryLoad(requestId);
                });
            } catch (Exception error) {
                Log.e(TAG,"Unable to load new history",error);
                runOnUiThread(()->finishHistoryLoad(requestId));
            }
            return null;
        });
    }

    private HistoryAnchor captureHistoryAnchor() {
        LinearLayoutManager layoutManager=(LinearLayoutManager)messagesRecyclerView.getLayoutManager();
        if(layoutManager==null) {
            return null;
        }
        int position=layoutManager.findFirstVisibleItemPosition();
        if(position<0 || position>=messageAdapter.messageList.size()) {
            return null;
        }
        View view=layoutManager.findViewByPosition(position);
        int offset=view==null?0:view.getTop()-messagesRecyclerView.getPaddingTop();
        return new HistoryAnchor(messageAdapter.messageList.get(position).msgId,offset);
    }

    private void restoreHistoryAnchor(HistoryAnchor anchor) {
        if(anchor==null) {
            return;
        }
        LinearLayoutManager layoutManager=(LinearLayoutManager)messagesRecyclerView.getLayoutManager();
        int position=messageAdapter.getPositionById(anchor.msgId);
        if(layoutManager!=null && position!=-1) {
            layoutManager.scrollToPositionWithOffset(position,anchor.offset);
        }
    }

    private void finishHistoryLoad(long requestId) {
        if(requestId==historyJumpRequestId.get()) {
            isLoadingHistory=false;
            historyLoadFuture=null;
        }
    }

    private void updateScrollToBottomButton() {
        LinearLayoutManager layoutManager=(LinearLayoutManager)messagesRecyclerView.getLayoutManager();
        if(layoutManager==null) {
            scrollToBottomButton.setVisibility(View.GONE);
            return;
        }

        int lastVisible=layoutManager.findLastVisibleItemPosition();
        int messagesBelow=lastVisible==RecyclerView.NO_POSITION
                ? 0
                : messageAdapter.getItemCount()-1-lastVisible;
        boolean shouldShow=hasMoreNewHistory || messagesBelow>BOTTOM_BUTTON_THRESHOLD;
        scrollToBottomButton.setVisibility(shouldShow?View.VISIBLE:View.GONE);
    }

    private void goToLatestMessages() {
        isUserHistoryScroll=false;
        scrollToBottomButton.setVisibility(View.GONE);

        if(!hasMoreNewHistory) {
            int lastPosition=messageAdapter.getItemCount()-1;
            if(lastPosition>=0) {
                messagesRecyclerView.scrollToPosition(lastPosition);
                messagesRecyclerView.post(this::updateScrollToBottomButton);
            }
            return;
        }

        long requestId=historyJumpRequestId.incrementAndGet();
        if(historyJumpFuture!=null)historyJumpFuture.cancel(true);
        if(historyLoadFuture!=null)historyLoadFuture.cancel(true);
        if(initialHistoryFuture!=null)initialHistoryFuture.cancel(true);
        isLoadingHistory=true;

        historyJumpFuture=TaskExecutorManager.getInstance().submitDecryptChatActivity(
                "history_latest:"+chatId+":"+commentId,
                ()->{
                    try {
                        List<HCMessage> latest=hc.chatGroupChats.getChatLastMessages(
                                chatId,commentId,100,false
                        );
                        List<HCMessage> decrypted=hc.chatGroupChats.decryptMessages(chatId,latest);
                        boolean hasMoreOld=latest.size()>=100
                                || !hc.chatGroupChats.isChatHistoryEnd(chatId);

                        runOnUiThread(()->{
                            if(requestId!=historyJumpRequestId.get()) {
                                return;
                            }
                            messageAdapter.clearMessages(decrypted);
                            hasMoreOldHistory=hasMoreOld;
                            hasMoreNewHistory=false;
                            messagesRecyclerView.post(()->{
                                if(requestId!=historyJumpRequestId.get()) {
                                    return;
                                }
                                messagesRecyclerView.scrollToPosition(messageAdapter.getItemCount()-1);
                                isLoadingHistory=false;
                                historyJumpFuture=null;
                                messagesRecyclerView.post(this::updateScrollToBottomButton);
                            });
                        });
                    } catch(Exception error) {
                        Log.e(TAG,"Unable to return to latest messages",error);
                        runOnUiThread(()->{
                            if(requestId==historyJumpRequestId.get()) {
                                isLoadingHistory=false;
                                historyJumpFuture=null;
                                updateScrollToBottomButton();
                            }
                        });
                    }
                    return null;
                }
        );
    }

    private static class HistoryAnchor {
        final long msgId;
        final int offset;

        HistoryAnchor(long msgId,int offset) {
            this.msgId=msgId;
            this.offset=offset;
        }
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

        //Polls
        List<String> variants=new ArrayList<>();
        JSONArray variantsJSON=new JSONArray();
        if(isPollMenuVisible) {
            for (int i = 0; i < pollVariants; i++) {
                EditText editText = (EditText) pollMenuVariants.getChildAt(i);
                String text = editText.getText().toString();
                if (text.isEmpty()) {
                    //Вариант пустой, прерываем
                    Toast.makeText(this, "Удалите незаполненные варианты", Toast.LENGTH_SHORT).show();
                    return;
                }
                variants.add(text);
                variantsJSON.put(text);
            }

            message.type = 4;
            message.setPollVariants(variants);

            togglePollMenu();
        }

        hc.chatGroupChats.sendMessage(message,variants).thenAccept(res->{
            if(res==null){
                return;
            }

            //Sound
            hc.playSound("resources/audio/send_message.wav");
            try {
                if (res.getInt("errorCode") == 0) {
                    Log.e(TAG, "Successful send message");
                    HCMessage nmsg=hc.chatGroupChats.jsonMsgToHCMSG(res.getJSONObject("message"));

                    nmsg=hc.chatGroupChats.deencryptMessage(nmsg,hc.chatGroupChats.getPasswordChat(nmsg.chatId));
                    if(nmsg.type==4)nmsg.setPollVariants(variants);
                    hc.chatGroupChats.addMessageToChat(nmsg,true);
                    HCMessage finalNmsg = nmsg;

                    if(nmsg.type==4&&nmsg.data!=null&&nmsg.data.has("variants")) {
                        nmsg.data.put("variants", variantsJSON);
                    }

                    runOnUiThread(() -> {
                        messageAdapter.addMessageLoaded(finalNmsg);
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
        ImageButton btnPoll=popupView.findViewById(R.id.btn_poll);

        btnAttachFile.setOnClickListener(v -> {
            selectFile();
            popupWindow.dismiss();
        });

        btnSelectEmoji.setOnClickListener(v -> {
            toggleEmojiMenu();
            popupWindow.dismiss();
        });

        btnPoll.setOnClickListener(v->{
            togglePollMenu();
            popupWindow.dismiss();
        });

        popupWindow.setElevation(10);
        popupWindow.showAsDropDown(anchorView, 0, -anchorView.getHeight() - 520);
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
        TextView copyMessage=menuView.findViewById(R.id.copy_message);
        TextView pinMessage=menuView.findViewById(R.id.pin_message);
        GridView reactionEmojiGrid=menuView.findViewById(R.id.reaction_emoji_grid);
        final String messageText=message.decryptedMessage==null?message.message:message.decryptedMessage;

        ReactionEmojiAdapter reactionEmojiAdapter=new ReactionEmojiAdapter(
                this,
                hc.EPSystem.getListEmoji(),
                emoji->{
                    popupWindow.dismiss();
                    setMessageReaction(message,emoji.emojiId);
                },
                hc
        );
        reactionEmojiGrid.setAdapter(reactionEmojiAdapter);

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

        if(messageText==null||messageText.isEmpty()) {
            copyMessage.setVisibility(View.GONE);
        } else {
            copyMessage.setOnClickListener(v->{
                popupWindow.dismiss();
                HalChatFunctionsLib.setClipboard(this,"HalChat",messageText);
                Toast.makeText(this,R.string.message_copied,Toast.LENGTH_SHORT).show();
            });
        }

        pinMessage.setText(message.isPinned?R.string.unpin_message:R.string.pin_message);
        pinMessage.setOnClickListener(v->{
            popupWindow.dismiss();
            setMessagePinned(message);
        });

        Rect visibleFrame=new Rect();
        View decorView=getWindow().getDecorView();
        decorView.getWindowVisibleDisplayFrame(visibleFrame);
        menuView.measure(
                View.MeasureSpec.makeMeasureSpec(visibleFrame.width(),View.MeasureSpec.AT_MOST),
                View.MeasureSpec.makeMeasureSpec(visibleFrame.height(),View.MeasureSpec.AT_MOST)
        );

        int popupWidth=menuView.getMeasuredWidth();
        int popupHeight=menuView.getMeasuredHeight();
        int popupMargin=(int) (8*getResources().getDisplayMetrics().density);
        int[] anchorLocation=new int[2];
        anchorView.getLocationOnScreen(anchorLocation);

        int popupX=anchorLocation[0]+(anchorView.getWidth()-popupWidth)/2;
        popupX=Math.max(
                visibleFrame.left+popupMargin,
                Math.min(popupX,visibleFrame.right-popupWidth-popupMargin)
        );

        int popupY=anchorLocation[1]+anchorView.getHeight();
        if(popupY+popupHeight>visibleFrame.bottom-popupMargin) {
            popupY=anchorLocation[1]-popupHeight;
        }
        popupY=Math.max(
                visibleFrame.top+popupMargin,
                Math.min(popupY,visibleFrame.bottom-popupHeight-popupMargin)
        );

        popupWindow.setWidth(popupWidth);
        popupWindow.setHeight(popupHeight);
        popupWindow.showAtLocation(decorView,Gravity.TOP|Gravity.START,popupX,popupY);
    }

    protected void setMessageReaction(HCMessage message,long emojiId) {
        if(message==null || message.msgId<0 || !pendingReactionMessages.add(message.msgId)) {
            return;
        }

        HCMessage currentMessage=messageAdapter.getMessageById(message.msgId);
        if(currentMessage==null)currentMessage=message;
        final HCMessage reactionMessage=currentMessage;
        final List<MessageReaction> oldReactions=reactionMessage.reactions;
        final long oldSelectedReaction=reactionMessage.selectedReaction;

        reactionMessage.applyReaction(emojiId);
        messageAdapter.updateMessageReactions(reactionMessage.msgId);

        hc.chatGroupChats.setReaction(reactionMessage.chatId,reactionMessage.msgId,emojiId)
                .whenComplete((success,error)->{
                    pendingReactionMessages.remove(reactionMessage.msgId);
                    if(error!=null) {
                        Log.e(TAG,"Unable to set reaction",error);
                    }
                    if(error!=null || !Boolean.TRUE.equals(success)) {
                        runOnUiThread(()->{
                            if(messageAdapter.getMessageById(reactionMessage.msgId)==reactionMessage) {
                                reactionMessage.setReactions(oldReactions,oldSelectedReaction);
                                messageAdapter.updateMessageReactions(reactionMessage.msgId);
                            }
                            Toast.makeText(
                                    this,
                                    "Не удалось изменить реакцию",
                                    Toast.LENGTH_SHORT
                            ).show();
                        });
                    }
                });
    }

    private void setMessagePinned(HCMessage message) {
        if(message==null||message.msgId<0||!pendingPinnedMessages.add(message.msgId))return;

        HCMessage currentMessage=messageAdapter.getMessageById(message.msgId);
        if(currentMessage==null)currentMessage=message;
        final HCMessage pinnedMessage=currentMessage;
        final boolean oldPinned=pinnedMessage.isPinned;
        final boolean newPinned=!oldPinned;

        updatePinnedMessageLocal(pinnedMessage,newPinned);
        hc.chatGroupChats.pinMessage(pinnedMessage.chatId,pinnedMessage.msgId)
                .whenComplete((isPinned,error)->{
                    pendingPinnedMessages.remove(pinnedMessage.msgId);
                    runOnUiThread(()->{
                        if(error!=null) {
                            Log.e(TAG,"Unable to pin message",error);
                            if(messageAdapter.getMessageById(pinnedMessage.msgId)==pinnedMessage) {
                                updatePinnedMessageLocal(pinnedMessage,oldPinned);
                            } else {
                                loadPinnedMessages();
                            }
                            Toast.makeText(this,R.string.pin_message_failed,Toast.LENGTH_SHORT).show();
                        } else if(isPinned!=newPinned) {
                            updatePinnedMessageLocal(pinnedMessage,isPinned);
                        }
                    });
                });
    }

    private void updatePinnedMessageLocal(HCMessage message,boolean isPinned) {
        pinnedLoadRequestId.incrementAndGet();
        if(pinnedLoadFuture!=null) {
            pinnedLoadFuture.cancel(true);
            pinnedLoadFuture=null;
        }

        message.isPinned=isPinned;
        for(int i=pinnedMsgs.size()-1;i>=0;i--) {
            if(pinnedMsgs.get(i).msgId==message.msgId)pinnedMsgs.remove(i);
        }
        if(isPinned) {
            pinnedMsgs.add(message);
            pinnedMsgs.sort((first,second)->Long.compare(first.msgId,second.msgId));
        }

        currentPinnedMsg=0;
        if(pinnedMsgs.isEmpty()) {
            pinnedMsgDiv.setVisibility(View.GONE);
        } else {
            pinnedMsgDiv.setVisibility(View.VISIBLE);
            displayPinnedMsg();
        }
    }

    private void updateMessage(HCMessage message) {
        loadPinnedMessages();
        hc.hnUsers.getUserByUserId(message.fromId).thenAccept(hnUser -> {
            runOnUiThread(()-> {
                message.setFromIcon(hnUser.icon);
                messageAdapter.editMessage(message);
                cacheMessageForSearch(message);
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
                runOnUiThread(()->{
                    messageAdapter.deleteMessage(message.msgId);
                    removeMessageFromSearchIndex(message.msgId);
                });
            }
        });
    }

    private void deleteMessageById(long id) {
        messageAdapter.deleteMessage(id);
        removeMessageFromSearchIndex(id);
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
                runOnUiThread(()->{
                    messageAdapter.deleteMessage(action.fromMsg);
                    removeMessageFromSearchIndex(action.fromMsg);
                });
            } else if(type==1||type==17) {
                HCMessage message=action.newmsg;
                if(message==null)return;

                hc.hnUsers.getUserByUserId(message.fromId).thenAccept(hnUser -> {
                    runOnUiThread(()->{
                        message.setFromIcon(hnUser.icon);
                        messageAdapter.editMessage(message);
                        cacheMessageForSearch(message);
                    });
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
                runOnUiThread(()->{
                    message.setFromIcon(hnUser.icon);
                    messageAdapter.addMessageLoaded(message);
                    cacheMessageForSearch(message);
                    if (isAtBottom) {
                        messagesRecyclerView.scrollToPosition(messageAdapter.getItemCount()-1);
                    }
                    updateScrollToBottomButton();

                    if(message.isPinned) {
                        loadPinnedMessages();
                    }

                    if(message.fromId!=hc.idUser) {
                        hc.playSound("resources/audio/receive_message.wav");
                    }
                });
            }
        );
    }

    public void onLoadMessage(HCMessage message) {
        try {
            hc.hnUsers.getUserByUserId(message.fromId,true).thenAccept(hnUser->{
                message.setFromIcon(hnUser.icon);
                runOnUiThread(()->{
                    messageAdapter.addMessageLoaded(message);
                    cacheMessageForSearch(message);
                    if(message.isPinned) {
                        loadPinnedMessages();
                    }
                });
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
    protected void onDestroy() {
        super.onDestroy();
        historyJumpRequestId.incrementAndGet();
        pinnedLoadRequestId.incrementAndGet();
        messageSearchHandler.removeCallbacksAndMessages(null);
        if(historyJumpFuture!=null)historyJumpFuture.cancel(true);
        if(historyLoadFuture!=null)historyLoadFuture.cancel(true);
        if(initialHistoryFuture!=null)initialHistoryFuture.cancel(true);
        if(pinnedLoadFuture!=null)pinnedLoadFuture.cancel(true);
        if(messageSearchIndexFuture!=null)messageSearchIndexFuture.cancel(true);
        TaskExecutorManager.getInstance().cancelTasks("chat"+chatId);
    }

    @Override
    public void onBackPressed() {
        if(isMessageSearchOpen()) {
            closeMessageSearch();
            return;
        }
        super.onBackPressed();
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
        long requestId = historyJumpRequestId.incrementAndGet();

        if (historyJumpFuture != null) {
            historyJumpFuture.cancel(true);
            historyJumpFuture = null;
        }
        if(historyLoadFuture!=null) {
            historyLoadFuture.cancel(true);
            historyLoadFuture=null;
        }
        if(initialHistoryFuture!=null) {
            initialHistoryFuture.cancel(true);
            initialHistoryFuture=null;
        }

        int position = messageAdapter.getPositionById(msgId);
        if (position != -1) {
            isLoadingHistory = false;
            jumpToMessageInstantly(msgId);
            return;
        }

        isUserHistoryScroll=false;
        isLoadingHistory = true;

        String taskTag = "history_jump:" + chatId + ":" + commentId;

        historyJumpFuture = TaskExecutorManager.getInstance()
                .submitDecryptChatActivity(taskTag, () -> {
                    try {
                        List<HCMessage> contextMessages = new ArrayList<>();

                        List<HCMessage> previous =
                                hc.chatGroupChats.getChatLastMessages(
                                        chatId, commentId, 50, false, msgId
                                );

                        if (previous != null) {
                            contextMessages.addAll(previous);
                        }

                        if (Thread.currentThread().isInterrupted()) {
                            return null;
                        }

                        HCMessage target=hc.chatGroupChats.getLocalMessage(chatId,commentId,msgId);
                        if(target==null) {
                            hc.chatGroupChats.loadMessage(chatId,msgId);
                            runOnUiThread(()->{
                                if(requestId==historyJumpRequestId.get()) {
                                    isLoadingHistory=false;
                                    historyJumpFuture=null;
                                }
                            });
                            return null;
                        }
                        contextMessages.add(target);

                        List<HCMessage> next =
                                hc.chatGroupChats.getChatNextMessages(
                                        chatId, commentId, 50, false, msgId
                                );

                        if (next != null) {
                            contextMessages.addAll(next);
                        }

                        if (Thread.currentThread().isInterrupted()) {
                            return null;
                        }

                        List<HCMessage> messages=hc.chatGroupChats.decryptMessages(chatId,contextMessages);
                        boolean hasMoreOld=(previous!=null && previous.size()>=50)
                                || !hc.chatGroupChats.isChatHistoryEnd(chatId);
                        boolean hasMoreNew=next!=null && next.size()>=50;
                        runOnUiThread(() -> {
                                    // Старый callback не должен перетирать новый переход.
                                    if (requestId != historyJumpRequestId.get()) {
                                        return;
                                    }

                                    messageAdapter.clearMessages(messages);
                                    hasMoreOldHistory=hasMoreOld;
                                    hasMoreNewHistory=hasMoreNew;

                                    messagesRecyclerView.post(() -> {
                                        if (requestId != historyJumpRequestId.get()) {
                                            return;
                                        }

                                        try {
                                            int pos =
                                                    messageAdapter.getPositionById(msgId);

                                            if (pos != -1) {
                                                jumpToMessageInstantly(msgId);
                                            }
                                        } finally {
                                            isLoadingHistory = false;
                                            historyJumpFuture = null;
                                        }
                                    });
                                });
                    } catch (Exception e) {
                        runOnUiThread(() -> {
                            if (requestId == historyJumpRequestId.get()) {
                                isLoadingHistory = false;
                                historyJumpFuture = null;
                            }
                        });

                        Log.e(TAG, "Unable to load message " + msgId, e);
                    }

                    return null;
                });
    }

    private void jumpToMessageInstantly(long msgId) {
        isUserHistoryScroll=false;
        messagesRecyclerView.stopScroll();
        positionMessageInViewport(msgId);
        messagesRecyclerView.post(()->{
            positionMessageInViewport(msgId);
            messagesRecyclerView.postOnAnimation(()->{
                positionMessageInViewport(msgId);
                flashFoundMessage(msgId);
            });
        });
    }

    private void positionMessageInViewport(long msgId) {
        int position=messageAdapter.getPositionById(msgId);
        if(position==-1) {
            return;
        }
        LinearLayoutManager lm = (LinearLayoutManager) messagesRecyclerView.getLayoutManager();
        if (lm != null) {
            int offset = messagesRecyclerView.getHeight() / 3;
            lm.scrollToPositionWithOffset(position, offset);
            messagesRecyclerView.post(this::updateScrollToBottomButton);
        }
    }

    private void flashFoundMessage(long msgId) {
        int position=messageAdapter.getPositionById(msgId);
        if(position==-1) {
            return;
        }
        RecyclerView.ViewHolder holder=messagesRecyclerView.findViewHolderForAdapterPosition(position);
        if(holder==null) {
            return;
        }
        holder.itemView.animate().cancel();
        holder.itemView.setAlpha(0.55f);
        holder.itemView.animate().alpha(1f).setDuration(300).start();
    }


    //Polls
    private void togglePollMenu() {
        if (isPollMenuVisible) {
            pollMenu.animate()
                    .translationY(pollMenu.getHeight())
                    .alpha(0f)
                    .setDuration(300)
                    .withEndAction(() -> pollMenu.setVisibility(View.GONE));
        } else {
            for(int i=2;i<pollVariants;i++) {
                pollMenuVariants.getChildAt(i).setVisibility(View.GONE);
            }
            for(int i=0;i<12;i++) {
                ((EditText)pollMenuVariants.getChildAt(i)).setText("");
            }
            pollVariants=2;

            pollMenu.setVisibility(View.VISIBLE);
            pollMenu.setAlpha(0f);
            pollMenu.animate()
                    .translationY(0)
                    .alpha(1f)
                    .setDuration(300);
        }
        isPollMenuVisible = !isPollMenuVisible;
    }

    private void removePollMenu(View v) {
        if(pollVariants<=2) {
            Toast.makeText(this,"Меньше 2-х вариантов не может быть",Toast.LENGTH_SHORT).show();
            return;
        }

        pollVariants--;
        pollMenuVariants.getChildAt(pollVariants).setVisibility(View.GONE);
    }

    private void addPollMenu(View v) {
        if(pollVariants>=12) {
            Toast.makeText(this,"Нельзя добавлять более 12 вариантов",Toast.LENGTH_SHORT).show();
            return;
        }
        pollMenuVariants.getChildAt(pollVariants).setVisibility(View.VISIBLE);
        pollVariants++;
    }

    //Pinned messages
    private void loadPinnedMessages() {
        long requestId=pinnedLoadRequestId.incrementAndGet();
        if(pinnedLoadFuture!=null) {
            pinnedLoadFuture.cancel(true);
        }

        pinnedLoadFuture=TaskExecutorManager.getInstance().submitDecryptChatActivity("pinned:chatId:"+chatId,()->{
            List<HCMessage> messages=hc.chatGroupChats.getPinnedMessages(chatId);
            List<HCMessage> decryptedMessages=hc.chatGroupChats.decryptMessages(chatId,messages);

            runOnUiThread(()->{
                if(requestId!=pinnedLoadRequestId.get() || isFinishing() || isDestroyed()) {
                    return;
                }
                pinnedMsgs=decryptedMessages;
                pinnedLoadFuture=null;
                if(pinnedMsgs.isEmpty()) {
                    currentPinnedMsg=0;
                    pinnedMsgDiv.setVisibility(View.GONE);
                    return;
                }
                currentPinnedMsg=Math.min(currentPinnedMsg,pinnedMsgs.size()-1);
                pinnedMsgDiv.setVisibility(View.VISIBLE);
                displayPinnedMsg();
            });
            return null;
        });
    }

    private void goToPinnedMsg(View v) {
        if(pinnedMsgs==null || currentPinnedMsg>=pinnedMsgs.size())return;
        HCMessage msg=pinnedMsgs.get(pinnedMsgs.size()-1-currentPinnedMsg);
        goToMessage(msg.msgId);
    }

    private void displayPinnedMsg() {
        if(pinnedMsgs==null || currentPinnedMsg>=pinnedMsgs.size())return;
        HCMessage msg=pinnedMsgs.get(pinnedMsgs.size()-1-currentPinnedMsg);
        long displayedMessageId=msg.msgId;
        pinnedMsgNickname.setText("");
        pinnedMsgText.setText(msg.decryptedMessage==null?msg.message:msg.decryptedMessage);
        hc.hnUsers.getUserByUserId(msg.fromId).thenAccept(hnUser -> {
            runOnUiThread(()->{
                if(pinnedMsgs==null || currentPinnedMsg>=pinnedMsgs.size()) {
                    return;
                }
                HCMessage current=pinnedMsgs.get(pinnedMsgs.size()-1-currentPinnedMsg);
                if(current.msgId==displayedMessageId) {
                    pinnedMsgNickname.setText(hnUser.nickname+":");
                }
            });
        });
    }

    private void upPinMsg(View v) {
        if(currentPinnedMsg>=pinnedMsgs.size()-1)return;
        currentPinnedMsg++;
        displayPinnedMsg();
        goToPinnedMsg(v);
    }

    private void downPinMsg(View v) {
        if(currentPinnedMsg==0)return;
        currentPinnedMsg--;
        displayPinnedMsg();
        goToPinnedMsg(v);
    }
}
