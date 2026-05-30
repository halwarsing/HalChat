package halwarsing.net.halchatandroid.views;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.util.AttributeSet;
import android.view.MotionEvent;

import androidx.annotation.Nullable;

public class ZoomableImageView extends androidx.appcompat.widget.AppCompatImageView {
    private Matrix matrix;
    private float currentScale = 1f;
    private float minScale = 1f;
    private float dx = 0f, dy = 0f;

    public ZoomableImageView(Context context) {
        super(context);
        init();
    }

    public ZoomableImageView(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public ZoomableImageView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    private void init() {
        matrix = new Matrix();
        setScaleType(ScaleType.MATRIX);
        post(this::resetZoom);
        post(this::fitToCenter);
    }

    private void fitToCenter() {
        if (getDrawable() == null) return;

        float viewWidth = getWidth();
        float viewHeight = getHeight();
        float drawableWidth = getDrawable().getIntrinsicWidth();
        float drawableHeight = getDrawable().getIntrinsicHeight();

        // Рассчитать минимальный масштаб для заполнения рамки
        minScale = Math.min(viewWidth / drawableWidth, viewHeight / drawableHeight);
        currentScale = minScale;

        float dx = (viewWidth - drawableWidth * minScale) / 2f;
        float dy = (viewHeight - drawableHeight * minScale) / 2f;

        matrix.setScale(minScale, minScale);
        matrix.postTranslate(dx, dy);

        setImageMatrix(matrix);
    }

    public void setZoom(float zoom) {
        if (getDrawable() == null) return;

        float scale = minScale * zoom;

        // Обновить масштаб
        matrix.reset();
        fitToCenter();
        matrix.postScale(scale / minScale, scale / minScale, getWidth() / 2f, getHeight() / 2f);
        setImageMatrix(matrix);

        currentScale = scale;
    }

    public void resetZoom() {
        setZoom(1.0f);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (event.getPointerCount() == 1) {
            handleDrag(event);
        }
        return true;
    }

    private void handleDrag(MotionEvent event) {
        switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                dx = event.getX();
                dy = event.getY();
                break;

            case MotionEvent.ACTION_MOVE:
                float offsetX = event.getX() - dx;
                float offsetY = event.getY() - dy;

                matrix.postTranslate(offsetX, offsetY);
                setImageMatrix(matrix);

                dx = event.getX();
                dy = event.getY();
                break;
        }
    }

    public Bitmap getTransformedBitmap() {
        if (getDrawable() == null) return null;

        Bitmap resultBitmap = Bitmap.createBitmap(getWidth(), getHeight(), Bitmap.Config.ARGB_8888);

        Canvas canvas = new Canvas(resultBitmap);
        draw(canvas);

        return resultBitmap;
    }
}