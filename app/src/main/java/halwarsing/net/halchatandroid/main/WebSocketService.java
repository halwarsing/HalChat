package halwarsing.net.halchatandroid.main;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.PowerManager;
import android.util.Log;

import java.util.concurrent.TimeUnit;

import halwarsing.net.halchatandroid.type.HalChatWSI;
import okhttp3.*;

public class WebSocketService extends Service {
    private WebSocket wsMain;

    private PowerManager.WakeLock wakeLock;
    private static final String WEBSOCKET_URL = "wss://halchat.halwarsing.net/ws/";
    private static final String TAG="WSS";
    private HalChat hc;

    @Override
    public void onCreate() {
        super.onCreate();
        HalChatApp app=(HalChatApp)getApplicationContext();
        hc=app.getHalChat();
        acquireWakeLock();
        connectWebSocket();
    }


    private void connectWebSocket() {
        Log.e(TAG,"Start connections");
        OkHttpClient client = new OkHttpClient.Builder()
                .retryOnConnectionFailure(true)
                .pingInterval(30, TimeUnit.SECONDS)
                .build();
        Request request = new Request.Builder()
                .url(WEBSOCKET_URL).build();
        wsMain = client.newWebSocket(request, new HalChatWS(this::reconWS,hc.codeUser,hc));
    }

    private void reconWS() {
        new android.os.Handler(getMainLooper()).postDelayed(this::connectWebSocket, 5000);
    }

    private void acquireWakeLock() {
        PowerManager powerManager = (PowerManager) getSystemService(POWER_SERVICE);
        wakeLock = powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "HalChat:WakeLockTag");
        wakeLock.acquire(24*60*60*1000L);//1 день
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (wakeLock != null && wakeLock.isHeld()) {
            wakeLock.release();
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}