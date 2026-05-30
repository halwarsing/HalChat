package halwarsing.net.halchatandroid.main;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
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
import halwarsing.net.halchatandroid.type.EmojiPack;

public class EmojiAdapter extends BaseAdapter {
    private Context context;
    private List<Emoji> emojiList;
    private OnEmojiClickListener listener;
    private HalChat hc;
    private static final String TAG="EmojiAdapter";

    public interface OnEmojiClickListener {
        void onEmojiClick(Emoji emoji);
    }

    public EmojiAdapter(@NonNull Context context, @NonNull List<Emoji> emojiList, @NonNull OnEmojiClickListener listener, HalChat hc) {
        this.context = context;
        this.emojiList = emojiList;
        this.listener = listener;
        this.hc=hc;
    }

    @Override
    public int getCount() {
        return emojiList.size();
    }

    @Override
    public Object getItem(int position) {
        return emojiList.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        if (convertView == null) {
            convertView = LayoutInflater.from(context).inflate(R.layout.emoji_item, parent, false);
        }

        Emoji emoji=emojiList.get(position);

        ImageView emojiImage = convertView.findViewById(R.id.emoji_image);
        hc.hd.getFileById(emoji.image).thenAccept(fileIcon->{
            new Handler(Looper.getMainLooper()).post(() -> {
                Glide.with(context)
                        .asDrawable()
                        .load(fileIcon)
                        .override(128, 128)
                        .placeholder(R.drawable.ic_robot)
                        .into(emojiImage);
            });
        });


        // Нажатие на эмодзи
        emojiImage.setOnClickListener(v -> listener.onEmojiClick(emoji));

        return convertView;
    }
}