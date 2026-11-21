package halwarsing.net.halchatandroid.main;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.provider.OpenableColumns;
import android.webkit.MimeTypeMap;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.List;
import java.util.Locale;

import halwarsing.net.halchatandroid.type.HDFile;

//Отдаёт скачанные файлы HalDrive другим приложениям только на чтение: content://<пакет>.halDriveFiles/<id>/<имя.расширение>
public class HalDriveFileProvider extends ContentProvider {
    private static final String TAG="HCAHDFP";
    private static final String AUTHORITY_SUFFIX=".halDriveFiles";

    /**
     * URI для открытия файла в другом приложении: настоящее имя с расширением в пути и в DISPLAY_NAME
     *
     * @param context Контекст
     * @param hdFile Файл HalDrive (скачанный)
     * @return content:// URI
     */
    public static Uri getUri(Context context,HDFile hdFile) {
        return new Uri.Builder().scheme("content").authority(context.getPackageName()+AUTHORITY_SUFFIX)
                .appendPath(hdFile.id).appendPath(HalDrive.getSaveFileName(hdFile)).build();
    }

    /**
     * MIME-тип по расширению имени
     *
     * @param name Имя файла
     * @return MIME-тип или application/octet-stream
     */
    public static String getMimeTypeByName(String name) {
        int dot=name==null?-1:name.lastIndexOf('.');
        if(dot!=-1&&dot<name.length()-1) {
            String mimeType=MimeTypeMap.getSingleton().getMimeTypeFromExtension(name.substring(dot+1).toLowerCase(Locale.ROOT));
            if(mimeType!=null)return mimeType;
        }
        return "application/octet-stream";
    }

    //Только файлы HalDriveFiles/<id>.hdf, id без разделителей пути
    @Nullable
    private File getFile(Uri uri) {
        List<String> segments=uri.getPathSegments();
        if(segments.size()!=2)return null;
        String id=segments.get(0);
        if(!id.matches("[A-Za-z0-9_\\-]+"))return null;
        File file=new File(new File(getContext().getExternalFilesDir(null),HalDrive.PATH_HD_FILES),id+".hdf");
        return file.exists()?file:null;
    }

    @Override
    public boolean onCreate() {
        return true;
    }

    @Nullable
    @Override
    public Cursor query(@NonNull Uri uri,@Nullable String[] projection,@Nullable String selection,@Nullable String[] selectionArgs,@Nullable String sortOrder) {
        File file=getFile(uri);
        if(file==null)return null;
        if(projection==null)projection=new String[]{OpenableColumns.DISPLAY_NAME,OpenableColumns.SIZE};

        MatrixCursor cursor=new MatrixCursor(projection,1);
        Object[] row=new Object[projection.length];
        for(int i=0;i<projection.length;i++) {
            if(OpenableColumns.DISPLAY_NAME.equals(projection[i])) {
                row[i]=uri.getLastPathSegment();
            } else if(OpenableColumns.SIZE.equals(projection[i])) {
                row[i]=file.length();
            }
        }
        cursor.addRow(row);
        return cursor;
    }

    @Nullable
    @Override
    public String getType(@NonNull Uri uri) {
        return getMimeTypeByName(uri.getLastPathSegment());
    }

    @Nullable
    @Override
    public ParcelFileDescriptor openFile(@NonNull Uri uri,@NonNull String mode) throws FileNotFoundException {
        if(!"r".equals(mode))throw new FileNotFoundException("Read only: "+uri);
        File file=getFile(uri);
        if(file==null)throw new FileNotFoundException("No file: "+uri);
        return ParcelFileDescriptor.open(file,ParcelFileDescriptor.MODE_READ_ONLY);
    }

    @Nullable
    @Override
    public Uri insert(@NonNull Uri uri,@Nullable ContentValues values) {
        throw new UnsupportedOperationException("Read only");
    }

    @Override
    public int delete(@NonNull Uri uri,@Nullable String selection,@Nullable String[] selectionArgs) {
        return 0;
    }

    @Override
    public int update(@NonNull Uri uri,@Nullable ContentValues values,@Nullable String selection,@Nullable String[] selectionArgs) {
        return 0;
    }
}
