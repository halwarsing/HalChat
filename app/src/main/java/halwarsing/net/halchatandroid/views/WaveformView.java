package halwarsing.net.halchatandroid.views;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.media.MediaCodec;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class WaveformView extends View {
    private Paint paint;
    private int[] waveHeights=new int[]{};
    public static final int RECT_WIDTH = 8;
    public static final int SPACE_BETWEEN = 4;
    private static final String TAG="WV";

    private float progress = 0.0f;
    private int playedColor = 0xFF4CAF50;   // Цвет для проигранной части
    private int unplayedColor = 0xFFBDBDBD; // Цвет для оставшейся части

    public int maxWidth=0;

    public WaveformView(Context context, AttributeSet attrs) {
        super(context, attrs);
        paint = new Paint();
        paint.setStyle(Paint.Style.FILL);
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
    }

    public void setWaveformData(int[] waveform) {
        if(waveform==null)return;
        this.waveHeights = waveform;
        this.maxWidth=(RECT_WIDTH+SPACE_BETWEEN)*waveHeights.length+SPACE_BETWEEN;
        invalidate();
    }

    public int[] generateWaveformFromAudioFile(String filePath,int samplesCount) {
        try {
            if(samplesCount==0)return null;
            // 1. Инициализация MediaExtractor для получения аудиотрека
            MediaExtractor extractor = new MediaExtractor();
            extractor.setDataSource(filePath);
            int audioTrackIndex = -1;
            for (int i = 0; i < extractor.getTrackCount(); i++) {
                MediaFormat format = extractor.getTrackFormat(i);
                String mime = format.getString(MediaFormat.KEY_MIME);
                if (mime.startsWith("audio/")) {
                    audioTrackIndex = i;
                    break;
                }
            }
            if (audioTrackIndex == -1) {
                // Аудиодорожка не найдена
                return null;
            }
            extractor.selectTrack(audioTrackIndex);
            MediaFormat format = extractor.getTrackFormat(audioTrackIndex);

            // 2. Настройка декодера
            MediaCodec codec = MediaCodec.createDecoderByType(format.getString(MediaFormat.KEY_MIME));
            codec.configure(format, null, null, 0);
            codec.start();

            // 3. Декодирование аудио в PCM
            ByteArrayOutputStream pcmData = new ByteArrayOutputStream();
            boolean sawInputEOS = false;
            boolean sawOutputEOS = false;
            while (!sawOutputEOS) {
                if (!sawInputEOS) {
                    int inputBufferIndex = codec.dequeueInputBuffer(10000);
                    if (inputBufferIndex >= 0) {
                        ByteBuffer inputBuffer = codec.getInputBuffer(inputBufferIndex);
                        int sampleSize = extractor.readSampleData(inputBuffer, 0);
                        if (sampleSize < 0) {
                            codec.queueInputBuffer(inputBufferIndex, 0, 0, 0, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                            sawInputEOS = true;
                        } else {
                            long presentationTimeUs = extractor.getSampleTime();
                            codec.queueInputBuffer(inputBufferIndex, 0, sampleSize, presentationTimeUs, 0);
                            extractor.advance();
                        }
                    }
                }

                MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
                int outputBufferIndex = codec.dequeueOutputBuffer(bufferInfo, 10000);
                if (outputBufferIndex >= 0) {
                    ByteBuffer outputBuffer = codec.getOutputBuffer(outputBufferIndex);
                    byte[] chunk = new byte[bufferInfo.size];
                    outputBuffer.get(chunk);
                    outputBuffer.clear();
                    pcmData.write(chunk);
                    codec.releaseOutputBuffer(outputBufferIndex, false);
                    if ((bufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                        sawOutputEOS = true;
                    }
                }
            }
            codec.stop();
            codec.release();
            extractor.release();

            // 4. Преобразуем полученные байты в PCM-данные
            byte[] audioBytes = pcmData.toByteArray();
            int shortCount = audioBytes.length / 2;
            short[] audioSamples = new short[shortCount];
            ByteBuffer.wrap(audioBytes)
                    .order(ByteOrder.LITTLE_ENDIAN)
                    .asShortBuffer()
                    .get(audioSamples);

            // 5. Разбиваем массив аудиосэмплов на заданное количество сегментов
            int samplesPerSegment = audioSamples.length / samplesCount;
            int[] waveform = new int[samplesCount];
            for (int i = 0; i < samplesCount; i++) {
                int start = i * samplesPerSegment;
                int end = start + samplesPerSegment;
                short max = 0;
                // Вычисляем максимальное абсолютное значение в сегменте
                for (int j = start; j < end && j < audioSamples.length; j++) {
                    short sample = (short) Math.abs(audioSamples[j]);
                    if (sample > max) {
                        max = sample;
                    }
                }
                waveform[i] = max;
            }
            return waveform;
        } catch (IOException e) {
            Log.e(TAG,"generateWaveformFromAudioFile",e);
        }
        return null;
    }

    public void setProgress(float progress) {
        this.progress = progress;
        invalidate();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        if (waveHeights == null || waveHeights.length == 0) return;

        // Найдем максимальное значение амплитуды
        int maxValue = 0;
        for (int amp : waveHeights) {
            if (amp > maxValue) {
                maxValue = amp;
            }
        }
        if (maxValue == 0) maxValue = 1; // избегаем деления на 0

        int x = 0;
        int progWidth=(int)(maxWidth*progress);

        for (int height : waveHeights) {
            int scaledHeight = (int) (((float) height / maxValue) * getHeight());

            if(x+RECT_WIDTH<progWidth) {
                paint.setColor(playedColor);
            } else {
                paint.setColor(unplayedColor);
            }

            canvas.drawRect(x, Math.min(getHeight() - scaledHeight,getHeight()-1), x + RECT_WIDTH, getHeight(), paint);

            x += RECT_WIDTH + SPACE_BETWEEN;
        }
    }

    @Override
    public boolean performClick() {
        return super.performClick();
    }
}