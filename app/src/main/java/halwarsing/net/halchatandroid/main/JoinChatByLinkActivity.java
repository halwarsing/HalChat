package halwarsing.net.halchatandroid.main;

import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import com.bumptech.glide.Glide;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.type.HCChat;

public class JoinChatByLinkActivity extends AppCompatActivity {

    private ImageView iconChat;
    private TextView nameChat;
    private Button joinChatBtn;
    private HalChat hc;
    private HCChat chat;
    private static final String TAG="JCBLA";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_join_chat_by_link);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });

        HalChatApp app=(HalChatApp)getApplicationContext();
        hc=app.getHalChat();

        //Variables
        iconChat=findViewById(R.id.iconChat);
        nameChat=findViewById(R.id.nameChat);
        joinChatBtn=findViewById(R.id.joinChatBtn);

        //Get Info
        String chatId=getIntent().getStringExtra("chatId");
        String psw=getIntent().getStringExtra("psw");

        hc.chatGroupChats.getInfoChatOnline(Long.parseLong(chatId)).thenAccept(c->{
            this.chat=c;

            hc.hd.getFileById(c.icon).thenAccept(fileIcon->{
                new Handler(Looper.getMainLooper()).post(() -> {
                    Glide.with(this)
                            .asDrawable()
                            .load(fileIcon)
                            .override(150, 150)
                            .placeholder(R.drawable.ic_robot)
                            .into(iconChat);
                });
            });

            nameChat.setText(c.name);
        });

        //OnClick
        joinChatBtn.setOnClickListener(v->{
            TaskExecutorManager.getInstance().submitDecryptChatActivity("enterChat:"+chatId,()->{
                runOnUiThread(()->joinChatBtn.setEnabled(false));
                hc.chatGroupChats.enterNewChatWithoutThread(Long.parseLong(chatId), psw,hc.hh.Str2Hash(psw,100,10,100000)).thenAccept(a->{
                    if (a) {
                        runOnUiThread(()->{
                            Intent intent=new Intent(JoinChatByLinkActivity.this,ChatActivity.class);
                            intent.putExtra("uid",Long.parseLong(chatId));
                            startActivity(intent);
                        });
                    } else {
                        runOnUiThread(()->{Toast.makeText(this, "Неправильный пароль", Toast.LENGTH_SHORT).show();joinChatBtn.setEnabled(true);});
                    }
                });
                return null;
            });
        });
    }
}