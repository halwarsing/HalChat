package halwarsing.net.halchatandroid.main;


import android.content.Context;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.os.Build;

public class ConnectivityObserver {

    public interface Callback { void onStateChanged(boolean connected); }

    private final ConnectivityManager cm;
    private final ConnectivityManager.NetworkCallback cb;
    private final Callback  listener;

    public ConnectivityObserver(Context ctx, Callback listener) {
        this.listener = listener;
        cm = (ConnectivityManager) ctx.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo activeNetworkInfo = cm.getActiveNetworkInfo();
        if (activeNetworkInfo == null || !activeNetworkInfo.isConnected()) {
            listener.onStateChanged(false);
        }

        cb = new ConnectivityManager.NetworkCallback() {
            @Override public void onAvailable(Network network)   { listener.onStateChanged(true); }
            @Override public void onLost(Network network)        { listener.onStateChanged(false); }
        };
    }

    public void start() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            cm.registerDefaultNetworkCallback(cb);                       // API ≥ 24 :contentReference[oaicite:0]{index=0}
        }
    }

    public void stop()  {
        try { cm.unregisterNetworkCallback(cb); } catch (Exception ignored) {}
    }

    /** Быстрый синхронный чек — пригодится для Retry-кнопки */
    public boolean isConnected() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            Network nw = cm.getActiveNetwork();
            if (nw == null) return false;
            NetworkCapabilities caps = cm.getNetworkCapabilities(nw);
            return caps != null &&
                    caps.hasCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET) &&
                    caps.hasCapability(NetworkCapabilities.NET_CAPABILITY_VALIDATED);
        } else {
            NetworkInfo ni = cm.getActiveNetworkInfo();
            return ni != null && ni.isConnectedOrConnecting();
        }
    }
}