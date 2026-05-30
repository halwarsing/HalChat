package halwarsing.net.halchatandroid.main;

import android.content.Intent;
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
import halwarsing.net.halchatandroid.type.HNUser;

public class UserActivity extends AppCompatActivity {
    private long userId;
    private HalChat hc;
    private HNUser user;
    private static final String TAG="USERAct";

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
        HalChatApp app=(HalChatApp)getApplicationContext();
        hc=app.getHalChat();

        /*Cursor userCursor=hc.chatUsers.getCursorByUserId(userId);
        user=hc.chatUsers.getChatUserFromCursor(userCursor);
        userCursor.close();*/
        buttonBack=findViewById(R.id.toolbar_back);
        createChatUser=findViewById(R.id.buttonCreateChatUser);
        chatUserIcon=findViewById(R.id.chat_user_icon);
        chatUserNickname=findViewById(R.id.chat_user_nickname);

        hc.hnUsers.getUserByUserId(userId).thenAccept(hnuser-> runOnUiThread(() -> {
            user=hnuser;
            chatUserNickname.setText(user.nickname);
            hc.hd.getFileById(user.icon).thenAccept(file->{
                chatUserIcon.setImageBitmap(BitmapFactory.decodeFile(file.getAbsolutePath()));
            });
        }));




        buttonBack.setOnClickListener((View v)->{
            if(!isTaskRoot()) {
                finish();
            } else {
                Intent intent=new Intent(UserActivity.this,MainActivity.class);
                intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP|Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivity(intent);
                finish();
            }
            overridePendingTransition(R.anim.stay,R.anim.slide_out_up);
        });

        createChatUser.setOnClickListener((View v)->{
            Intent intent=new Intent(UserActivity.this,CreatePrivateChatActivity.class);
            intent.putExtra("toId",userId);
            startActivity(intent);
        });
    }
}