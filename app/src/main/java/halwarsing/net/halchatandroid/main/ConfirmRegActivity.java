package halwarsing.net.halchatandroid.main;

import android.content.Intent;
import android.database.sqlite.SQLiteDatabase;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
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

public class ConfirmRegActivity extends AppCompatActivity {
    private HalChat hc;

    private EditText editTextCode;
    private Button confirmButton;
    private String id;
    private ImageButton buttonBack;

    private static final String TAG="CREGAct";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_confirm_reg);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars() | WindowInsetsCompat.Type.displayCutout());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });

        HalChatApp app=(HalChatApp)getApplicationContext();
        hc=app.getHalChat();

        //Variables

        id=getIntent().getStringExtra("id");

        buttonBack=findViewById(R.id.toolbar_back);
        editTextCode=findViewById(R.id.editTextCode);
        confirmButton=findViewById(R.id.confirmButton);

        //Listeners
        confirmButton.setOnClickListener(v->{
            final String code=editTextCode.getText().toString();

            if(code.length()<6) {
                Toast.makeText(this,"Введите код",Toast.LENGTH_SHORT).show();
                return;
            }
            try {
                JSONObject postData=new JSONObject();
                postData.put("id",id);
                postData.put("codeReg",code);
                HalNetAPI.apiReqWU("confirmReg",postData).thenAccept(data->{
                    try {
                        if(data.getLong("errorCode")==0) {
                            long id;
                            String uid,nickname,icon;

                            id=data.getLong("id");
                            uid=data.getString("uid");
                            nickname=data.getString("nickname");
                            icon=data.getString("icon");

                            HalChatDatabaseHelper dbHelper=new HalChatDatabaseHelper(ConfirmRegActivity.this);
                            SQLiteDatabase db=dbHelper.getWritableDatabase();
                            db.execSQL("INSERT INTO `sessions` (`id`, `fromId`) VALUES (?, ?)",new String[]{uid, String.valueOf(id)});
                            HalNetUsers halNetUsers=new HalNetUsers(null,db,new HalDrive(ConfirmRegActivity.this,db,uid));
                            HNUser newuser=new HNUser(-1,id,nickname,icon,false,"");
                            halNetUsers.addUser(newuser);
                            hc.init(ConfirmRegActivity.this);

                            runOnUiThread(()-> {
                                Intent intent = new Intent(ConfirmRegActivity.this, MainActivity.class);
                                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
                                startActivity(intent);
                            });
                        } else {
                            final String error=data.getString("error");
                            runOnUiThread(()->Toast.makeText(this,error,Toast.LENGTH_SHORT).show());
                            Log.e(TAG,"confirmReg "+data.getLong("errorCode")+";"+error);
                        }
                    } catch (JSONException e) {
                        Log.e(TAG,"confirmReg",e);
                    }
                });
            } catch (JSONException e) {
                Log.e(TAG,"creg listener",e);
            }
        });

        buttonBack.setOnClickListener((View v)->{
            if(!isTaskRoot()) {
                finish();
            } else {
                Intent intent=new Intent(ConfirmRegActivity.this,MainActivity.class);
                intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP|Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivity(intent);
                finish();
            }
            overridePendingTransition(R.anim.stay,R.anim.slide_out_up);
        });
    }
}