package halwarsing.net.halchatandroid.main;

import android.media.MediaPlayer;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.View;
import android.widget.ImageButton;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.VideoView;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import java.util.Locale;

import halwarsing.net.halchatandroid.R;

//Просмотр видео из чата: своя панель (кнопка, время, перемотка) вместо MediaController
public class VideoPlayerActivity extends AppCompatActivity {
    private static final String TAG="HCAVIDEO";
    public static final String EXTRA_PATH="path";
    private static final String KEY_POSITION="position";
    private static final String KEY_PLAYING="playing";
    private static final long UPDATE_MS=250;
    private static final long HIDE_CONTROLS_MS=3000;

    private VideoView videoView;
    private SeekBar seekBar;
    private TextView timeText,durationText;
    private ImageButton playPauseBtn;
    private View controls,closeBtn;
    private final Handler handler=new Handler(Looper.getMainLooper());
    private int position=0;
    private boolean isPlaying=true;
    private boolean isSeeking=false;
    private MediaPlayer mediaPlayer; // из onPrepared, VideoView пересоздаёт его после сворачивания
    private static final long SEEK_TIMEOUT_MS=2000;

    // Если onSeekComplete не пришёл, не держим полосу вечно
    private final Runnable seekFinished=()->isSeeking=false;

    private final Runnable updateProgress=new Runnable() {
        @Override
        public void run() {
            if(!isSeeking&&videoView.getDuration()>0) {
                int pos=videoView.getCurrentPosition();
                seekBar.setProgress(pos);
                timeText.setText(formatTime(pos));
            }
            handler.postDelayed(this,UPDATE_MS);
        }
    };

    private final Runnable hideControls=()->{
        if(videoView.isPlaying()&&!isSeeking)setControlsVisible(false);
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video_player);
        if(savedInstanceState!=null) {
            position=savedInstanceState.getInt(KEY_POSITION,0);
            isPlaying=savedInstanceState.getBoolean(KEY_PLAYING,true);
        }

        String path=getIntent().getStringExtra(EXTRA_PATH);
        if(path==null){finish();return;}

        videoView=findViewById(R.id.videoView);
        seekBar=findViewById(R.id.videoSeek);
        timeText=findViewById(R.id.videoTime);
        durationText=findViewById(R.id.videoDuration);
        playPauseBtn=findViewById(R.id.videoPlayPause);
        controls=findViewById(R.id.videoControls);
        closeBtn=findViewById(R.id.videoClose);

        // После сворачивания VideoView готовит видео заново: продолжаем с сохранённой позиции
        videoView.setOnPreparedListener(mp->{
            mediaPlayer=mp;
            mp.setOnSeekCompleteListener(player->{
                handler.removeCallbacks(seekFinished);
                isSeeking=false;
            });
            int duration=videoView.getDuration();
            seekBar.setMax(Math.max(duration,0));
            durationText.setText(formatTime(duration));
            if(position>0)seekToExact(position);
            seekBar.setProgress(position);
            timeText.setText(formatTime(position));
            if(isPlaying)videoView.start();
            updatePlayPause();
            scheduleHideControls();
        });
        videoView.setOnCompletionListener(mp->{
            isPlaying=false;
            position=0;
            updatePlayPause();
            setControlsVisible(true);
        });
        videoView.setOnErrorListener((mp,what,extra)->{
            Log.e(TAG,"Video error "+what+";"+extra);
            Toast.makeText(this,R.string.video_play_failed,Toast.LENGTH_SHORT).show();
            finish();
            return true;
        });
        videoView.setVideoPath(path);

        playPauseBtn.setOnClickListener((View v)->{
            if(videoView.isPlaying()) {
                videoView.pause();
                isPlaying=false;
            } else {
                videoView.start();
                isPlaying=true;
            }
            updatePlayPause();
            scheduleHideControls();
        });

        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar bar,int progress,boolean fromUser) {
                if(fromUser)timeText.setText(formatTime(progress));
            }

            @Override
            public void onStartTrackingTouch(SeekBar bar) {
                isSeeking=true;
                handler.removeCallbacks(seekFinished);
                handler.removeCallbacks(hideControls);
            }

            @Override
            public void onStopTrackingTouch(SeekBar bar) {
                // isSeeking снимется в onSeekComplete, до этого полоса стоит там, куда нажали
                seekToExact(bar.getProgress());
                scheduleHideControls();
            }
        });

        findViewById(R.id.videoRoot).setOnClickListener((View v)->toggleControls());
        videoView.setOnClickListener((View v)->toggleControls());
        closeBtn.setOnClickListener((View v)->finish());
    }

    @Override
    protected void onResume() {
        super.onResume();
        if(videoView==null)return;
        handler.removeCallbacks(updateProgress);
        handler.post(updateProgress);
    }

    @Override
    protected void onPause() {
        super.onPause();
        if(videoView==null)return;
        handler.removeCallbacks(updateProgress);
        handler.removeCallbacks(hideControls);
        if(videoView.getCurrentPosition()>0)position=videoView.getCurrentPosition();
        videoView.pause();
    }

    @Override
    protected void onSaveInstanceState(@NonNull Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putInt(KEY_POSITION,position);
        outState.putBoolean(KEY_PLAYING,isPlaying);
    }

    /**
     * Перемотка точно в указанное время. VideoView.seekTo прыгает к ключевому кадру перед ним (на несколько секунд раньше)
     *
     * @param ms Время в миллисекундах
     * @return void
     */
    private void seekToExact(int ms) {
        isSeeking=true;
        handler.removeCallbacks(seekFinished);
        handler.postDelayed(seekFinished,SEEK_TIMEOUT_MS);
        seekBar.setProgress(ms);
        timeText.setText(formatTime(ms));
        try {
            if(mediaPlayer!=null) {
                mediaPlayer.seekTo(ms,MediaPlayer.SEEK_CLOSEST);
                return;
            }
        } catch(IllegalStateException e) {
            Log.w(TAG,"seekTo",e);
        }
        videoView.seekTo(ms);
    }

    private void updatePlayPause() {
        playPauseBtn.setImageResource(isPlaying?R.drawable.ic_file_pause:R.drawable.ic_file_play);
    }

    private void toggleControls() {
        setControlsVisible(controls.getVisibility()!=View.VISIBLE);
        scheduleHideControls();
    }

    private void setControlsVisible(boolean visible) {
        int visibility=visible?View.VISIBLE:View.GONE;
        controls.setVisibility(visibility);
        closeBtn.setVisibility(visibility);
    }

    private void scheduleHideControls() {
        handler.removeCallbacks(hideControls);
        handler.postDelayed(hideControls,HIDE_CONTROLS_MS);
    }

    /**
     * Время для панели плеера
     *
     * @param ms Миллисекунды
     * @return m:ss или h:mm:ss
     */
    private static String formatTime(int ms) {
        int s=Math.max(ms,0)/1000;
        if(s>=3600)return String.format(Locale.ROOT,"%d:%02d:%02d",s/3600,s/60%60,s%60);
        return String.format(Locale.ROOT,"%d:%02d",s/60,s%60);
    }
}
