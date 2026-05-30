package halwarsing.net.halchatandroid.main;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.Objects;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.encryption.RSAEncryptor;

public class CreatePrivateChatActivity extends AppCompatActivity {
    private final static String TAG="CreatePrivateChatAct";
    private HalChat hc;
    private long toId;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);


        HalChatApp app=(HalChatApp)getApplicationContext();
        hc=app.getHalChat();

        setContentView(R.layout.activity_create_password_chat);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars() | WindowInsetsCompat.Type.displayCutout());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });
        Toolbar toolbar=findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        Objects.requireNonNull(getSupportActionBar()).setDisplayShowTitleEnabled(false);

        toId=getIntent().getLongExtra("toId",-1);

        if(hc.chatGroupChats.hasPrivateChat(toId)) {
            Intent intent=new Intent(CreatePrivateChatActivity.this,ChatActivity.class);
            intent.putExtra("uid",hc.chatGroupChats.getPrivateChat(toId));
            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP|Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(intent);
            overridePendingTransition(R.anim.stay,R.anim.slide_out_up);
            finish();
        }

        EditText editText=findViewById(R.id.editTextPasswordChat);
        Button button=findViewById(R.id.createPasswordChatButton);
        button.setOnClickListener((View v)->{
            String passwordText=editText.getText().toString();
            //FUC проверку на длину пароля и проверка сложности
            if(passwordText.isEmpty()) {
                Log.d(TAG,"Password is empty");
                runOnUiThread(()->
                        Toast.makeText(this,"Вы не ввели пароль",Toast.LENGTH_SHORT).show());
                return;
            }

            button.setEnabled(false);

            String p=hc.hh.Str2Hash(passwordText,100,10);
            String hashP=hc.hh.Str2Hash(p,100,10);
            String encP=RSAEncryptor.encryptPassword(p);
            Log.d(TAG,"Start create chat");
            TaskExecutorManager.getInstance().submitSend("createPrivateChat:"+toId,()->{
                try {
                    //Создаётся новый чат
                    JSONObject postData=new JSONObject();
                    postData.put("password",encP);
                    postData.put("hashPassword",hashP);
                    postData.put("userId",toId);
                    hc.chatWS.apiReq("createPrivateChat",postData).thenAccept(res->{
                        try {
                            if(res.getInt("errorCode")==0) {
                                Log.d(TAG,"Successful create private chat");
                                runOnUiThread(()->Toast.makeText(CreatePrivateChatActivity.this,"Подождите, идёт загрузка чата...",Toast.LENGTH_SHORT).show());
                                long chatId=res.getLong("chatUID");
                                hc.chatGroupChats.addChat(chatId,p).thenAccept(n->{
                                    hc.db.execSQL("UPDATE `groupChats` SET `password`=? WHERE `chatUID`=?", new String[]{p, String.valueOf(chatId)});
                                    Intent intent=new Intent(CreatePrivateChatActivity.this,ChatActivity.class);
                                    intent.putExtra("uid",chatId);
                                    startActivity(intent);
                                });
                            } else {
                                Log.e(TAG,"Error create private chat: "+res.getInt("errorCode")+";"+res.getString("error"));
                                runOnUiThread(()->{button.setEnabled(true);Toast.makeText(CreatePrivateChatActivity.this,"Ошибка: чат не создан",Toast.LENGTH_SHORT).show();});
                            }
                        } catch (Exception e) {
                            runOnUiThread(()->button.setEnabled(true));
                            Log.e(TAG,"createPrivateChat",e);
                        }
                    });
                } catch (JSONException e) {
                    Log.e(TAG, "Failed to fetch HalChat API", e);
                    runOnUiThread(()->button.setEnabled(true));
                }
                return null;
            });
        });
    }
}