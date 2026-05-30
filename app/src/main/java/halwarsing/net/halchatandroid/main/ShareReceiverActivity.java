package halwarsing.net.halchatandroid.main;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import java.util.ArrayList;

import halwarsing.net.halchatandroid.R;

public class ShareReceiverActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Intent intent = getIntent();
        String action = intent.getAction();
        String type = intent.getType();

        if (Intent.ACTION_SEND.equals(action) && type != null) {
            handleSend(intent, type);
        } else if (Intent.ACTION_SEND_MULTIPLE.equals(action) && type != null) {
            handleSendMultiple(intent, type);
        }

        finish(); // Закрыть активити после обработки
    }

    private void handleSend(Intent intent, String type) {
        if ("text/plain".equals(type)) {
            String sharedText = intent.getStringExtra(Intent.EXTRA_TEXT);
            openHalChatWithData(sharedText);
        } else {
            Uri fileUri = intent.getParcelableExtra(Intent.EXTRA_STREAM);
            openHalChatWithUri(fileUri);
        }
    }

    private void handleSendMultiple(Intent intent, String type) {
        ArrayList<Uri> uris = intent.getParcelableArrayListExtra(Intent.EXTRA_STREAM);
        openHalChatWithUris(uris);
    }

    private void openHalChatWithData(String data) {
        Intent chatIntent = new Intent(this, MainActivity.class);
        chatIntent.putExtra("shared_text", data);
        chatIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP);
        startActivity(chatIntent);
    }

    private void openHalChatWithUri(Uri uri) {
        Intent chatIntent = new Intent(this, MainActivity.class);
        chatIntent.putExtra("shared_uri", uri.toString());
        chatIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP);
        startActivity(chatIntent);
    }

    private void openHalChatWithUris(ArrayList<Uri> uris) {
        Intent chatIntent = new Intent(this, MainActivity.class);
        chatIntent.putParcelableArrayListExtra("shared_uris", uris);
        chatIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP);
        startActivity(chatIntent);
    }
}