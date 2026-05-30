package halwarsing.net.halchatandroid.main;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.drawable.Drawable;
import android.text.style.ReplacementSpan;

public class PaddedImageSpan extends ReplacementSpan {

    private final Drawable drawable;
    private final int padding; // Отступ между эмодзи
    private final int verticalOffset; // Регулировка высоты эмодзи

    public PaddedImageSpan(Context context, Drawable drawable, int padding, int verticalOffset) {
        this.drawable = drawable;
        this.padding = padding;
        this.verticalOffset = verticalOffset;

        // Устанавливаем размеры изображения
        //this.drawable.setBounds(5, 0, drawable.getIntrinsicWidth(), drawable.getIntrinsicHeight());
    }

    @Override
    public int getSize(Paint paint, CharSequence text, int start, int end, Paint.FontMetricsInt fm) {
        // Ширина эмодзи + отступ
        return drawable.getBounds().right + padding;
    }

    @Override
    public void draw(Canvas canvas, CharSequence text, int start, int end, float x, int top, int y, int bottom, Paint paint) {
        int transY = y - drawable.getBounds().bottom + verticalOffset; // Регулировка по вертикали
        canvas.save();
        canvas.translate(x+padding, transY);
        drawable.draw(canvas);
        canvas.restore();
    }
}