package halwarsing.net.halchatandroid.main;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

import androidx.annotation.Nullable;
import androidx.core.app.NotificationCompat;

import halwarsing.net.halchatandroid.R;

public class SyncService extends Service {
    private static final String TAG="SyncService";

    @Override
    public void onCreate() {
        super.onCreate();
        //startForeground(1,createNotification());
        //syncMessages();
        stopSelf();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return START_NOT_STICKY;
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private void syncMessages() {

        //Log.e(TAG,"Check new messages...");
    }

    private Notification createNotification() {
        NotificationChannel channel = new NotificationChannel(
                "halchat_sync", "HalChat Синхронизация", NotificationManager.IMPORTANCE_LOW);
        getSystemService(NotificationManager.class).createNotificationChannel(channel);

        return new NotificationCompat.Builder(this, "halchat_sync")
                .setContentTitle("HalChat")
                .setContentText("Проверка сообщений")
                .setSmallIcon(R.drawable.ic_notification)
                .build();
    }
}
