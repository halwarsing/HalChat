package halwarsing.net.halchatandroid.main;

import android.content.Intent;
import android.os.Bundle;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;

import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import androidx.appcompat.widget.Toolbar;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.Objects;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.encryption.PasswordGenerator;
import halwarsing.net.halchatandroid.encryption.RSAEncryptor;

//Придумывание новому чату пароля
public class CreatePasswordChatActivity extends AppCompatActivity {
    private final static String TAG="HCACreatePasswordChat";
    private long chatUid;
    private HalChat hc;
    private final String[] types=new String[]{"0","2"};

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);

        String nameChat,publicChat,typeChat;

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

        nameChat=getIntent().getStringExtra("nameChat");
        publicChat=getIntent().getLongExtra("publicChat",-1)==0?"true":"false";
        typeChat=types[(int) getIntent().getLongExtra("typeChat",-1)];

        EditText editText=findViewById(R.id.editTextPasswordChat);
        Button button=findViewById(R.id.createPasswordChatButton);
        Button generatePswBtn=findViewById(R.id.generatePasswordBtn);


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
            Toast.makeText(this,"Подождите, идёт создание чата...",Toast.LENGTH_SHORT).show();
            try {
                //Создаётся новый чат
                JSONObject postData=new JSONObject();
                postData.put("password",encP);
                postData.put("hashPassword",hashP);
                postData.put("name",nameChat);
                postData.put("type",typeChat);
                postData.put("publicType",publicChat);
                postData.put("description","");
                hc.chatWS.apiReq("createChat",postData).thenAccept(res->{
                    try {
                        if(res.getInt("errorCode")==0) {
                            Log.d(TAG,"Successful create chat");
                            runOnUiThread(()-> Toast.makeText(this,"Подождите, идёт загрузка чата...",Toast.LENGTH_SHORT).show());
                            long chatId=res.getLong("chatId");

                            hc.chatGroupChats.addChat(chatId,p).thenAccept(n->{
                                hc.db.execSQL("UPDATE `groupChats` SET `password`=? WHERE `chatUID`=?", new String[]{p, String.valueOf(chatId)});
                                Intent intent=new Intent(this,ChatActivity.class);
                                intent.putExtra("uid",chatId);
                                startActivity(intent);
                            });
                        } else {
                            runOnUiThread(()->{Toast.makeText(CreatePasswordChatActivity.this,"Ошибка: чат не создан",Toast.LENGTH_SHORT).show();button.setEnabled(true);});
                            Log.e(TAG,"ERROR: "+res.getString("error"));
                        }
                    } catch (Exception e) {
                        runOnUiThread(()->button.setEnabled(true));
                        Log.e(TAG,"createChat",e);
                    }
                });
            } catch (JSONException e) {
                runOnUiThread(()->button.setEnabled(true));
                Log.e(TAG, "Failed to fetch HalChat API", e);
            }
        });

        generatePswBtn.setOnClickListener(v -> {
            char[] pswChars=PasswordGenerator.generatePassword(16);
            editText.setText(pswChars,0,16);
            PasswordGenerator.clearPassword(pswChars);
        });
    }
}