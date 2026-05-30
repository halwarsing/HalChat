package halwarsing.net.halchatandroid.main;

import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.type.HCUser;

public class ChatUserActivity extends AppCompatActivity {
    private long userId,chatId;
    private HalChat hc;
    private HCUser user;
    private static final String TAG="CUA";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        ImageButton buttonBack;
        Button createChatUser;
        ImageView chatUserIcon;
        TextView chatUserNickname;

        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_chat_user);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars() | WindowInsetsCompat.Type.displayCutout());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });

        userId=getIntent().getLongExtra("uid",-1);
        chatId=getIntent().getLongExtra("chatId",-1);
        HalChatApp app=(HalChatApp)getApplicationContext();
        hc=app.getHalChat();

        /*Cursor userCursor=hc.chatUsers.getCursorByUserId(userId);
        user=hc.chatUsers.getChatUserFromCursor(userCursor);
        userCursor.close();*/
        buttonBack=findViewById(R.id.toolbar_back);
        createChatUser=findViewById(R.id.buttonCreateChatUser);
        chatUserIcon=findViewById(R.id.chat_user_icon);
        chatUserNickname=findViewById(R.id.chat_user_nickname);

        hc.chatUsers.getChatUser(chatId,userId).thenAccept(hcuser-> runOnUiThread(() -> {
            user=hcuser;
            chatUserNickname.setText(user.user.nickname);
            hc.hd.getFileById(user.user.icon).thenAccept(file->{
                chatUserIcon.setImageBitmap(BitmapFactory.decodeFile(file.getAbsolutePath()));
            });
        }));

        buttonBack.setOnClickListener((View v)->{
            finish();
            overridePendingTransition(R.anim.stay,R.anim.slide_out_up);
        });

        createChatUser.setOnClickListener((View v)->{
            Toast.makeText(this,"Create chat",Toast.LENGTH_SHORT).show();
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