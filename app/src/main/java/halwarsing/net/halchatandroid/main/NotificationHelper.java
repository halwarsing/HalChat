package halwarsing.net.halchatandroid.main;

import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.AudioAttributes;
import android.net.Uri;

import androidx.core.app.NotificationCompat;
import androidx.core.app.Person;
import androidx.core.graphics.drawable.IconCompat;

import java.util.HashMap;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.type.HCChat;
import halwarsing.net.halchatandroid.type.HCMessage;
import halwarsing.net.halchatandroid.type.HNUser;

public class NotificationHelper {
    private static final String CHANNEL_ID = "halchat_channel";
    private static final String CHANNEL_NAME = "HalChat Уведомления";
    private static final String GROUP_KEY_CHAT = "halchat_group";
    private static final String TAG="NH";

    private static final HashMap<Long, Integer> unreadMessages = new HashMap<>();
    private static final HashMap<Long, HashMap<Long, NotificationCompat.MessagingStyle>> chatMessages = new HashMap<>();

    public static void showNotification(Context context, HCMessage msg, HCChat chat, HNUser user, String iconChat) {
        NotificationManager manager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);

        NotificationChannel channel = new NotificationChannel(
                CHANNEL_ID, CHANNEL_NAME, NotificationManager.IMPORTANCE_HIGH
        );

        //Sound
        Uri soundUri=Uri.parse("android.resource://"+context.getPackageName()+"/"+R.raw.new_notification);

        AudioAttributes audioAttributes=new AudioAttributes.Builder()
                .setUsage(AudioAttributes.USAGE_NOTIFICATION)
                .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                .build();

        channel.setSound(soundUri,audioAttributes);

        manager.createNotificationChannel(channel);

        int messageCount = unreadMessages.getOrDefault(msg.chatId, 0) + 1;
        unreadMessages.put(msg.chatId, messageCount);

        Intent intent = new Intent(context, ChatActivity.class);
        intent.putExtra("uid", msg.chatId);
        PendingIntent pendingIntent = PendingIntent.getActivity(
                context, (int) msg.chatId, intent, PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE
        );

        Bitmap avatar=BitmapFactory.decodeFile(user.fullPathIcon);

        Person sender = new Person.Builder()
                .setName(user.nickname)
                .setIcon(IconCompat.createWithBitmap(avatar))
                .build();


        HashMap<Long, NotificationCompat.MessagingStyle> messages=chatMessages.getOrDefault(msg.chatId,new HashMap<>());

        NotificationCompat.MessagingStyle messagingStyle = messages.getOrDefault(msg.msgId,new NotificationCompat.MessagingStyle(sender)
                        .setConversationTitle(user.nickname));

        messagingStyle.addMessage(msg.decryptedMessage, msg.time, sender);

        messages.put(msg.msgId, messagingStyle);
        chatMessages.put(msg.chatId,messages);

        NotificationCompat.Builder chatNotification = new NotificationCompat.Builder(context, CHANNEL_ID)
                .setSmallIcon(R.drawable.zero)
                .setStyle(messagingStyle)
                .setAutoCancel(true)
                .setContentIntent(pendingIntent)
                .setNumber(messageCount)
                .setGroup(GROUP_KEY_CHAT+msg.chatId)
                .setAutoCancel(true);

        Bitmap bitmapChat=BitmapFactory.decodeFile(iconChat);

        NotificationCompat.Builder groupSummary = new NotificationCompat.Builder(context, CHANNEL_ID)
                .setSmallIcon(IconCompat.createWithBitmap(bitmapChat))
                .setLargeIcon(bitmapChat)
                .setContentTitle(chat.name)
                .setStyle(new NotificationCompat.InboxStyle()
                        .setSummaryText(chat.name+" (" + messageCount+")"))
                .setGroup(GROUP_KEY_CHAT + msg.chatId)
                .setGroupSummary(true)
                .setAutoCancel(true);

        manager.notify((int)msg.msgId,chatNotification.build());
        manager.notify((int)msg.chatId,groupSummary.build());
    }

    public static void showChatNotification(Context context, HCMessage msg, HCChat chat, String iconChat) {
        NotificationManager manager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);

        NotificationChannel channel = new NotificationChannel(
                CHANNEL_ID, CHANNEL_NAME, NotificationManager.IMPORTANCE_HIGH
        );

        //Sound
        Uri soundUri=Uri.parse("android.resource://"+context.getPackageName()+"/"+R.raw.new_notification);

        AudioAttributes audioAttributes=new AudioAttributes.Builder()
                .setUsage(AudioAttributes.USAGE_NOTIFICATION)
                .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                .build();

        channel.setSound(soundUri,audioAttributes);

        manager.createNotificationChannel(channel);

        int messageCount = unreadMessages.getOrDefault(msg.chatId, 0) + 1;
        unreadMessages.put(msg.chatId, messageCount);

        Intent intent = new Intent(context, ChatActivity.class);
        intent.putExtra("uid", msg.chatId);
        PendingIntent pendingIntent = PendingIntent.getActivity(
                context, (int) msg.chatId, intent, PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE
        );

        Bitmap avatar=BitmapFactory.decodeFile(iconChat);

        Person sender = new Person.Builder()
                .setName(chat.name)
                .setIcon(IconCompat.createWithBitmap(avatar))
                .build();

        HashMap<Long, NotificationCompat.MessagingStyle> messages=chatMessages.getOrDefault(msg.chatId,new HashMap<>());

        NotificationCompat.MessagingStyle messagingStyle = messages.getOrDefault(msg.msgId,new NotificationCompat.MessagingStyle(sender)
                .setConversationTitle(chat.name));

        messagingStyle.addMessage(msg.decryptedMessage, msg.time, sender);

        messages.put(msg.msgId, messagingStyle);
        chatMessages.put(msg.chatId,messages);

        NotificationCompat.Builder chatNotification = new NotificationCompat.Builder(context, CHANNEL_ID)
                .setSmallIcon(R.drawable.zero)
                .setStyle(messagingStyle)
                .setAutoCancel(true)
                .setContentIntent(pendingIntent)
                .setNumber(messageCount)
                .setGroup(GROUP_KEY_CHAT+msg.chatId)
                .setAutoCancel(true);

        Bitmap bitmapChat=BitmapFactory.decodeFile(iconChat);

        NotificationCompat.Builder groupSummary = new NotificationCompat.Builder(context, CHANNEL_ID)
                .setSmallIcon(IconCompat.createWithBitmap(bitmapChat))
                .setLargeIcon(bitmapChat)
                .setContentTitle(chat.name)
                .setStyle(new NotificationCompat.InboxStyle()
                        .setSummaryText(chat.name+" (" + messageCount+")"))
                .setGroup(GROUP_KEY_CHAT + msg.chatId)
                .setGroupSummary(true)
                .setAutoCancel(true);

        manager.notify((int)msg.msgId,chatNotification.build());
        manager.notify((int)msg.chatId,groupSummary.build());
    }

    // Сброс счётчика при открытии чата
    public static void clearChatNotifications(Context context, long chatId) {
        unreadMessages.remove(chatId);
        chatMessages.remove(chatId);
        NotificationManager manager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        manager.cancel((int) chatId);
    }
}