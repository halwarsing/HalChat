package halwarsing.net.halchatandroid.main;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;

import androidx.annotation.NonNull;

import com.bumptech.glide.Glide;

import java.util.List;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.type.Emoji;
import halwarsing.net.halchatandroid.type.Pixel;
import halwarsing.net.halchatandroid.type.PixelsPack;

public class PixelAdapter extends BaseAdapter {
    private Context context;
    private List<Pixel> pixelsList;
    private OnPixelClickListener listener;
    private HalChat hc;
    private static final String TAG="PixelAdapter";

    public interface OnPixelClickListener {
        void onPixelClick(Pixel pixel);
    }

    public PixelAdapter(@NonNull Context context, @NonNull List<Pixel> pixelList, @NonNull OnPixelClickListener listener, HalChat hc) {
        this.context = context;
        this.pixelsList = pixelList;
        this.listener = listener;
        this.hc=hc;
    }

    @Override
    public int getCount() {
        return pixelsList.size();
    }

    @Override
    public Object getItem(int position) {
        return pixelsList.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        if (convertView == null) {
            convertView = LayoutInflater.from(context).inflate(R.layout.pixel_item, parent, false);
        }

        Pixel pixel=pixelsList.get(position);

        ImageView pixelImage = convertView.findViewById(R.id.pixel_image);
        hc.hd.getFileById(pixel.image).thenAccept(fileIcon->{
            new Handler(Looper.getMainLooper()).post(() -> {
                Glide.with(context)
                        .asDrawable()
                        .load(fileIcon)
                        .override(512, 512)
                        .placeholder(R.drawable.ic_robot)
                        .into(pixelImage);
            });
        });


        // Нажатие на пиксель
        pixelImage.setOnClickListener(v -> listener.onPixelClick(pixel));

        return convertView;
    }
}