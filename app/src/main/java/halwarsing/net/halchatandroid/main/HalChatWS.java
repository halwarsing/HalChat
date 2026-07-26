package halwarsing.net.halchatandroid.main;

import android.util.Base64;
import android.util.Log;

import androidx.annotation.NonNull;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.nio.charset.StandardCharsets;
import java.security.KeyFactory;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.SecureRandom;
import java.security.spec.PKCS8EncodedKeySpec;
import java.security.spec.X509EncodedKeySpec;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ConcurrentHashMap;

import javax.crypto.Cipher;

import halwarsing.net.halchatandroid.type.HCAction;
import halwarsing.net.halchatandroid.type.HCChat;
import halwarsing.net.halchatandroid.type.HCMessage;
import halwarsing.net.halchatandroid.type.HalChatWSI;
import okhttp3.Response;
import okhttp3.WebSocket;
import okhttp3.WebSocketListener;

public class HalChatWS extends WebSocketListener {
    private HalChatWSI func;
    private String token;
    private static final String TAG="HCWS";
    private WebSocket ws;
    private HalChat hc;
    private Map<String, CompletableFuture<JSONObject>> pendingRequests = new ConcurrentHashMap<>();
    protected boolean isAuth;

    public HalChatWS(HalChatWSI func,String token,HalChat hc) {
        this.func=func;
        this.token=token;
        this.hc=hc;
        this.isAuth=false;
        hc.chatWS=this;
    }

    private void sendAuth(WebSocket socket) {
        JSONObject jdata=new JSONObject();
        try {
            jdata.put("action", "auth");
            jdata.put("token", token);
            socket.send(jdata.toString());
            Log.d(TAG,"Successfully send auth: "+token);
        } catch (JSONException e) {
            Log.e(TAG,"sendAuth",e);
        }
    }

    private String generateRequestId() {
        try {
            return UUID.randomUUID().toString(); // Генерация UUID
        } catch (Exception e) {
            // Если UUID не поддерживается, создаём случайный идентификатор
            SecureRandom random = new SecureRandom();
            return "req_" + Long.toString(random.nextLong(), 36).substring(2, 11);
        }
    }

    public void close() {
        ws.close(1000,"Close connection");
        this.isAuth=false;
        this.ws=null;
        hc.runEvent("onClose",new JSONObject());
    }

    protected CompletableFuture<JSONObject> apiReq(String req,JSONObject postData) {
        CompletableFuture<JSONObject> future=new CompletableFuture<>();
        // 1. Генерируем requestId
        String requestId = generateRequestId();

        // 2. Сохраняем в pendingRequests
        pendingRequests.put(requestId, future);

        try {
            // 3. Формируем JSON
            postData.put("req", req);
            JSONObject payload = new JSONObject();
            payload.put("action", "api");
            payload.put("reqId", requestId);
            payload.put("data", postData);

            // 4. Отправляем через WebSocket
            ws.send(payload.toString());

            Log.e(TAG,"Req: "+req+";"+requestId);
        } catch (Exception e) {
            pendingRequests.remove(requestId);
            future.completeExceptionally(e);
        }

        return future;
    }

    @Override
    public void onOpen(@NonNull WebSocket webSocket, @NonNull Response response) {
        Log.d(TAG, "Соединение установлено");
        this.ws=webSocket;
        sendAuth(webSocket);

        apiReq("getListChats",new JSONObject()).thenAccept(res->{
            Log.d(TAG, "Ответ получен: " + res.toString());
            if(res.optInt("errorCode")==0&&res.has("chats")) {
                JSONArray chats=res.optJSONArray("chats");
                TaskExecutorManager.getInstance().submitDecrypt("chats", () -> {
                    for (int i = 0; i < chats.length(); i++) {
                        JSONObject c = chats.optJSONObject(i);
                        String psw = hc.chatGroupChats.getPasswordChat(c.optLong("uid"));
                        if (psw == null) {
                            //NAF: need update system
                            //requestPasswordChat(c.optLong("uid")).get();
                        }
                    }
                    return null;
                });
            }
        }).exceptionally(error -> {
            Log.e(TAG, "Ошибка запроса", error);
            return null;
        });
    }

    @Override
    public void onMessage(@NonNull WebSocket webSocket, String text) {
        if(text.equals("pong")){
            if(!isAuth) {
                isAuth=true;
                TaskExecutorManager.getInstance().submitRealtime("ws-auth", () -> {
                    hc.runEvent("onAuth",new JSONObject());
                    return null;
                });
            }
            return;
        }
        Log.e(TAG, "Новое сообщение: " + text);

        JSONObject data;

        try {
            data=new JSONObject(text);
        } catch (JSONException e) {
            Log.e(TAG,"Received non-JSON data: "+text);
            return;
        }

        try {
            if(data.has("action")) {
                String action = data.getString("action");
                if (action.equals("api")) {
                    String requestId = data.optString("reqId");
                    if (data.has("reqId") && this.pendingRequests.containsKey(requestId)) {
                        CompletableFuture<JSONObject> future = pendingRequests.remove(requestId);
                        if (future != null) {
                            future.complete(data.optJSONObject("result"));
                        }
                    }
                }
                return;
            }
        } catch (JSONException e) {
            Log.e(TAG,"onMessage",e);
        }

        if(data.has("rtype")) {
            TaskExecutorManager.getInstance().submitRealtime("ws-realtime-event", () -> {
                handleRealtimeEvent(data);
                return null;
            });
        }
    }

    private void handleRealtimeEvent(JSONObject data) {
        String rtype=data.optString("rtype");
        if(rtype.equals("act")) {
            HCAction action=HCAction.getFromJSON(data);
            hc.runEvent("onNewAction",data,action);
        } else if(rtype.equals("msg")) {
            try {
                HCMessage msg=hc.chatGroupChats.jsonMsgToHCMSG(data);
                HCChat chat=hc.chatGroupChats.getChatInfo(msg.chatId);
                hc.runEvent("onNewMessage",data,msg,chat);
            } catch (JSONException e) {
                Log.e(TAG,"msg",e);
            }
        } else if(rtype.equals("sendPSW")) {
            hc.runEvent("onReceivePassword",data);
        } else if(rtype.equals("checkPSW")) {
            hc.runEvent("onCheckPassword",data);
        }
    }

    @Override
    public void onFailure(@NonNull WebSocket webSocket, @NonNull Throwable t, Response response) {
        Log.e(TAG, "Ошибка соединения", t);
        isAuth=false;
        func.reconnectWebSocket();
    }
}
