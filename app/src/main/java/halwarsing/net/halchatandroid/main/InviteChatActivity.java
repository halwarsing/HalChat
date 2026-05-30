package halwarsing.net.halchatandroid.main;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.recyclerview.widget.DividerItemDecoration;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.type.HCChat;
import halwarsing.net.halchatandroid.type.HNUser;
import halwarsing.net.halchatandroid.type.SearchUser;

public class InviteChatActivity extends AppCompatActivity {
    private HCChat chat;
    private long chatId;
    private HalChat hc;
    private Button createLinkWP, createShortLinkWP;
    private EditText findUsersEdit;
    private RecyclerView usersView;
    private InviteUsersListAdapter inviteUsersListAdapter;
    private static final String TAG="ICA";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_invite_chat);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars() | WindowInsetsCompat.Type.displayCutout());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });

        ImageButton buttonBack;

        HalChatApp app = (HalChatApp) getApplicationContext();
        hc = app.getHalChat();

        //Variables
        chatId = getIntent().getLongExtra("uid", -1);
        buttonBack = findViewById(R.id.toolbar_back);
        chat = hc.chatGroupChats.getChatInfo(chatId);
        createLinkWP = findViewById(R.id.createLinkWithPassword);
        createShortLinkWP = findViewById(R.id.createShortLinkWithPassword);
        findUsersEdit=findViewById(R.id.editTextFindUsers);
        usersView=findViewById(R.id.usersView);

        //Adapter
        usersView.setLayoutManager(new LinearLayoutManager(this));
        DividerItemDecoration dividerItemDecoration=new DividerItemDecoration(usersView.getContext(), LinearLayout.VERTICAL);
        usersView.addItemDecoration(dividerItemDecoration);
        inviteUsersListAdapter=new InviteUsersListAdapter(this,hc);
        usersView.setAdapter(inviteUsersListAdapter);

        //On click
        buttonBack.setOnClickListener((View v) -> {
            finish();
            overridePendingTransition(R.anim.stay, R.anim.slide_out_up);
        });

        createLinkWP.setOnClickListener((View v) -> {
            Intent sendIntent = new Intent();
            sendIntent.setAction(Intent.ACTION_SEND);
            sendIntent.putExtra(Intent.EXTRA_TEXT, "Присоединяйся к чату: " + "https://halchat.halwarsing.net/join/" + chat.id + "/" + hc.hh.Str2Hash(chat.password, 100, 10) + "#" + chat.password);
            sendIntent.setType("text/plain");
            startActivity(Intent.createChooser(sendIntent, "Пригласить в HalChat"));
        });

        createShortLinkWP.setOnClickListener(v -> {
            JSONObject postData = new JSONObject();
            try {
                postData.put("chatId", chat.chatUID);
                postData.put("psw", chat.password);
                hc.chatWS.apiReq("createShortLink", postData).thenAccept(res -> {
                    try {
                        if (res.getLong("errorCode") == 0) {
                            Log.d(TAG, "Successful createShortLink");
                            Intent sendIntent = new Intent();
                            sendIntent.setAction(Intent.ACTION_SEND);
                            sendIntent.putExtra(Intent.EXTRA_TEXT, "Присоединяйся к чату: " + "https://halch.at/c/" + res.getString("code"));
                            sendIntent.setType("text/plain");
                            startActivity(Intent.createChooser(sendIntent, "Пригласить в HalChat"));
                        } else {
                            Log.e(TAG, "CreateShortLink Error: " + res.getLong("errorCode") + ";" + res.getString("error"));
                            Toast.makeText(this, res.getString("error"), Toast.LENGTH_SHORT).show();
                        }
                    } catch (JSONException e) {
                        Log.e(TAG, "createShortLink", e);
                    }
                });
            } catch (JSONException e) {
                throw new RuntimeException(e);
            }
        });

        findUsersEdit.setOnEditorActionListener((v, actionId, event) -> {
            if (actionId == EditorInfo.IME_ACTION_DONE || actionId == EditorInfo.IME_ACTION_SEARCH) {
                String finalInput = v.getText().toString();

                findUsers(finalInput);

                return true;
            }
            return false;
        });
    }

    private void findUsers(String nickname) {
        try {
            JSONObject j=new JSONObject();
            j.put("limit",100);
            j.put("query",nickname);
            j.put("lastId",-1);

            hc.hnapi.apiReq("searchPeople",j).thenAccept(data->{
                try {
                    if(data.getLong("errorCode")==0) {
                        JSONArray results=data.getJSONArray("results");
                        if(results.length()==0) {
                            inviteUsersListAdapter.removeAll();
                            return;
                        }

                        List<SearchUser> users=new ArrayList<>();
                        for(int i=0;i<results.length();i++) {
                            JSONObject userObject=results.getJSONObject(i);
                            HNUser user=new HNUser(-1,userObject.getLong("id"),userObject.getString("nickname"),userObject.getString("icon"),false,"");

                            users.add(new SearchUser(user,null));
                        }

                        runOnUiThread(()->inviteUsersListAdapter.updateUsers(users));
                    }
                } catch (JSONException e) {
                    Log.e(TAG,"SearchPeople",e);
                }
            });
        } catch (JSONException e) {
            Log.e(TAG,"findUsers",e);
        }
    }

    public void inviteUser(HNUser user) {
        try {
            JSONObject postData=new JSONObject();
            postData.put("chatId",chatId);
            postData.put("userId",user.id);

            hc.hcapi.apiReq("inviteUser",postData).thenAccept(data->{
                try {
                    if(data.getLong("errorCode")==0) {
                        runOnUiThread(()->Toast.makeText(this,"Приглашение отправлено",Toast.LENGTH_SHORT).show());
                    } else {
                        runOnUiThread(()->Toast.makeText(this,"Не удалось пригласить",Toast.LENGTH_SHORT).show());
                    }
                } catch (JSONException e) {
                    Log.e(TAG,"inviteUser",e);
                }
            });
        } catch (JSONException e) {
            Log.e(TAG,"inviteU",e);
        }
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