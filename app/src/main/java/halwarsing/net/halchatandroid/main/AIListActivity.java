package halwarsing.net.halchatandroid.main;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ImageButton;
import android.widget.LinearLayout;
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
import halwarsing.net.halchatandroid.encryption.PasswordGenerator;
import halwarsing.net.halchatandroid.encryption.RSAEncryptor;

public class AIListActivity extends AppCompatActivity {
    private HalChat hc;
    private LinearLayout mira,eva,kira;
    private static final String TAG="AIListA";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_ailist);

        HalChatApp app=(HalChatApp)getApplicationContext();
        hc=app.getHalChat();

        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });


        Toolbar toolbar=findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        Objects.requireNonNull(getSupportActionBar()).setDisplayShowTitleEnabled(false);

        ImageButton buttonBack=findViewById(R.id.toolbar_back);
        buttonBack.setOnClickListener((View v)->{
            finish();
        });


        mira=findViewById(R.id.mira);
        eva=findViewById(R.id.eva);
        kira=findViewById(R.id.kira);

        mira.setOnClickListener(v->{
            createAIChat(68,"Мира");
        });

        eva.setOnClickListener(v->{
            createAIChat(69,"Ева");
        });

        kira.setOnClickListener(v->{
            createAIChat(70,"Кира");
        });
    }

    private void createAIChat(long marketId, String name) {
        JSONObject postData=new JSONObject();

        Toast.makeText(this,"Подождите, идёт создание чата...",Toast.LENGTH_SHORT).show();

        char[] pswChars= PasswordGenerator.generatePassword(16);
        String p=hc.hh.Str2Hash(new String(pswChars),100,10);
        String hashP=hc.hh.Str2Hash(p,100,10);
        String encP=RSAEncryptor.encryptPassword(p);

        try {
            postData.put("password",encP);
            postData.put("hashPassword",hashP);
            postData.put("name",name);
            postData.put("type",0);
            postData.put("publicType","false");
            postData.put("description","");
            postData.put("marketId",marketId);

            hc.hcapi.apiReq("createChat",postData).thenAccept(res->{
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
                        runOnUiThread(()->{Toast.makeText(AIListActivity.this,"Ошибка: чат не создан",Toast.LENGTH_SHORT).show();});
                        Log.e(TAG,"ERROR: "+res.getString("error"));
                    }
                } catch (Exception e) {
                    Log.e(TAG,"createChat",e);
                }
            });
        } catch (JSONException e) {
            Log.e(TAG,"Create AI Chat JSON: ",e);
        }

    }
}