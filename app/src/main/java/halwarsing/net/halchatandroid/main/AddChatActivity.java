package halwarsing.net.halchatandroid.main;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import java.util.Arrays;
import java.util.List;
import java.util.Objects;

import halwarsing.net.halchatandroid.R;

//Страница добавления нового чаты
public class AddChatActivity extends AppCompatActivity {
    private final static String TAG="HCAAddChat";
    private long chatUid;
    private final List<String> publicChatList= Arrays.asList("Публичный","Приватный");
    private final List<String> typeChatList= Arrays.asList("Обычный","С постами");
    private HalChat hc;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);

        HalChatApp app=(HalChatApp)getApplicationContext();
        hc=app.getHalChat();

        setContentView(R.layout.activity_add_chat);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars() | WindowInsetsCompat.Type.displayCutout());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });
        Toolbar toolbar=findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        Objects.requireNonNull(getSupportActionBar()).setDisplayShowTitleEnabled(false);

        Spinner publicSpinner=findViewById(R.id.spinnerPublicChat);
        ArrayAdapter publicChatAdapter=new ArrayAdapter(this,R.layout.selected_item,publicChatList);
        publicChatAdapter.setDropDownViewResource(R.layout.dropdown_item);
        publicSpinner.setAdapter(publicChatAdapter);

        Spinner typeSpinner=findViewById(R.id.spinnerTypeChat);
        ArrayAdapter typeChatAdapter=new ArrayAdapter(this,R.layout.selected_item,typeChatList);
        typeChatAdapter.setDropDownViewResource(R.layout.dropdown_item);
        typeSpinner.setAdapter(typeChatAdapter);

        EditText editText=findViewById(R.id.editTextNameChat);

        Button createChatBtn=findViewById(R.id.createChatButton);
        createChatBtn.setOnClickListener((View v)->{
            //Переход на страницу с придумыванием пароля для чата
            Intent intent=new Intent(AddChatActivity.this, CreatePasswordChatActivity.class);
            String nameChat=editText.getText().toString();
            intent.putExtra("codeUser",hc.codeUser);
            intent.putExtra("nameChat",nameChat);
            intent.putExtra("publicChat",publicSpinner.getSelectedItemId());
            intent.putExtra("typeChat",typeSpinner.getSelectedItemId());
            //FUC проверка на другие соответствия названию чата
            if(nameChat.isEmpty()) {
                Log.d(TAG,"Name Chat is empty");
                //Пользователь не указал название чата
                Toast.makeText(this,"Название чата не заполнено",Toast.LENGTH_LONG).show();
                return;
            }
            startActivity(intent);
        });

    }
}