package halwarsing.net.halchatandroid.main;

import android.util.Log;

import org.json.JSONObject;

import java.io.IOException;
import java.util.Iterator;
import java.util.concurrent.CompletableFuture;

import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.FormBody;
import okhttp3.HttpUrl;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.RequestBody;
import okhttp3.Response;

public class HalDriveAPI {
    private static final String TAG="HALDRIVEAPI";
    private static final String API_URL="https://haldrive.halwarsing.net/api";

    private final OkHttpClient client;
    private final String codeUser;

    public HalDriveAPI(OkHttpClient client, String codeUser) {
        this.client=client;
        this.codeUser=codeUser;
    }

    public CompletableFuture<JSONObject> apiGet(String reqAction, JSONObject params) {
        CompletableFuture<JSONObject> future=new CompletableFuture<>();

        HttpUrl.Builder urlBuilder=HttpUrl.parse(API_URL).newBuilder();
        urlBuilder.addQueryParameter("req",reqAction);

        if(params!=null) {
            Iterator<String> keys=params.keys();
            while(keys.hasNext()) {
                String key=keys.next();
                try {
                    urlBuilder.addQueryParameter(key,String.valueOf(params.get(key)));
                } catch (Exception e) {
                    Log.e(TAG, "Error parse param: "+key,e);
                }
            }
        }

        Request request=new Request.Builder()
                .url(urlBuilder.build())
                .addHeader("Cookie","uid="+codeUser)
                .get()
                .build();

        executeRequest(request, future);
        return future;
    }

    public CompletableFuture<JSONObject> apiReq(String reqAction, JSONObject params) {
        CompletableFuture<JSONObject> future = new CompletableFuture<>();

        HttpUrl url = HttpUrl.parse(API_URL).newBuilder()
                .addQueryParameter("req", reqAction)
                .build();

        FormBody.Builder formBuilder=new FormBody.Builder();

        if(params!=null) {
            Iterator<String> keys=params.keys();
            while(keys.hasNext()) {
                String key=keys.next();
                try {
                    formBuilder.add(key,String.valueOf(params.get(key)));
                } catch (Exception e) {
                    Log.e(TAG,"Error add param: "+key,e);
                }
            }
        }

        RequestBody body=formBuilder.build();

        Request request = new Request.Builder()
                .url(url)
                .addHeader("Cookie", "uid=" + codeUser)
                .post(body)
                .build();

        executeRequest(request, future);
        return future;
    }

    protected void executeRequest(Request request, CompletableFuture<JSONObject> future) {
        client.newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e) {
                Log.e(TAG, "Ошибка сети при запросе: " + request.url(), e);
                future.completeExceptionally(e);
            }

            @Override
            public void onResponse(Call call, Response response) {
                try {
                    if (!response.isSuccessful()) {
                        Log.e(TAG, "Сервер вернул ошибку HTTP " + response.code());
                        JSONObject errorJson = new JSONObject();
                        errorJson.put("errorCode", response.code());
                        errorJson.put("error", "HTTP Error: " + response.code());
                        future.complete(errorJson);
                        return;
                    }

                    String responseData = response.body() != null ? response.body().string() : "{}";
                    Log.e(TAG,"RES DATA: "+responseData);
                    JSONObject jsonResponse = new JSONObject(responseData);
                    future.complete(jsonResponse);

                } catch (Exception e) {
                    Log.e(TAG, "Ошибка парсинга ответа: " + request.url(), e);
                    future.completeExceptionally(e);
                } finally {
                    if (response.body() != null) {
                        response.body().close(); // Важно закрывать тело ответа, чтобы не было утечек памяти
                    }
                }
            }
        });
    }
}
