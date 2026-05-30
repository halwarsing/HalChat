package halwarsing.net.halchatandroid.main;

import android.content.Intent;
import android.database.sqlite.SQLiteDatabase;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import org.json.JSONException;
import org.json.JSONObject;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.type.HNUser;

public class AuthActivity extends AppCompatActivity {
    private EditText editTextEmail, editTextPassword;
    private Button authButton;
    private HalChat hc;
    private ImageButton buttonBack;
    private static final String TAG="AuthAct";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_auth);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            int topPadding = insets.getInsets(WindowInsetsCompat.Type.systemBars()).top;
            v.setPadding(v.getPaddingLeft(), topPadding, v.getPaddingRight(), v.getPaddingBottom());
            return insets;
        });

        HalChatApp app=(HalChatApp)getApplicationContext();
        hc=app.getHalChat();

        //Variables8b54e@deltajohnsons.com

        buttonBack=findViewById(R.id.toolbar_back);
        editTextEmail=findViewById(R.id.editTextEmail);
        editTextPassword=findViewById(R.id.editTextPassword);
        authButton=findViewById(R.id.authButton);

        //Listeners
        authButton.setOnClickListener(v->{
            final String email, password;
            email=editTextEmail.getText().toString();
            password=editTextPassword.getText().toString();
            if(email.isEmpty()) {
                Toast.makeText(this,"Вы не выбрали эл. почту",Toast.LENGTH_SHORT).show();
                return;
            }

            if(password.isEmpty()) {
                Toast.makeText(this,"Вы не написали пароль",Toast.LENGTH_SHORT).show();
                return;
            }

            if(password.length()<8) {
                Toast.makeText(this,"Пароль должен быть длиной от 8 символов",Toast.LENGTH_SHORT).show();
                return;
            }

            try {
                JSONObject postData=new JSONObject();
                postData.put("email",email);
                postData.put("password",hc.hh.Str2Hash(password,256,64));

                HalNetAPI.apiReqWU("auth",postData).thenAccept(data->{
                    try {
                        if(data.getLong("errorCode")==0) {
                            long id;
                            String uid,nickname,icon;

                            id=data.getLong("id");
                            uid=data.getString("uid");
                            nickname=data.getString("nickname");
                            icon=data.getString("icon");

                            HalChatDatabaseHelper dbHelper=new HalChatDatabaseHelper(AuthActivity.this);
                            SQLiteDatabase db=dbHelper.getWritableDatabase();
                            db.execSQL("INSERT INTO `sessions` (`id`, `fromId`) VALUES (?, ?)",new String[]{uid, String.valueOf(id)});
                            HalNetUsers halNetUsers=new HalNetUsers(null,db,new HalDrive(AuthActivity.this,db,uid));
                            HNUser newuser=new HNUser(-1,id,nickname,icon,false,"");
                            halNetUsers.addUser(newuser);
                            hc.init(AuthActivity.this);

                            runOnUiThread(()-> {
                                Intent intent = new Intent(AuthActivity.this, MainActivity.class);
                                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
                                startActivity(intent);
                            });
                        } else {
                            String error=data.getString("error");
                            runOnUiThread(()->Toast.makeText(this,error,Toast.LENGTH_SHORT).show());
                            Log.e(TAG,"auth "+data.getLong("errorCode")+";"+data.getString("error"));
                        }
                    } catch (JSONException e) {
                        Log.e(TAG,"auth",e);
                    }
                });
            } catch (JSONException e) {
                Log.e(TAG,"Auth JSON",e);
            }
        });

        buttonBack.setOnClickListener((View v)->{
            if(!isTaskRoot()) {
                finish();
            } else {
                Intent intent=new Intent(AuthActivity.this,MainActivity.class);
                intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP|Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivity(intent);
                finish();
            }
            overridePendingTransition(R.anim.stay,R.anim.slide_out_up);
        });
    }
}