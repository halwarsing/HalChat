package halwarsing.net.halchatandroid.main;

import okhttp3.MediaType;
import okhttp3.RequestBody;
import okio.BufferedSink;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

public class ProgressRequestBody extends RequestBody {
    private final File file;
    private final String contentType;
    private final ProgressCallback listener;

    public interface ProgressCallback {
        void onProgressUpdate(int percentage);
    }

    public ProgressRequestBody(File file, String contentType, ProgressCallback listener) {
        this.file = file;
        this.contentType = contentType;
        this.listener = listener;
    }

    @Override
    public MediaType contentType() {
        return MediaType.parse(contentType);
    }

    @Override
    public long contentLength() {
        return file.length();
    }

    @Override
    public void writeTo(BufferedSink sink) throws IOException {
        long fileLength = file.length();
        byte[] buffer = new byte[4096];
        try (FileInputStream in = new FileInputStream(file)) {
            long uploaded = 0;
            int read;
            while ((read = in.read(buffer)) != -1) {
                uploaded += read;
                sink.write(buffer, 0, read);
                // Вычисляем и отправляем прогресс
                if (listener != null && fileLength > 0) {
                    int progress = (int) ((uploaded * 100) / fileLength);
                    listener.onProgressUpdate(progress);
                }
            }
        }
    }
}