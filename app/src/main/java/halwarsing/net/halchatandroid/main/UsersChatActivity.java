package halwarsing.net.halchatandroid.main;

import android.content.Intent;
import android.os.Bundle;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import java.util.ArrayList;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.type.HCChat;

public class UsersChatActivity extends AppCompatActivity {
    private RecyclerView usersRecyclerView;
    private UserChatAdapter userAdapter;
    private HalChat hc;
    private HCChat chat;
    private long chatId;
    private static final String TAG="UCA";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        ImageButton buttonBack,buttonMenu;

        LinearLayout invitePanel;

        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_users_chat);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars() | WindowInsetsCompat.Type.displayCutout());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });

        HalChatApp app=(HalChatApp)getApplicationContext();
        hc=app.getHalChat();

        //Variables
        chatId=getIntent().getLongExtra("uid",-1);
        buttonBack=findViewById(R.id.toolbar_back);
        buttonMenu=findViewById(R.id.toolbar_menu);
        chat=hc.chatGroupChats.getChatInfo(chatId);
        invitePanel=findViewById(R.id.invite_panel);

        buttonMenu.setOnClickListener((View v0)->{
            View popupView = LayoutInflater.from(this).inflate(R.layout.chat_menu, null);

            PopupWindow popupWindow = new PopupWindow(popupView,
                    LinearLayout.LayoutParams.WRAP_CONTENT,
                    LinearLayout.LayoutParams.WRAP_CONTENT,
                    true);

            LinearLayout menuChatSettings = popupView.findViewById(R.id.menuChatSettings);
            LinearLayout menuExitChat = popupView.findViewById(R.id.menuExitChat);

            menuChatSettings.setOnClickListener(v -> {
                Intent intent=new Intent(UsersChatActivity.this,SettingsChatActivity.class);
                intent.putExtra("uid",chatId);
                startActivity(intent);
                overridePendingTransition(R.anim.slide_in_down,R.anim.stay);
                popupWindow.dismiss();
            });

            menuExitChat.setOnClickListener(v -> {
                Toast.makeText(this, "Выход из чата", Toast.LENGTH_SHORT).show();
                popupWindow.dismiss();
            });

            popupWindow.showAsDropDown(buttonMenu, -buttonMenu.getWidth(), 0, Gravity.END);
        });

        buttonBack.setOnClickListener((View v)->{
            finish();
            overridePendingTransition(R.anim.stay,R.anim.slide_out_up);
        });

        invitePanel.setOnClickListener((View v)->{
            Intent intent=new Intent(UsersChatActivity.this,InviteChatActivity.class);
            intent.putExtra("uid",chatId);
            startActivity(intent);
            overridePendingTransition(R.anim.slide_in_down,R.anim.stay);
        });

        usersRecyclerView=findViewById(R.id.users_recycler_view);
        userAdapter=new UserChatAdapter(new ArrayList<>(),this);

        //reverse messages
        LinearLayoutManager layoutManager=new LinearLayoutManager(this);
        layoutManager.setReverseLayout(true);
        layoutManager.setStackFromEnd(true);
        usersRecyclerView.setLayoutManager(layoutManager);
        usersRecyclerView.setAdapter(userAdapter);

        hc.chatUsers.getChatUsers(chatId).thenAccept(users->{
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    for(int i=0;i<users.size();i++) {
                        userAdapter.addUser(users.get(i));
                    }
                }
            });

        });

    }

    @Override
    public void finish() {
        super.finish();
        overridePendingTransition(R.anim.stay,R.anim.slide_out_up);
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        overridePendingTransition(R.anim.stay, R.anim.slide_out_up);
    }
}