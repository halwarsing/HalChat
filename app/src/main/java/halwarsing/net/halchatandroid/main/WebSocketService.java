package halwarsing.net.halchatandroid.main;

import android.app.Service;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.Network;
import android.os.IBinder;
import android.os.Handler;
import android.os.Looper;
import android.os.PowerManager;
import android.util.Log;

import java.util.Arrays;
import java.util.Objects;
import java.util.concurrent.TimeUnit;

import halwarsing.net.halchatandroid.type.HalChatWSI;
import okhttp3.*;

public class WebSocketService extends Service {
    private WebSocket wsMain;
    private OkHttpClient client;
    private ConnectivityManager connectivityManager;
    private ConnectivityManager.NetworkCallback networkCallback;
    private Network currentNetwork;
    private final Handler reconnectHandler=new Handler(Looper.getMainLooper());
    private Runnable scheduledReconnect;
    private int connectionGeneration;
    private boolean destroyed;

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
        connectivityManager=(ConnectivityManager)getSystemService(CONNECTIVITY_SERVICE);
        registerNetworkCallback();
        switchNetwork(connectivityManager.getActiveNetwork());
    }

    private OkHttpClient buildClient(Network network) {
        OkHttpClient.Builder builder=new OkHttpClient.Builder()
                .retryOnConnectionFailure(true)
                .connectTimeout(10,TimeUnit.SECONDS)
                .pingInterval(30,TimeUnit.SECONDS);
        if(network!=null) {
            builder.socketFactory(network.getSocketFactory());
            builder.dns(hostname->Arrays.asList(network.getAllByName(hostname)));
        }
        return builder.build();
    }

    private void registerNetworkCallback() {
        networkCallback=new ConnectivityManager.NetworkCallback() {
            @Override
            public void onAvailable(Network network) {
                reconnectHandler.post(()->{
                    if(!destroyed&&!Objects.equals(currentNetwork,network))switchNetwork(network);
                });
            }

            @Override
            public void onLost(Network network) {
                reconnectHandler.post(()->{
                    if(destroyed||!Objects.equals(currentNetwork,network))return;
                    Network activeNetwork=connectivityManager.getActiveNetwork();
                    switchNetwork(activeNetwork);
                });
            }
        };
        connectivityManager.registerDefaultNetworkCallback(networkCallback);
    }

    private void switchNetwork(Network network) {
        if(destroyed)return;
        currentNetwork=network;
        cancelReconnect();
        cancelSocket();
        shutdownClient();
        client=buildClient(network);
        if(network==null) {
            Log.w(TAG,"No active network for WebSocket");
            return;
        }
        connectWebSocket();
    }

    private void connectWebSocket() {
        if(destroyed||client==null||currentNetwork==null)return;
        cancelReconnect();
        cancelSocket();
        client.connectionPool().evictAll();

        int generation=++connectionGeneration;
        Log.e(TAG,"Start connection on network "+currentNetwork);
        Request request = new Request.Builder()
                .url(WEBSOCKET_URL).build();
        wsMain=client.newWebSocket(
                request,
                new HalChatWS(()->reconWS(generation),hc.codeUser,hc)
        );
    }

    private void reconWS(int failedGeneration) {
        reconnectHandler.post(()->{
            if(destroyed||failedGeneration!=connectionGeneration||scheduledReconnect!=null)return;
            scheduledReconnect=()->{
                scheduledReconnect=null;
                if(!destroyed&&failedGeneration==connectionGeneration)connectWebSocket();
            };
            reconnectHandler.postDelayed(scheduledReconnect,5000);
        });
    }

    private void cancelReconnect() {
        if(scheduledReconnect!=null) {
            reconnectHandler.removeCallbacks(scheduledReconnect);
            scheduledReconnect=null;
        }
    }

    private void cancelSocket() {
        connectionGeneration++;
        if(wsMain!=null) {
            wsMain.cancel();
            wsMain=null;
        }
    }

    private void shutdownClient() {
        if(client==null)return;
        client.connectionPool().evictAll();
        client.dispatcher().executorService().shutdown();
        client=null;
    }

    private void acquireWakeLock() {
        PowerManager powerManager = (PowerManager) getSystemService(POWER_SERVICE);
        wakeLock = powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "HalChat:WakeLockTag");
        wakeLock.acquire(24*60*60*1000L);//1 день
    }

    @Override
    public void onDestroy() {
        destroyed=true;
        cancelReconnect();
        reconnectHandler.removeCallbacksAndMessages(null);
        if(connectivityManager!=null&&networkCallback!=null) {
            try {
                connectivityManager.unregisterNetworkCallback(networkCallback);
            } catch(Exception ignored) {}
        }
        cancelSocket();
        shutdownClient();
        if (wakeLock != null && wakeLock.isHeld()) {
            wakeLock.release();
        }
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}
