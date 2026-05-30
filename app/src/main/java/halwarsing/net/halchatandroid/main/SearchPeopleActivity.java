package halwarsing.net.halchatandroid.main;

import android.content.Intent;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.LinearLayout;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.recyclerview.widget.DividerItemDecoration;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.File;
import java.net.HttpURLConnection;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.Timer;
import java.util.TimerTask;

import javax.net.ssl.HttpsURLConnection;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.type.HNUser;
import halwarsing.net.halchatandroid.type.SearchUser;

public class SearchPeopleActivity extends AppCompatActivity {
    private Timer timer = new Timer();
    private final long DELAY = 1000;
    private static final String TAG="SPA";
    private HalChat hc;
    private String lastQuery="";
    private long lastId=-1;
    private SearchUsersListAdapter searchUsersListAdapter=null;
    private RecyclerView userRecyclerView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        //Variables
        EditText editTextSearch;

        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_search_people);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars() | WindowInsetsCompat.Type.displayCutout());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });



        //Default
        ImageButton buttonBack=findViewById(R.id.toolbar_back);
        buttonBack.setOnClickListener((View v)->{
            if(!isTaskRoot()) {
                finish();
            } else {
                Intent intent=new Intent(SearchPeopleActivity.this,MainActivity.class);
                intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP|Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivity(intent);
                finish();
            }
        });

        //Init variables
        HalChatApp app=(HalChatApp)getApplicationContext();
        hc=app.getHalChat();
        editTextSearch=findViewById(R.id.edit_text_search);

        //Adapter
        userRecyclerView=findViewById(R.id.userRecyclerView);
        userRecyclerView.setLayoutManager(new LinearLayoutManager(this));
        DividerItemDecoration dividerItemDecoration=new DividerItemDecoration(userRecyclerView.getContext(), LinearLayout.VERTICAL);
        userRecyclerView.addItemDecoration(dividerItemDecoration);
        searchUsersListAdapter=new SearchUsersListAdapter(new ArrayList<>(),this);
        userRecyclerView.setAdapter(searchUsersListAdapter);

        //Set Events
        editTextSearch.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {

            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                if(timer != null)
                    timer.cancel();
            }

            @Override
            public void afterTextChanged(Editable s) {
                timer = new Timer();
                timer.schedule(new TimerTask() {
                    @Override
                    public void run() {
                        String text=s.toString();
                        searchPeople(text);
                    }

                }, DELAY);
            }
        });
    }

    private void searchPeople(String text) {
        TaskExecutorManager.getInstance().submitAPI("searchPeople",()->{
            if(text.isEmpty()) {
                lastQuery=text;
                lastId=-1;
                runOnUiThread(()->searchUsersListAdapter.removeAll());
                return null;
            }
            if(lastQuery.equals(text)) {
                if(lastId==-2){return null;}
            } else {
                lastId=-1;
                runOnUiThread(()->searchUsersListAdapter.removeAll());
            }
            lastQuery=text;
            StringBuilder postData=new StringBuilder();
            postData.append("query=").append(URLEncoder.encode(text, "UTF-8"));
            postData.append("&lastId=").append(URLEncoder.encode(String.valueOf(lastId), "UTF-8"));

            HttpsURLConnection connection= HalChatFunctionsLib.postHTTPSRequest("https://halwarsing.net/api/api?req=searchPeople&limit=100",postData.toString(),hc.codeUser);
            connection.connect();
            if(connection.getResponseCode()!= HttpURLConnection.HTTP_OK) {
                Log.e(TAG, "Server returned HTTP " + connection.getResponseCode() + " " + connection.getResponseMessage());
                return null;
            }

            String jsonString = HalChatFunctionsLib.getStringFromConnection(connection);
            JSONObject jsonObject = new JSONObject(jsonString);
            if(jsonObject.getInt("errorCode")==0) {
                Log.e(TAG,"Successful search people");
                Log.e(TAG,"People: "+ jsonObject.getJSONArray("results"));
                JSONArray results=jsonObject.getJSONArray("results");
                for(int i=0;i<results.length();i++) {
                    JSONObject userJson=results.getJSONObject(i);
                    HNUser hnUser=hc.hnUsers.loadUserWithoutThread(userJson.getLong("id"));
                    Log.e(TAG,"Start loading file hnUser: "+hnUser.nickname);
                    File iconFile=hc.hd.getFileById(hnUser.icon).get();
                    Log.e(TAG,"End loading file hnUser: "+hnUser.nickname);
                    SearchUser user=new SearchUser(hnUser, BitmapFactory.decodeFile(iconFile.getAbsolutePath()));
                    Log.e(TAG,"End loading hnUser: "+hnUser.nickname);
                    runOnUiThread(()->searchUsersListAdapter.addNewUser(user));
                }
            } else {
                Log.e(TAG,"Error search people: "+jsonObject.getInt("errorCode")+";"+jsonObject.getString("error"));
            }
            Log.e(TAG,"END SEARCH PEOPLE");
            return null;
        });
    }

    public void openUser(HNUser user) {
        Intent intent=new Intent(SearchPeopleActivity.this,UserActivity.class);
        intent.putExtra("uid",user.id);
        //intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP|Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
        overridePendingTransition(R.anim.slide_in_down,R.anim.stay);
    }
}