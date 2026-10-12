package halwarsing.net.halchatandroid.localai;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.StatFs;
import android.provider.OpenableColumns;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;
import java.util.Locale;
import java.util.Objects;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/** Copies a user-selected GGUF into stable app storage without using broad storage permissions. */
public final class ModelFileImporter implements AutoCloseable {
    private static final long FREE_SPACE_HEADROOM = 128L * 1024L * 1024L;
    private static final int BUFFER_SIZE = 1024 * 1024;

    public interface Callback {
        void onProgress(long copiedBytes, long totalBytes);

        void onImported(File file, String displayName);

        void onError(Exception error);
    }

    private final Context appContext;
    private final ExecutorService executor = Executors.newSingleThreadExecutor(r -> {
        Thread thread = new Thread(r, "gguf-import");
        thread.setPriority(Thread.NORM_PRIORITY - 1);
        return thread;
    });

    public ModelFileImporter(Context context) {
        appContext = context.getApplicationContext();
    }

    public void importGguf(Uri uri, Callback callback) {
        Objects.requireNonNull(uri, "uri");
        Objects.requireNonNull(callback, "callback");
        executor.execute(() -> {
            File partialFile = null;
            try {
                DocumentInfo info = queryDocumentInfo(uri);
                if (!info.displayName.toLowerCase(Locale.ROOT).endsWith(".gguf")) {
                    throw new IOException("Выбранный файл не имеет расширение .gguf");
                }

                File modelDirectory = getModelDirectory();
                ensureFreeSpace(modelDirectory, info.size);

                String safeName = sanitizeFileName(info.displayName);
                File destination = new File(modelDirectory, safeName);
                partialFile = new File(modelDirectory, safeName + ".part");

                copy(uri, partialFile, info.size, callback);
                verifyGguf(partialFile);
                Files.move(
                        partialFile.toPath(),
                        destination.toPath(),
                        StandardCopyOption.REPLACE_EXISTING
                );
                callback.onImported(destination, info.displayName);
            } catch (Exception error) {
                if (partialFile != null && partialFile.exists()) {
                    // Only our incomplete import is removed; an older valid model is never touched.
                    //noinspection ResultOfMethodCallIgnored
                    partialFile.delete();
                }
                callback.onError(error);
            }
        });
    }

    private File getModelDirectory() throws IOException {
        File storageRoot = appContext.getExternalFilesDir(null);
        if (storageRoot == null) {
            storageRoot = appContext.getFilesDir();
        }
        File directory = new File(storageRoot, "local-ai/models");
        if (!directory.exists() && !directory.mkdirs()) {
            throw new IOException("Не удалось создать каталог моделей: " + directory);
        }
        return directory;
    }

    private DocumentInfo queryDocumentInfo(Uri uri) throws IOException {
        String displayName = "model.gguf";
        long size = -1L;
        ContentResolver resolver = appContext.getContentResolver();
        try (Cursor cursor = resolver.query(
                uri,
                new String[]{OpenableColumns.DISPLAY_NAME, OpenableColumns.SIZE},
                null,
                null,
                null
        )) {
            if (cursor != null && cursor.moveToFirst()) {
                int nameColumn = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
                int sizeColumn = cursor.getColumnIndex(OpenableColumns.SIZE);
                if (nameColumn >= 0 && !cursor.isNull(nameColumn)) {
                    displayName = cursor.getString(nameColumn);
                }
                if (sizeColumn >= 0 && !cursor.isNull(sizeColumn)) {
                    size = cursor.getLong(sizeColumn);
                }
            }
        }
        return new DocumentInfo(displayName, size);
    }

    private static void ensureFreeSpace(File directory, long incomingSize) throws IOException {
        if (incomingSize < 0) {
            return;
        }
        StatFs statFs = new StatFs(directory.getAbsolutePath());
        long required = incomingSize + FREE_SPACE_HEADROOM;
        if (statFs.getAvailableBytes() < required) {
            throw new IOException("Недостаточно места. Нужно ещё минимум "
                    + formatBytes(required - statFs.getAvailableBytes()));
        }
    }

    private void copy(Uri uri, File destination, long totalBytes, Callback callback)
            throws IOException, InterruptedException {
        ContentResolver resolver = appContext.getContentResolver();
        try (InputStream rawInput = resolver.openInputStream(uri)) {
            if (rawInput == null) {
                throw new IOException("Android не смог открыть выбранный файл");
            }
            try (BufferedInputStream input = new BufferedInputStream(rawInput, BUFFER_SIZE);
                 BufferedOutputStream output = new BufferedOutputStream(
                         new FileOutputStream(destination), BUFFER_SIZE)) {
                byte[] buffer = new byte[BUFFER_SIZE];
                long copied = 0L;
                long lastReportAt = 0L;
                int read;
                while ((read = input.read(buffer)) != -1) {
                    if (Thread.currentThread().isInterrupted()) {
                        throw new InterruptedException("Импорт модели отменён");
                    }
                    output.write(buffer, 0, read);
                    copied += read;
                    if (copied - lastReportAt >= 8L * BUFFER_SIZE || copied == totalBytes) {
                        callback.onProgress(copied, totalBytes);
                        lastReportAt = copied;
                    }
                }
                output.flush();
                callback.onProgress(copied, totalBytes);
            }
        }
    }

    private static void verifyGguf(File file) throws IOException {
        byte[] magic = new byte[4];
        try (FileInputStream input = new FileInputStream(file)) {
            if (input.read(magic) != magic.length
                    || magic[0] != 'G'
                    || magic[1] != 'G'
                    || magic[2] != 'U'
                    || magic[3] != 'F') {
                throw new IOException("Файл не является GGUF-моделью");
            }
        }
    }

    private static String sanitizeFileName(String displayName) {
        String sanitized = displayName.replaceAll("[\\\\/:*?\"<>|\\p{Cntrl}]", "_").trim();
        return sanitized.isEmpty() ? "model.gguf" : sanitized;
    }

    private static String formatBytes(long bytes) {
        double gib = bytes / (1024.0 * 1024.0 * 1024.0);
        if (gib >= 1.0) {
            return String.format(Locale.ROOT, "%.2f ГБ", gib);
        }
        return String.format(Locale.ROOT, "%.1f МБ", bytes / (1024.0 * 1024.0));
    }

    @Override
    public void close() {
        executor.shutdownNow();
    }

    private static final class DocumentInfo {
        final String displayName;
        final long size;

        DocumentInfo(String displayName, long size) {
            this.displayName = displayName;
            this.size = size;
        }
    }
}
