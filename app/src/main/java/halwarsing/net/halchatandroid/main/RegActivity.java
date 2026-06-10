package halwarsing.net.halchatandroid.main;

import android.content.Intent;
import android.os.Bundle;

import com.google.android.material.datepicker.MaterialDatePicker;
import com.google.android.material.snackbar.Snackbar;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;

import android.text.Html;
import android.text.method.LinkMovementMethod;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;

import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.navigation.NavController;
import androidx.navigation.Navigation;
import androidx.navigation.ui.AppBarConfiguration;
import androidx.navigation.ui.NavigationUI;

import org.json.JSONException;
import org.json.JSONObject;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;

import halwarsing.net.halchatandroid.databinding.ActivityRegBinding;

import halwarsing.net.halchatandroid.R;

public class RegActivity extends AppCompatActivity {
    private EditText editTextEmail, editTextNickname, editTextPassword;
    private TextView textViewConfirm;
    private Button regButton,regAuthButton;
    private CheckBox checkBoxConfirm,checkBoxConfirmNotifs;
    private HalChat hc;
    private ImageButton buttonBack;

    private static final String TAG="RegAct";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_reg);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars() | WindowInsetsCompat.Type.displayCutout());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });

        HalChatApp app=(HalChatApp)getApplicationContext();
        hc=app.getHalChat();

        //Variables

        buttonBack=findViewById(R.id.toolbar_back);
        editTextEmail=findViewById(R.id.editTextEmail);
        editTextNickname=findViewById(R.id.editTextNickname);
        editTextPassword=findViewById(R.id.editTextPassword);
        regButton=findViewById(R.id.regButton);
        regAuthButton=findViewById(R.id.regAuthButton);
        textViewConfirm=findViewById(R.id.textViewConfirm);
        checkBoxConfirm=findViewById(R.id.checkBoxConfirm);
        checkBoxConfirmNotifs=findViewById(R.id.checkBoxConfirmNotifs);

        //TextHTML
        textViewConfirm.setText(Html.fromHtml(getString(R.string.reg_confirm), Html.FROM_HTML_MODE_LEGACY));
        textViewConfirm.setMovementMethod(LinkMovementMethod.getInstance());

        //Listeners
        regButton.setOnClickListener(v->{
            String email,nickname,password;
            email=editTextEmail.getText().toString();
            nickname=editTextNickname.getText().toString();
            password=editTextPassword.getText().toString();

            boolean confirm=checkBoxConfirm.isChecked();
            boolean confirmNotifs=checkBoxConfirmNotifs.isChecked();

            if(!confirm) {
                Toast.makeText(this,"Регистрация невозможна без согласия с основными документами",Toast.LENGTH_SHORT).show();
                return;
            }

            if(email.isEmpty()) {
                Toast.makeText(this,"Вы не выбрали эл. почту",Toast.LENGTH_SHORT).show();
                return;
            }
            if(nickname.isEmpty()) {
                Toast.makeText(this,"Вы не выбрали имя пользователя",Toast.LENGTH_SHORT).show();
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
                postData.put("nickname",nickname);
                postData.put("password",hc.hh.Str2Hash(password,256,64));
                postData.put("confirm",confirm?"1":"0");
                postData.put("confirmNotifs",confirmNotifs?"1":"0");


                HalNetAPI.apiReqWU("register",postData).thenAccept(data->{
                    try {
                        if(data.getLong("errorCode")==0) {
                            final String id=data.getString("id");
                            runOnUiThread(()->{
                                Intent intent=new Intent(RegActivity.this, ConfirmRegActivity.class);
                                intent.putExtra("id",id);
                                startActivity(intent);
                            });
                        } else {
                            String error=data.getString("error");
                            runOnUiThread(()->Toast.makeText(this,error,Toast.LENGTH_SHORT).show());
                            Log.e(TAG,"register "+data.getLong("errorCode")+";"+data.getString("error"));
                        }
                    } catch (JSONException e) {
                        Log.e(TAG,"register",e);
                    }
                });
            } catch (JSONException e) {
                Log.e(TAG,"Register JSON",e);
            }
        });

        regAuthButton.setOnClickListener(v->{
            Intent intent=new Intent(RegActivity.this, AuthActivity.class);
            startActivity(intent);
        });

        buttonBack.setOnClickListener((View v)->{
            if(!isTaskRoot()) {
                finish();
            } else {
                Intent intent=new Intent(RegActivity.this,MainActivity.class);
                intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP|Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivity(intent);
                finish();
            }
            overridePendingTransition(R.anim.stay,R.anim.slide_out_up);
        });
    }
}