package halwarsing.net.halchatandroid.main;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.Spinner;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.recyclerview.widget.RecyclerView;

import java.util.ArrayList;
import java.util.List;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.type.HCChat;

public class SettingsChatActivity extends AppCompatActivity {
    private RecyclerView usersRecyclerView;
    private UserChatAdapter userAdapter;
    private HalChat hc;
    private HCChat chat;
    private long chatId;

    private Button changeAvatar,saveButton;
    private EditText editTextName,editTextId;
    private Spinner spinnerPublic,spinnerType;

    private static final String[] listPublic={"Публичный","Приватный"};
    private static final String[] listType={"Обычный","С постами"};

    protected void setValues() {
        //Set values
        editTextName.setText(chat.name);
        editTextId.setText(chat.id);
        spinnerPublic.setSelection(chat.publicType==1?0:1);
        spinnerType.setSelection(chat.chatType==2?1:0);
    }

    protected Void onUpdateSettings(HCChat nc) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                chat=nc;
                setValues();
                Toast.makeText(SettingsChatActivity.this,"Настройки обновлены", Toast.LENGTH_SHORT).show();
            }
        });
        return null;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_settings_chat);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars() | WindowInsetsCompat.Type.displayCutout());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });
        ImageButton buttonBack;

        HalChatApp app=(HalChatApp)getApplicationContext();
        hc=app.getHalChat();

        //Variables
        chatId=getIntent().getLongExtra("uid",-1);
        buttonBack=findViewById(R.id.toolbar_back);
        chat=hc.chatGroupChats.getChatInfo(chatId);
        changeAvatar=(Button)findViewById(R.id.buttonChangeAvatar);
        editTextName=(EditText)findViewById(R.id.editTextNameChat);
        editTextId=(EditText)findViewById(R.id.editTextIdChat);
        spinnerPublic=(Spinner)findViewById(R.id.selectPublicChat);
        spinnerType=(Spinner)findViewById(R.id.selectTypeChat);
        saveButton=(Button)findViewById(R.id.buttonSaveSettings);

        //Init lists
        ArrayAdapter<String> adapterPublic = new ArrayAdapter(this, android.R.layout.simple_spinner_item, listPublic);
        ArrayAdapter<String> adapterType = new ArrayAdapter(this, android.R.layout.simple_spinner_item, listType);
        adapterPublic.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        adapterType.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinnerPublic.setAdapter(adapterPublic);
        spinnerType.setAdapter(adapterType);

        setValues();

        buttonBack.setOnClickListener((View v)->{
            finish();
            overridePendingTransition(R.anim.stay,R.anim.slide_out_up);
        });

        saveButton.setOnClickListener((View v)->{
            List<UpdateChatSetting> updateChatSettingList=new ArrayList<>();
            String strValue;
            int intValue;
            UpdateChatSetting updateChatSetting;

            strValue=editTextName.getText().toString().trim();
            if(!strValue.isEmpty()&&!strValue.equals(chat.name)) {
                updateChatSetting=new UpdateChatSetting("name");
                updateChatSetting.setValue(strValue);
                updateChatSettingList.add(updateChatSetting);
            }

            strValue=editTextId.getText().toString().trim();
            if(!strValue.isEmpty()&&!strValue.equals(chat.id)) {
                updateChatSetting=new UpdateChatSetting("id");
                updateChatSetting.setValue(strValue);
                updateChatSettingList.add(updateChatSetting);
            }

            intValue=spinnerPublic.getSelectedItemPosition()==0?1:0;
            if(intValue!=chat.publicType) {
                updateChatSetting=new UpdateChatSetting("public");
                updateChatSetting.setValue(intValue);
                updateChatSettingList.add(updateChatSetting);
            }

            intValue=spinnerType.getSelectedItemPosition()==0?0:2;
            if(intValue!=chat.publicType) {
                updateChatSetting=new UpdateChatSetting("type");
                updateChatSetting.setValue(intValue);
                updateChatSettingList.add(updateChatSetting);
            }

            hc.chatGroupChats.updateChatSettings(chat,updateChatSettingList,this::onUpdateSettings);
        });

        changeAvatar.setOnClickListener((View v)->{
            Intent intent=new Intent(SettingsChatActivity.this,ChangeAvatarActivity.class);
            intent.putExtra("uid",chatId);
            startActivity(intent);
            overridePendingTransition(R.anim.slide_in_down,R.anim.stay);
        });
    }

    @Override
    public void finish() {
        super.finish();
        overridePendingTransition(R.anim.stay,R.anim.slide_out_up);
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        overridePendingTransition(R.anim.stay, R.anim.slide_out_up);
    }
}