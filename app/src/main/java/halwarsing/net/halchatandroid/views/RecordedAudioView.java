package halwarsing.net.halchatandroid.views;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.media.MediaMetadataRetriever;
import android.media.MediaPlayer;
import android.os.Handler;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.annotation.Nullable;
import androidx.core.content.res.ResourcesCompat;

import java.io.IOException;

import halwarsing.net.halchatandroid.R;

public class RecordedAudioView extends LinearLayout {
    public String filePath;
    public TextView timeSpan;
    public WaveformView waveformView;
    public ImageButton playBtn;
    private long timeS;
    public boolean isPlay=false;
    private MediaPlayer mediaPlayer;
    private Handler handler = new Handler();
    private static final String TAG="RAV";
    private Runnable updateRunnable;

    public RecordedAudioView(Context context) {
        super(context);
    }

    public RecordedAudioView(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
    }

    public RecordedAudioView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    public void setAudio(Activity activity, String filePath) {
        this.filePath=filePath;
        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
        retriever.setDataSource(filePath);
        String durationStr = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION);
        long durationMs = Long.parseLong(durationStr);
        timeS=durationMs/1000;

        activity.runOnUiThread(() -> timeSpan.setText(getTimespanAudio(timeS)));
        isPlay=false;

        mediaPlayer=new MediaPlayer();
        try {
            mediaPlayer.setDataSource(filePath);
            mediaPlayer.prepare();
        } catch (IOException e) {
            Log.e(TAG,"setAudio",e);
        }

        mediaPlayer.setOnCompletionListener(mp -> {
            reset();
        });


    }

    public void reset() {
        mediaPlayer.seekTo(0);
        mediaPlayer.pause();
        isPlay=false;
        playBtn.setImageDrawable(ResourcesCompat.getDrawable(getResources(),R.drawable.ic_play,null));
        timeSpan.setText(getTimespanAudio(timeS));
        waveformView.setProgress(0.0f);
        handler.removeCallbacks(updateRunnable);
    }

    @SuppressLint("ClickableViewAccessibility")
    public void init(TextView timeSpan, WaveformView waveformView, ImageButton playBtn) {
        this.timeSpan=timeSpan;
        this.waveformView=waveformView;
        this.playBtn=playBtn;
        playBtn.setOnClickListener(this::playPause);
        updateRunnable=new Runnable() {
            @Override
            public void run() {
                if (mediaPlayer != null && mediaPlayer.isPlaying()) {
                    int currentPos = mediaPlayer.getCurrentPosition();
                    int duration = mediaPlayer.getDuration();
                    timeSpan.setText(getTimespanAudio(currentPos/1000));
                    waveformView.setProgress((float) currentPos / duration);
                    handler.postDelayed(this, 100);
                }
            }
        };

        waveformView.setOnTouchListener((view, event) -> {
            if (event.getAction() == MotionEvent.ACTION_DOWN || event.getAction() == MotionEvent.ACTION_MOVE) {
                float x = event.getX();
                float relativeProgress = x / waveformView.maxWidth;

                relativeProgress = Math.max(0f, Math.min(relativeProgress, 1f));
                int duration = mediaPlayer.getDuration();
                int seekToPosition = (int) (duration * relativeProgress);
                mediaPlayer.seekTo(seekToPosition);
                waveformView.setProgress(relativeProgress);
                return true;
            }
            return false;
        });
    }

    public void playPause(View v) {
        if(isPlay) {
            isPlay=false;
            playBtn.setImageDrawable(ResourcesCompat.getDrawable(getResources(),R.drawable.ic_play,null));
            if(mediaPlayer!=null&&mediaPlayer.isPlaying()) {
                mediaPlayer.pause();
            }
            handler.removeCallbacks(updateRunnable);
            return;
        }
        isPlay=true;
        playBtn.setImageDrawable(ResourcesCompat.getDrawable(getResources(),R.drawable.ic_pause,null));
        if(mediaPlayer!=null) {
            mediaPlayer.start();
            handler.post(updateRunnable);
        }
    }

    private String getTimespanAudio(long sec) {
        String s=String.valueOf(sec%60);
        if(s.length()==1){s="0"+s;}
        String min=String.valueOf(sec/60);
        return min+":"+s;
    }
}
