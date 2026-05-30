package halwarsing.net.halchatandroid.main;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.ImageButton;
import android.widget.Switch;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.FileProvider;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import java.io.File;

import halwarsing.net.halchatandroid.R;

public class SettingsAppActivity extends AppCompatActivity {
    private static final String TAG="SAA";
    private HalChat hc;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        //Variables of elements
        ImageButton buttonBack;
        Button exitAccount,privacyBtn,userAgreementBtn;
        Switch switchTRPSW,switchMute;

        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_settings_app);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars() | WindowInsetsCompat.Type.displayCutout());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });

        //Initialize variables
        HalChatApp app=(HalChatApp)getApplicationContext();
        hc=app.getHalChat();
        buttonBack=findViewById(R.id.toolbar_back);
        exitAccount=findViewById(R.id.exitAccountBtn);
        privacyBtn=findViewById(R.id.privacyBtn);
        userAgreementBtn=findViewById(R.id.userAgreementBtn);
        switchTRPSW=findViewById(R.id.switch_trpsw);
        switchMute=findViewById(R.id.switch_mute);

        //Set parameters
        switchTRPSW.setChecked(hc.chatSettingsApp.getBoolean(HalChatSettingsApp.KEY_TRANSFER_PASSWORDS));
        switchMute.setChecked(hc.chatSettingsApp.getBoolean(HalChatSettingsApp.KEY_MUTE_NOTIFICATIONS));

        //Set clickable buttons and panel
        buttonBack.setOnClickListener((View v)->{
            if(!isTaskRoot()) {
                finish();
            } else {
                Intent intent=new Intent(SettingsAppActivity.this,MainActivity.class);
                intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP|Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivity(intent);
                finish();
            }
        });

        exitAccount.setOnClickListener((View v)->{
            hc.exitAccount(this,this);
            Log.d(TAG,"Successfull exit account");
            Toast.makeText(this,"Успешно",Toast.LENGTH_SHORT).show();
            Intent intent=new Intent(SettingsAppActivity.this,MainActivity.class);
            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP|Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(intent);
            finish();
        });

        userAgreementBtn.setOnClickListener((View v)->{
            Toast.makeText(this,"Подождите, идёт загрузка...",Toast.LENGTH_SHORT).show();
            hc.hd.getFileById(HalChat.ID_USER_AGREEMENT_DOC).thenAccept(file->{
                File pdfFile=new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS),file.getName().split("\\.")[0]+".pdf");

                try {
                    if(!pdfFile.exists()) {
                        pdfFile.createNewFile();
                    }
                    HalChatFunctionsLib.copyFile(file,pdfFile);

                    Uri contentUri = FileProvider.getUriForFile(
                            this,
                            getPackageName() + ".provider",
                            pdfFile
                    );

                    runOnUiThread(() -> {
                        Intent open = new Intent(Intent.ACTION_VIEW);
                        open.setDataAndType(contentUri, "application/pdf");
                        open.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                        startActivity(Intent.createChooser(open, "Открыть PDF"));
                    });
                } catch (Exception e) {
                    Log.e(TAG,"Failed to copy file",e);
                }
            });
        });

        privacyBtn.setOnClickListener((View v)->{
            Toast.makeText(this,"Подождите, идёт загрузка...",Toast.LENGTH_SHORT).show();
            hc.hd.getFileById(HalChat.ID_PRIVACY_DOC).thenAccept(file->{
                File pdfFile=new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS),file.getName()+".pdf");

                try {
                    if(!pdfFile.exists()) {
                        pdfFile.createNewFile();
                    }
                    HalChatFunctionsLib.copyFile(file,pdfFile);

                    Uri contentUri = FileProvider.getUriForFile(
                            this,
                            getPackageName() + ".provider",
                            pdfFile
                    );

                    runOnUiThread(() -> {
                        Intent open = new Intent(Intent.ACTION_VIEW);
                        open.setDataAndType(contentUri, "application/pdf");
                        open.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                        startActivity(Intent.createChooser(open, "Открыть PDF"));
                    });
                } catch (Exception e) {
                    Log.e(TAG,"Failed to copy file",e);
                }
            });
        });

        //switches
        switchTRPSW.setOnCheckedChangeListener((CompoundButton buttonView, boolean isChecked)->{
            hc.chatSettingsApp.setParameter(HalChatSettingsApp.KEY_TRANSFER_PASSWORDS,isChecked);
        });

        switchMute.setOnCheckedChangeListener((CompoundButton buttonView, boolean isChecked)->{
            hc.chatSettingsApp.setParameter(HalChatSettingsApp.KEY_MUTE_NOTIFICATIONS,isChecked);
        });
    }

    @Override
    public void finish() {
        super.finish();
        overridePendingTransition(R.anim.slide_in_right, R.anim.slide_out_left);
    }
}