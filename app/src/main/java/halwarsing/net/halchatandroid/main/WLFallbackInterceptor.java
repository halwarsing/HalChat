package halwarsing.net.halchatandroid.main;

import android.util.Log;

import androidx.annotation.NonNull;

import java.io.IOException;

import okhttp3.Interceptor;
import okhttp3.Request;
import okhttp3.Response;

public class WLFallbackInterceptor implements Interceptor {
    private static final String TAG="WLFI";

    @NonNull
    @Override
    public Response intercept(@NonNull Chain chain) throws IOException {
        Request request=chain.request();

        try {
            //Попытка выполнить запрос
            return chain.proceed(request);
        } catch (IOException e) {
            //Сбрасываем кэш белых списков
            WhitelistDNS.invalidateCache();
            try {
                return chain.proceed(request);
            } catch (IOException retryExp) {
                Log.e(TAG,"Error:",retryExp);
                throw retryExp;
            }
        }
    }
}
