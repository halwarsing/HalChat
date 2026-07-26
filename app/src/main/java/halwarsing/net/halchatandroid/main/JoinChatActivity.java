package halwarsing.net.halchatandroid.main;

import android.content.Intent;
import android.database.sqlite.SQLiteDatabase;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import com.bumptech.glide.Glide;

import java.util.Objects;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.type.HCChat;

//Страница для входа в существующий чат
public class JoinChatActivity extends AppCompatActivity {
    private final static String TAG="HCAJoinChat";
    private long chatUid;
    private HalChat hc;
    private HCChat chat;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);

        HalChatApp app=(HalChatApp)getApplicationContext();
        hc=app.getHalChat();

        setContentView(R.layout.activity_join_chat);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars() | WindowInsetsCompat.Type.displayCutout());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });
        Toolbar toolbar=findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        Objects.requireNonNull(getSupportActionBar()).setDisplayShowTitleEnabled(false);

        TextView titleMenu=findViewById(R.id.toolbar_title);
        titleMenu.setText(getIntent().getStringExtra("name"));

        chatUid=getIntent().getLongExtra("uid",-1);
        if (chatUid==-1) {
            Log.e(TAG,"No uid chat");
            finishAndRemoveTask();
        }

        chat=hc.chatGroupChats.getChatInfo(chatUid);

        if(chat==null) {
            Log.e(TAG,"No uid chat");
            finishAndRemoveTask();
        }

        //Variables
        Button enterButton=findViewById(R.id.buttonEnter);
        Button exitChatButton=findViewById(R.id.buttonExitChat);
        EditText inputPass=findViewById(R.id.editTextPassword);
        TextView nameChat=findViewById(R.id.nameChat);
        TextView descChat=findViewById(R.id.descChat);
        ImageView iconChat=findViewById(R.id.iconChat);
        LinearLayout chatInfoLL=findViewById(R.id.chatInfoLL);

        //Set Info Chat
        nameChat.setText(chat.name);
        if(chat.description==null||chat.description.isEmpty()) {
            chatInfoLL.removeView(descChat);
        } else {
            descChat.setText(chat.description);
        }
        hc.hd.getFileById(chat.icon).thenAccept(file->{
            Glide.with(this)
                    .load(file)
                    .override(150, 150)
                    .placeholder(R.drawable.ic_add_people)
                    .into(iconChat);
        });

        //Events
        enterButton.setOnClickListener(v->{
            String password=inputPass.getText().toString();
            //FUC проверка на длину и др. параметров пароля
            if (!password.isEmpty()) {
                //Проверка подходит ли пароль, если правильный, то входит в чат
                TaskExecutorManager.getInstance().submitChatSync("enterChat:"+chatUid,()->{
                    String origPassword=hc.hh.Str2Hash(password,100,10,100000);
                    hc.chatGroupChats.enterChatWithoutThread(chatUid, origPassword,hc.hh.Str2Hash(origPassword,100,10,100000)).thenAccept(a->{
                        if (a) {
                            runOnUiThread(()->{
                                Toast.makeText(this, "Пароль подошёл", Toast.LENGTH_SHORT).show();
                                Intent intent=new Intent(this,ChatActivity.class);
                                intent.putExtra("uid",chatUid);
                                startActivity(intent);
                            });
                        } else {
                            runOnUiThread(()->Toast.makeText(this, "Неправильный пароль", Toast.LENGTH_SHORT).show());
                        }
                    });
                    return null;
                });
            }
        });

        exitChatButton.setOnClickListener((View v) ->{
            hc.chatGroupChats.exitChat(chatUid).thenAccept(d->{
                if(d) {
                    Intent intent=new Intent(JoinChatActivity.this,MainActivity.class);
                    startActivity(intent);
                }
            });
        });
    }
}