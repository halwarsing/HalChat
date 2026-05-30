package halwarsing.net.halchatandroid.main;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Point;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.View;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.cardview.widget.CardView;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import com.google.android.material.slider.Slider;

import org.json.JSONObject;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;

import javax.net.ssl.HttpsURLConnection;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.type.HCChat;
import halwarsing.net.halchatandroid.views.ZoomableImageView;

public class ChangeAvatarActivity extends AppCompatActivity {
    private HCChat chat;
    private long chatId;
    private HalChat hc;
    private Button saveBtn,selectBtn;
    private Slider sliderZoom;
    private ZoomableImageView icon;
    private CardView iconDiv;
    private Bitmap origAvatar,curAvatar;

    private static final String TAG="CAA";
    private static final int SELECT_PHOTO=1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_change_avatar);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars() | WindowInsetsCompat.Type.displayCutout());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });


        ImageButton buttonBack;

        HalChatApp app=(HalChatApp)getApplicationContext();
        hc=app.getHalChat();

        //Variables
        chatId=getIntent().getLongExtra("uid",-1);
        buttonBack=findViewById(R.id.toolbar_back);
        chat=hc.chatGroupChats.getChatInfo(chatId);
        saveBtn=findViewById(R.id.buttonSaveAvatar);
        selectBtn=findViewById(R.id.buttonSelectImage);
        sliderZoom=findViewById(R.id.sliderZoomAvatar);
        icon=findViewById(R.id.icon);
        iconDiv=findViewById(R.id.iconDiv);

        //Init
        Display display = getWindowManager().getDefaultDisplay();
        Point size = new Point();
        display.getSize(size);
        int windowWidth = size.x;
        int windowHeight = size.y;
        iconDiv.getLayoutParams().width=windowWidth-100;
        iconDiv.getLayoutParams().height=windowWidth-100;
        iconDiv.setRadius(windowWidth-100);

        hc.hd.getFileById(chat.icon).thenAccept(file->{
            origAvatar=BitmapFactory.decodeFile(file.getAbsolutePath());
            curAvatar=origAvatar;
            icon.setImageBitmap(origAvatar);
        });


        icon.getLayoutParams().width=windowWidth-100;
        icon.getLayoutParams().height=windowWidth-100;

        //ActivityResult
        ActivityResultLauncher<Intent> photoPickerLauncher = registerForActivityResult(
                new ActivityResultContracts.StartActivityForResult(),
                result -> {
                    if (result.getResultCode() == Activity.RESULT_OK && result.getData() != null) {
                        Uri imageUri=result.getData().getData();
                        try {
                            //Загрузка изображения
                            InputStream inputStream=getContentResolver().openInputStream(imageUri);
                            origAvatar=BitmapFactory.decodeStream(inputStream);
                            icon.setImageBitmap(origAvatar);
                            //curAvatar=origAvatar;
                            //icon.setImageBitmap(origAvatar);
                            icon.post(icon::resetZoom);
                            sliderZoom.setValue(1.0f);
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    }
                }
        );

        //On click
        buttonBack.setOnClickListener((View v)->{
            finish();
            overridePendingTransition(R.anim.stay,R.anim.slide_out_up);
        });

        selectBtn.setOnClickListener((View v)->{
            Intent photoPickerIntent = new Intent(Intent.ACTION_PICK);
            photoPickerIntent.setType("image/*");
            photoPickerLauncher.launch(photoPickerIntent);
        });

        saveBtn.setOnClickListener((View v)->{
            curAvatar=icon.getTransformedBitmap();
            File file=new File(hc.hd.directory,"chatIcon"+ chat.chatUID +".png");
            String filePath=file.getAbsolutePath();
            try(FileOutputStream out=new FileOutputStream(filePath)) {
                curAvatar.compress(Bitmap.CompressFormat.PNG, 100, out);
                TaskExecutorManager.getInstance().submitUpload("changeIcon:chatId:"+chat.chatUID,()->{
                    String id=hc.hd.uploadHDFileWithoutThread("system",1,1,file,"image/png",true);
                    if(id!=null) {
                        try {
                            HttpsURLConnection connection = HalChatFunctionsLib.getHTTPSRequest("https://halchat.halwarsing.net/api?req=changeIcon&chatId="+chat.chatUID+"&id="+id, hc.codeUser);
                            connection.connect();
                            if (connection.getResponseCode() != HttpURLConnection.HTTP_OK) {
                                Log.e(TAG, "Server returned HTTP " + connection.getResponseCode() + " " + connection.getResponseMessage());
                                return null;
                            }

                            String jsonString = HalChatFunctionsLib.getStringFromConnection(connection);
                            JSONObject jsonObject = new JSONObject(jsonString);
                            if (jsonObject.getInt("errorCode") == 0) {
                                chat.icon=id;
                                hc.chatGroupChats.updateIcon(id,chat.uid);
                                runOnUiThread(() -> Toast.makeText(ChangeAvatarActivity.this,"Аватар обновлён",Toast.LENGTH_SHORT).show());
                            } else {
                                Log.e(TAG,"Error change icon: "+jsonString);
                            }
                        } catch (Exception e) {
                            Log.e(TAG, "Failed to fetch HalChat API", e);
                        }
                    }
                    return null;
                });
            } catch (Exception e) {
                Log.e(TAG,"Error save file");
            }
        });

        //range
        sliderZoom.addOnChangeListener(new Slider.OnChangeListener() {
            @Override
            public void onValueChange(@NonNull Slider slider, float value, boolean fromUser) {
                //curAvatar=zoomAndOffset(origAvatar,value,0,0);
                //icon.setImageBitmap(curAvatar);
                icon.setZoom(value);
            }
        });
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        /*if(requestCode==SELECT_PHOTO&&resultCode==RESULT_OK&&data!=null&&data.getData()!=null) {
            Uri imageUri=data.getData();
            try {
                //Загрузка изображения
                InputStream inputStream=getContentResolver().openInputStream(imageUri);
                origAvatar=BitmapFactory.decodeStream(inputStream);
                icon.setImageBitmap(origAvatar);
                //curAvatar=origAvatar;
                //icon.setImageBitmap(origAvatar);
                icon.post(icon::resetZoom);
                sliderZoom.setValue(1.0f);
            }catch (IOException e) {
                e.printStackTrace();
            }
        }*/
    }

    private Bitmap zoomAndOffset(Bitmap sourceBitmap,float zoom,float offsetX,float offsetY) {
        if(sourceBitmap==null||zoom<=0||zoom>1) {
            Log.e(TAG,"ERROR ZOOM");
            return sourceBitmap;
        }

        int width=sourceBitmap.getWidth();
        int height=sourceBitmap.getHeight();


        Bitmap result=Bitmap.createBitmap((int)(width*zoom),(int)(height*zoom),sourceBitmap.getConfig());

        float translateX=offsetX*width*zoom;
        float translateY=offsetY*height*zoom;

        Matrix matrix=new Matrix();
        matrix.postScale(zoom,zoom);
        matrix.postTranslate(-translateX,-translateY);

        Canvas canvas=new Canvas(result);
        canvas.drawBitmap(sourceBitmap,matrix,null);
        return result;
    }
}