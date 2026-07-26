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
import com.bumptech.glide.load.engine.DiskCacheStrategy;

import java.util.List;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.type.Emoji;

public class ReactionEmojiAdapter extends BaseAdapter {
    public interface OnEmojiClickListener {
        void onEmojiClick(Emoji emoji);
    }

    private final Context context;
    private final List<Emoji> emojis;
    private final OnEmojiClickListener listener;
    private final HalChat hc;

    public ReactionEmojiAdapter(
            @NonNull Context context,
            @NonNull List<Emoji> emojis,
            @NonNull OnEmojiClickListener listener,
            @NonNull HalChat hc
    ) {
        this.context=context;
        this.emojis=emojis;
        this.listener=listener;
        this.hc=hc;
    }

    @Override
    public int getCount() {
        return emojis.size();
    }

    @Override
    public Emoji getItem(int position) {
        return emojis.get(position);
    }

    @Override
    public long getItemId(int position) {
        return emojis.get(position).emojiId;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        if(convertView==null) {
            convertView=LayoutInflater.from(context).inflate(
                    R.layout.reaction_emoji_item,
                    parent,
                    false
            );
        }

        Emoji emoji=getItem(position);
        ImageView image=convertView.findViewById(R.id.reaction_emoji_image);
        image.setTag(emoji.emojiId);
        image.setImageResource(R.drawable.ic_robot);
        convertView.setOnClickListener(view->listener.onEmojiClick(emoji));

        hc.hd.getFileById(emoji.image).thenAccept(file->
                new Handler(Looper.getMainLooper()).post(()->{
                    Object tag=image.getTag();
                    if(tag instanceof Long && ((Long)tag)==emoji.emojiId) {
                        Glide.with(context)
                                .load(file)
                                .diskCacheStrategy(DiskCacheStrategy.ALL)
                                .skipMemoryCache(false)
                                .placeholder(R.drawable.ic_robot)
                                .into(image);
                    }
                })
        );

        return convertView;
    }
}
