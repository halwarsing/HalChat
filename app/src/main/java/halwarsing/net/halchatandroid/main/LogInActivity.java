package halwarsing.net.halchatandroid.main;

import android.content.Intent;
import android.database.sqlite.SQLiteDatabase;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.webkit.CookieManager;
import android.webkit.JavascriptInterface;
import android.webkit.WebResourceError;
import android.webkit.WebResourceRequest;
import android.webkit.WebResourceResponse;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;

import androidx.activity.EdgeToEdge;
import androidx.activity.OnBackPressedCallback;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.type.HNUser;

//Страница авторизации в HalNet
public class LogInActivity extends AppCompatActivity {
    private WebView webView;
    private static final String TAG="HCALOGIN";
    private View offlineStub;
    private ConnectivityObserver observer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_log_in);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars() | WindowInsetsCompat.Type.displayCutout());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });

        offlineStub=findViewById(R.id.offlineStub);
        webView=findViewById(R.id.webViewLogIn);

        observer=new ConnectivityObserver(this,this::updateUI);

        CookieManager.getInstance().removeAllCookies(null);
        WebSettings webSettings=webView.getSettings();
        webSettings.setJavaScriptEnabled(true);
        webSettings.setDatabaseEnabled(true);
        webSettings.setDomStorageEnabled(true);
        webSettings.setJavaScriptCanOpenWindowsAutomatically(true);
        webSettings.setLoadsImagesAutomatically(true);
        webSettings.setCacheMode(WebSettings.LOAD_NO_CACHE);
        webSettings.setAllowFileAccess(true);
        webSettings.setBuiltInZoomControls(true);
        webSettings.setDisplayZoomControls(false);
        webSettings.setMediaPlaybackRequiresUserGesture(false);
        webSettings.setLoadWithOverviewMode(true);
        webSettings.setUseWideViewPort(true);

        CookieManager.getInstance().setAcceptThirdPartyCookies(webView,true);
        CookieManager.getInstance().setCookie("https://halwarsing.net/","agreementCookie=1; max-age=189345600000; secure; path=/; domain=halwarsing.net");

        webView.addJavascriptInterface(new WebViewClient() {
            @JavascriptInterface
            public void login(String uid,String id,String nickname,String icon) {
                //Запись данных после успешной авторизации
                Log.d(TAG,"LogIn successful");
                HalChatDatabaseHelper dbHelper=new HalChatDatabaseHelper(LogInActivity.this);
                SQLiteDatabase db=dbHelper.getWritableDatabase();
                db.execSQL("INSERT INTO `sessions` (`id`, `fromId`) VALUES (?, ?)",new String[]{uid, id});
                HalNetUsers halNetUsers=new HalNetUsers(null,db,new HalDrive(LogInActivity.this,db,uid));
                //db.execSQL("INSERT INTO `users` (`id`, `nickname`, `icon`) VALUES (?, ?, ?)",new String[]{id,nickname,icon});
                HNUser newuser=new HNUser(-1,Long.parseLong(id),nickname,icon,false,"");
                halNetUsers.addUser(newuser);
                HalChatApp app=(HalChatApp)getApplicationContext();
                HalChat hc=app.getHalChat();
                hc.init(LogInActivity.this);
                startActivity(new Intent(LogInActivity.this,MainActivity.class));
            }
        },"HCALogIn");

        webView.setWebViewClient(new AuthWebClient("https://halwarsing.net/successLogIn"));



        getOnBackPressedDispatcher().addCallback(this,
                new OnBackPressedCallback(true) {
                    @Override public void handleOnBackPressed() {
                        if (webView.canGoBack()) {
                            webView.goBack();
                        } else {
                            setEnabled(false);
                            onBackPressed();
                        }
                    }
                });

        loadStartUrl();
    }

    private void loadStartUrl() {
        webView.loadUrl("https://halwarsing.net/auth?successUrl=https://halwarsing.net/successLogIn");
    }

    private void showOffline() {
        offlineStub.setVisibility(View.VISIBLE);
        webView.setVisibility(View.GONE);
    }

    private void hideOffline() {
        offlineStub.setVisibility(View.GONE);
        webView.setVisibility(View.VISIBLE);
        loadStartUrl();
    }

    protected void updateUI(boolean connected) {
        runOnUiThread(()->{
            if (connected) {
                hideOffline();
                if (webView.getUrl() == null)loadStartUrl();
            } else {
                showOffline();
            }
        });
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
    }

    @Override protected void onResume() { super.onResume(); observer.start(); }
    @Override protected void onPause()  { observer.stop();  super.onPause(); }

    /* replace url*/
    public class AuthWebClient extends WebViewClient {

        private final String successUrlParam;
        private final String AUTH_HOST = "halwarsing.net";

        public AuthWebClient(String successUrlParam) {
            this.successUrlParam = successUrlParam;
        }

        @Override
        public boolean shouldOverrideUrlLoading(WebView view,
                                                WebResourceRequest request) {
            return handle(request.getUrl(), view);
        }

        private boolean handle(Uri uri, WebView view) {
            if (!AUTH_HOST.equals(uri.getHost())) return false;
            Uri newUri = uri.buildUpon()
                    .appendQueryParameter("successUrl", successUrlParam)
                    .build();
            view.loadUrl(newUri.toString());
            return true;
        }

        @Override
        public void onReceivedHttpError(WebView view, WebResourceRequest request, WebResourceResponse errorResponse) {
            super.onReceivedHttpError(view, request, errorResponse);
            showOffline();
        }

        @Override
        public void onReceivedError(WebView view, WebResourceRequest request, WebResourceError error) {
            super.onReceivedError(view, request, error);

            if (request.isForMainFrame()) {
                Log.e(TAG, error.getErrorCode() + ": " + error.getDescription()
                        + " url=" + request.getUrl());
                showOffline();
            } else {
                Log.w(TAG, "Subresource blocked: " + request.getUrl()
                        + " (" + error.getErrorCode() + ": " + error.getDescription() + ")");
            }
        }
    }
}