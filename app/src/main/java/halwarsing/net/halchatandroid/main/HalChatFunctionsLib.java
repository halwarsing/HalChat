package halwarsing.net.halchatandroid.main;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.provider.OpenableColumns;
import android.text.Editable;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.style.ImageSpan;
import android.util.Log;
import android.webkit.MimeTypeMap;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.bumptech.glide.Glide;
import com.bumptech.glide.request.target.CustomTarget;
import com.bumptech.glide.request.transition.Transition;

import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.URL;
import java.net.URLConnection;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.net.ssl.HttpsURLConnection;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.type.Emoji;

//Библиотека всех основных независимых функций для оптимизации часто используемых функций
public class HalChatFunctionsLib {
    private static final String[] titles_seconds=new String[]{"секунду","секунды","секунд"};
    private static final String[] titles_minutes=new String[]{"минуту","минуты","минут"};
    private static final String[] titles_hours=new String[]{"час","часа","часов"};
    private static final String[] titles_days=new String[]{"день","дня","дней"};
    private static final String[] titles_weeks=new String[]{"неделю","недели","недель"};
    private static final String[] titles_months=new String[]{"месяц","месяца","месяцев"};
    private static final String TAG="HCFLib";

    private static final HashMap<String,Integer> emojiMap=createEmojiMap();

    private static HashMap<String,Integer> createEmojiMap() {
        HashMap<String, Integer> map = new HashMap<>();
        map.put("cringe", R.drawable.cringe);
        map.put("grinning",R.drawable.grinning);
        map.put("grinning_big_eyes",R.drawable.grinning_big_eyes);
        map.put("grinning_smiling_eyes",R.drawable.grinning_smiling_eyes);
        map.put("grinning_squinting",R.drawable.grinning_squinting);
        map.put("grinning_with_sweat",R.drawable.grinning_with_sweat);
        map.put("rolling_on_the_floor_laughing",R.drawable.rolling_on_the_floor_laughing);
        map.put("sad",R.drawable.sad);
        map.put("smile",R.drawable.smile);
        map.put("upside_down_smile",R.drawable.upside_down_smile);
        return map;
    }

    //Чтение текста из соединения
    public static String getStringFromConnection(HttpsURLConnection connection) throws IOException {
        InputStream inputStream=connection.getInputStream();
        BufferedReader reader=new BufferedReader(new InputStreamReader(inputStream));
        StringBuilder jsonBuilder=new StringBuilder();
        String line;
        while ((line=reader.readLine())!=null) {
            jsonBuilder.append(line);
        }

        reader.close();
        inputStream.close();

        return jsonBuilder.toString();
    }

    //Создание GET запроса
    public static HttpsURLConnection getHTTPSRequest(String urls, String codeUser) throws IOException {
        URL url=new URL(urls);
        HttpsURLConnection connection=(HttpsURLConnection)url.openConnection();
        connection.setDoOutput(true);
        connection.setDoInput(true);
        connection.setRequestMethod("GET");
        connection.setRequestProperty("Cookie","uid="+codeUser);
        return connection;
    }

    //Создание POST запроса
    public static HttpsURLConnection postHTTPSRequest(String urls,String postData,String codeUser) throws IOException {
        URL url=new URL(urls);
        HttpsURLConnection connection=(HttpsURLConnection)url.openConnection();
        connection.setDoOutput(true);
        connection.setDoInput(true);
        connection.setRequestMethod("POST");
        connection.setRequestProperty( "Content-Type", "application/x-www-form-urlencoded");
        connection.setRequestProperty("charset","utf-8");
        connection.setRequestProperty("Content-Length", Integer.toString(postData.length()));
        connection.setRequestProperty("Cookie","uid="+codeUser);
        connection.setUseCaches(false);
        DataOutputStream wr=new DataOutputStream(connection.getOutputStream());
        wr.write(postData.getBytes(StandardCharsets.UTF_8));
        return connection;
    }

    public static HttpsURLConnection postHTTPSRequestWithoutData(String urls, String codeUser) throws IOException {
        URL url=new URL(urls);
        HttpsURLConnection connection=(HttpsURLConnection)url.openConnection();
        connection.setDoOutput(true);
        connection.setDoInput(true);
        connection.setRequestMethod("POST");
        connection.setRequestProperty("Cookie","uid="+codeUser);
        connection.setUseCaches(false);
        return connection;
    }

    public static HttpsURLConnection postJSONHTTPSRequest(String urls, JSONObject postDataJSON, String codeUser) throws IOException {
        byte[] postDataBytes = postDataJSON.toString().getBytes(StandardCharsets.UTF_8);

        URL url = new URL(urls);
        HttpsURLConnection conn = (HttpsURLConnection) url.openConnection();
        conn.setDoOutput(true);
        conn.setRequestMethod("POST");
        conn.setRequestProperty("Content-Type", "application/json; charset=utf-8");
        conn.setRequestProperty("Accept", "application/json");
        conn.setRequestProperty("Cookie", "uid=" + codeUser);
        conn.setUseCaches(false);
        conn.setFixedLengthStreamingMode(postDataBytes.length);

        conn.connect();

        try (OutputStream os = conn.getOutputStream()) {
            os.write(postDataBytes);
            os.flush();
        }

        return conn;
    }

    private static String readStream(InputStream is) throws IOException {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        byte[] buffer = new byte[4096];
        int read;
        while ((read = is.read(buffer)) != -1) {
            baos.write(buffer, 0, read);
        }
        return baos.toString(StandardCharsets.UTF_8.name());
    }

    public static byte[] hexStringToByteArray(String s) {
        int len = s.length();
        byte[] data = new byte[(len+1) / 2];
        for (int i = 0; i < len; i += 2) {
            data[i / 2] = (byte) ((Character.digit(s.charAt(i), 16) << 4)+((i+1)!=len?Character.digit(s.charAt(i+1), 16):0));
        }
        return data;
    }

    protected static final char[] HEX_ARRAY = "0123456789abcdef".toCharArray();
    public static String bytesToHex(byte[] bytes) {
        char[] hexChars = new char[bytes.length * 2];
        for (int j = 0; j < bytes.length; j++) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = HEX_ARRAY[v >>> 4];
            hexChars[j * 2 + 1] = HEX_ARRAY[v & 0x0F];
        }
        return new String(hexChars);
    }

    public static void setClipboard(Context context, String label,String text) {
        android.content.ClipboardManager clipboard = (android.content.ClipboardManager) context.getSystemService(Context.CLIPBOARD_SERVICE);
        android.content.ClipData clip = android.content.ClipData.newPlainText(label, text);
        clipboard.setPrimaryClip(clip);
    }

    public static String getContentType(File file) {
        // Сначала пытаемся получить MIME-тип по расширению
        String extension = getFileExtension(file);
        if (extension != null) {
            String mimeType = MimeTypeMap.getSingleton().getMimeTypeFromExtension(extension.toLowerCase());
            if (mimeType != null) {
                return mimeType;
            }
        }
        // Если не получилось, пробуем через URLConnection
        String type=URLConnection.guessContentTypeFromName(file.getName());
        if(type==null) {
            return "*/*";
        }
        return type;
    }

    public static String getFileExtension(File file) {
        String name = file.getName();
        int lastIndex = name.lastIndexOf(".");
        if (lastIndex != -1) {
            return name.substring(lastIndex + 1);
        }
        return null;
    }

    public static String case_num(int number,String[] titles,int[] cases) {
        return titles[(number%100>4&&number%100<20)?2:cases[Math.min(number % 10, 5)]];
    }

    public static String getTimeFromSeconds(int seconds) {
        if (seconds<60) {
            return seconds+" "+case_num(seconds,titles_seconds,new int[]{2, 0, 1, 1, 1, 2})+" назад";
        } else if (seconds<3600) {
            int minutes=seconds/60;
            return minutes+" "+case_num(minutes,titles_minutes,new int[]{2,0,1,1,1,2})+" назад";
        } else if (seconds<86400) {
            int hours=seconds/3600;
            return hours+" "+case_num(hours,titles_hours,new int[]{2,0,1,1,1,2})+" назад";
        } else if (seconds<604800) {
            int days=seconds/86400;
            return days+" "+case_num(days,titles_days,new int[]{2,0,1,1,1,2})+" назад";
        } else if (seconds<2592000) {
            int weeks=seconds/604800;
            return weeks+" "+case_num(weeks,titles_weeks,new int[]{2,0,1,1,1,2})+" назад";
        } else if (seconds<31536000) {
            int months=seconds/2592000;
            return months+" "+case_num(months,titles_months,new int[]{2,0,1,1,1,2})+" назад";
        } else {
            int years=seconds/31536000;
            return years+" "+(years==1?"год":(years<5?"года":"лет"))+" назад";
        }
    }

    public static String getFileNameFromUri(ContentResolver contentResolver, Uri uri) {
        String fileName = null;
        Cursor cursor = null;
        try {
            String[] projection = {OpenableColumns.DISPLAY_NAME};
            cursor = contentResolver.query(uri, projection, null, null, null);
            if (cursor != null && cursor.moveToFirst()) {
                int nameIndex = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
                fileName = cursor.getString(nameIndex);
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return fileName;
    }

    public static void copyFile(File src, File dst) throws IOException {
        InputStream in = new FileInputStream(src);
        try {
            OutputStream out = new FileOutputStream(dst);
            try {
                byte[] buf = new byte[1024];
                int len;
                while ((len = in.read(buf)) > 0) {
                    out.write(buf, 0, len);
                }
            } finally {
                out.close();
            }
        } finally {
            in.close();
        }
    }

    public static Spannable replaceEmojis(Context context, TextView textView, HalChat hc, String text) {
        if(text.isEmpty()||text==null)return null;

        textView.setTag(text);

        final SpannableString spannableString=new SpannableString(text);

        Pattern pattern= Pattern.compile("\\[emoji-([0-9]+)\\]");
        Matcher matcher=pattern.matcher(text);

        final int size = (int) (textView.getTextSize() * 1.2);

        while(matcher.find()) {
            long emojiId=Long.parseLong(matcher.group(1));
            Emoji emoji=hc.EPSystem.getEmojiById(emojiId);

            if(emoji!=null) {
                /*Drawable drawable= ResourcesCompat.getDrawable(context.getResources(),emojiResId,null);

                drawable.setBounds(0,0,50,50);
                PaddedImageSpan imageSpan=new PaddedImageSpan(context,drawable,5,10);

                spannableString.setSpan(imageSpan,matcher.start(),matcher.end(),Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);*/

                Drawable placeholderDrawable = new ColorDrawable(Color.TRANSPARENT);
                placeholderDrawable.setBounds(0, 0, size, size);

                // Создание ImageSpan
                final PaddedImageSpan placeholderSpan = new PaddedImageSpan(context,placeholderDrawable,5,10);

                spannableString.setSpan(placeholderSpan, matcher.start(), matcher.end(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

                hc.hd.getFileById(emoji.image).thenAccept(fileIcon->{
                    new Handler(Looper.getMainLooper()).post(() -> {
                        if(!text.equals(textView.getTag()))return;
                        Glide.with(context)
                                .asDrawable()
                                .load(fileIcon)
                                .override(size, size)
                                .into(new CustomTarget<Drawable>() {
                                    @Override
                                    public void onResourceReady(@NonNull Drawable resource, @Nullable Transition<? super Drawable> transition) {
                                        if(!text.equals(textView.getTag()))return;
                                        resource.setBounds(0, 0, size, size);
                                        PaddedImageSpan readyImageSpan = new PaddedImageSpan(context, resource, 5, 10);

                                        int start = spannableString.getSpanStart(placeholderSpan);
                                        int end = spannableString.getSpanEnd(placeholderSpan);

                                        if (start != -1 && end != -1) {
                                            spannableString.removeSpan(placeholderSpan);
                                            spannableString.setSpan(readyImageSpan, start, end, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

                                            textView.setText(spannableString);
                                        }
                                    }

                                    @Override
                                    public void onLoadCleared(@Nullable Drawable placeholder) {

                                    }

                                    @Override
                                    public void onLoadFailed(@Nullable Drawable errorDrawable) {

                                    }
                                });
                    });
                });
            }


        }
        return spannableString;
    }
}

