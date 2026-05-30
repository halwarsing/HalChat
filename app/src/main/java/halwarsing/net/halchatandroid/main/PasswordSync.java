package halwarsing.net.halchatandroid.main;

import android.database.Cursor;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONObject;

import java.security.KeyFactory;
import java.security.PublicKey;
import java.security.spec.X509EncodedKeySpec;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CompletableFuture;

import halwarsing.net.halchatandroid.encryption.RSACipher;
import halwarsing.net.halchatandroid.type.HCChat;

public class PasswordSync {
    private static final String TAG="PSWSync";
    protected HalChat hc;

    public PasswordSync(HalChat hc) {
        this.hc=hc;
    }

    //Send availables passwords

    //Send available password to chat by publicKey and id request. ChatUID need for getting password.
    public CompletableFuture<Void> sendPasswordChat(long chatUID, long id_req, PublicKey publicKey) {
        if(!hc.chatSettingsApp.getBoolean(HalChatSettingsApp.KEY_TRANSFER_PASSWORDS))return CompletableFuture.completedFuture(null);
        if(chatUID>0&&id_req>0&&publicKey!=null&&hc.chatGroupChats.hasPasswordChat(chatUID)) {
            final String password=hc.chatGroupChats.getPasswordChat(chatUID);
            try {
                CompletableFuture<Void> future = new CompletableFuture<>();
                RSACipher rsa=new RSACipher(publicKey,null);
                final String encPSW=rsa.encrypt(password).replaceAll("\\r?\\n","");

                JSONObject postData=new JSONObject();
                postData.put("uid",id_req);
                postData.put("password",encPSW);

                hc.hcapi.apiReq("sendPasswordChatAuto",postData).thenAccept(data->{
                    try {
                        if (data.getInt("errorCode") == 0) {
                            Log.e(TAG,"Successfully sendPasswordChatAuto in chatUID:"+ chatUID);
                            future.complete(null);
                        } else {
                            Log.e(TAG,"Error sendPasswordChatAuto: "+data.getString("error")+";"+data.getInt("errorCode"));
                            future.complete(null);
                        }
                    } catch (Exception e) {
                        Log.e(TAG,"Error in sendPasswordChatAuto",e);
                    }
                    future.complete(null);
                }).exceptionally(throwable -> {
                    future.complete(null);
                    return null;
                });

                return future;
            } catch (Exception e) {
                Log.e(TAG,"Error in sendPasswordChat",e);
            }
        }
        return CompletableFuture.completedFuture(null);
    }

    //Get requests password for chat by chatUID. If requests are existing it send password to request.
    public CompletableFuture<Void> checkToSendRequestsPassword(long chatUID) {
        if(chatUID>0) {
            try {
                JSONObject postData=new JSONObject();
                postData.put("chatId",chatUID);

                return hc.hcapi.apiReq("getRequestsPasswordChat",postData).thenCompose(data->{
                    try {
                        if(data.getInt("errorCode")==0) {
                            JSONObject req;
                            JSONArray requests=data.getJSONArray("requests");
                            List<CompletableFuture<?>> futures = new ArrayList<>();

                            for(int i=0;i<requests.length();i++) {
                                req=requests.getJSONObject(i);
                                X509EncodedKeySpec spec = new X509EncodedKeySpec(hexToBytes(req.getString("publicKey")));
                                KeyFactory kf = KeyFactory.getInstance("RSA");
                                PublicKey publicKey=kf.generatePublic(spec);
                                CompletableFuture<Void> f=sendPasswordChat(chatUID,req.getLong("uid"),publicKey);
                                futures.add(f);
                            }
                            return CompletableFuture.allOf(futures.toArray(new CompletableFuture[0]));
                        } else {
                            Log.e(TAG,"Error getRequestsPasswordChat: "+data.getString("error")+";"+data.getInt("errorCode"));
                        }
                    } catch (Exception e) {
                        Log.e(TAG,"Error in getRequestsPasswordChat",e);

                    }
                    return CompletableFuture.completedFuture(null);
                });
            } catch (Exception e) {
                Log.e(TAG, "Error in checkToSendRequestsPassword", e);
            }
        }
        return CompletableFuture.completedFuture(null);
    }

    //Check all available password on this device (selected account)
    public void checkAllAvailablePasswords() {
        Cursor chatCur=hc.db.rawQuery("SELECT * FROM `groupChats` WHERE `isDelete`=0 AND `password`!='-1'",null);
        if (chatCur.moveToFirst()) {
            do {
                try {
                    HCChat chat=HalChatGroupChats.getChatFromCursor(chatCur);
                    checkToSendRequestsPassword(chat.chatUID).get();
                } catch (Exception e) {
                    Log.e(TAG,"Error in checkAllAvailablePasswords",e);
                }
            } while (chatCur.moveToNext());
        }
        chatCur.close();
    }

    //Detect and request missing passwords for downloaded chats

    //request missing password by chatUID
    public CompletableFuture<Void> requestMissingPassword(long chatUID) {
        if(chatUID>0&&!hc.chatGroupChats.hasPasswordChat(chatUID)) {
            try {
                CompletableFuture<Void> future=new CompletableFuture<>();

                RSACipher rsa=new RSACipher(hc.context,chatUID);
                String publicKeyHex=bytesToHex(rsa.getPublicKey().getEncoded());

                JSONObject postData=new JSONObject();
                postData.put("chatId",chatUID);
                postData.put("publicKey",publicKeyHex);

                hc.hcapi.apiReq("requestPasswordAuto",postData).thenAccept(data->{
                    try {
                        if(data.getInt("errorCode")==0) {
                            Log.e(TAG,"Successfully request missing password chatUID:"+chatUID);
                        } else {
                            Log.e(TAG,"Error requestPasswordAuto: "+data.getString("error")+";"+data.getInt("errorCode"));
                            RSACipher.deletePrivateKeyRSA(hc.context,chatUID);
                        }
                    } catch (Exception e) {
                        Log.e(TAG,"Error in requestPasswordAuto",e);
                    }
                    future.complete(null);
                });

                return future;
            } catch (Exception e) {
                Log.e(TAG, "Error in requestMissingPassword", e);
            }
        }
        return CompletableFuture.completedFuture(null);
    }

    //request missing passwords from downloaded chats
    public void requestMissingPasswords() {
        Cursor chatCur=hc.db.rawQuery("SELECT * FROM `groupChats` WHERE `isDelete`=0 AND `password`='-1'",null);
        if (chatCur.moveToFirst()) {
            do {
                try {
                    HCChat chat=HalChatGroupChats.getChatFromCursor(chatCur);
                    if(!RSACipher.hasPrivateKeyRSA(hc.context,chat.chatUID)) {
                        requestMissingPassword(chat.chatUID).get();
                    }
                } catch (Exception e) {
                    Log.e(TAG,"Error in requestMissingPasswords",e);
                }
            } while (chatCur.moveToNext());
        }
        chatCur.close();
    }

    //Check sent requests

    //decrypt received password
    public Boolean decryptRequestPassword(long chatUID,String psw) {
        try {
            if (chatUID > 0 && !hc.chatGroupChats.hasPasswordChat(chatUID) && RSACipher.hasPrivateKeyRSA(hc.context,chatUID)) {

                // Параметры OAEP с SHA-256
                RSACipher rsa=new RSACipher(hc.context,chatUID);
                String decryptedPassword=rsa.decrypt(psw);

                Log.e(TAG,"Successfully decrypted received password chatUID:"+chatUID);
                hc.db.execSQL("UPDATE `groupChats` SET `password`=? WHERE `chatUID`=?", new String[]{decryptedPassword, String.valueOf(chatUID)});

                //Download chat messages and additional info
                hc.chatGroupChats.checkChat(chatUID);

                //Add on chatlist
                if(hc.chatListI!=null) {
                    hc.chatListI.onEnterChat(hc.chatGroupChats.getChatInfo(chatUID));
                }

                return true;
            }
        } catch (Exception e) {
            Log.e(TAG, "Error in decryptRequestPassword", e);
        }
        return false;
    }

    //check request password in chat by chatUID
    public CompletableFuture<Void> checkRequestPassword(long chatUID) {
        if(chatUID>0&&!hc.chatGroupChats.hasPasswordChat(chatUID)) {
            try {
                CompletableFuture<Void> future=new CompletableFuture<>();

                JSONObject postData=new JSONObject();
                postData.put("chatId",chatUID);

                hc.hcapi.apiReq("getRequestPasswordAuto",postData).thenAccept(data->{
                    try {
                        if (data.getInt("errorCode") == 0) {
                            Log.e(TAG, "Successfully received password chatUID:" + chatUID);
                            decryptRequestPassword(chatUID,data.getString("psw"));
                        } else if(data.getInt("errorCode")==5) {
                            Log.e(TAG,"Password didn't receive chatUID:"+chatUID);
                        } else if(data.getInt("errorCode")==4) {
                            Log.e(TAG,data.getString("error")+" chatUID:"+chatUID);
                        } else {
                            Log.e(TAG,"Error getRequestPasswordAuto: "+data.getString("error")+";"+data.getInt("errorCode"));
                        }
                    } catch (Exception e) {
                        Log.e(TAG,"Error in getRequestPasswordAuto",e);
                    }
                    future.complete(null);
                });

                return future;
            } catch (Exception e) {
                Log.e(TAG, "Error in checkRequestPassword", e);
            }
        }
        return CompletableFuture.completedFuture(null);
    }

    //check all requests from this device (session)
    public void checkRequestsPasswords() {
        Cursor chatCur=hc.db.rawQuery("SELECT * FROM `groupChats` WHERE `isDelete`=0 AND `password`='-1'",null);
        if (chatCur.moveToFirst()) {
            do {
                try {
                    HCChat chat=HalChatGroupChats.getChatFromCursor(chatCur);
                    if(RSACipher.hasPrivateKeyRSA(hc.context,chat.chatUID)) {
                        checkRequestPassword(chat.chatUID).get();
                    }
                } catch (Exception e) {
                    Log.e(TAG,"Error in checkRequestsPasswords",e);
                }
            } while (chatCur.moveToNext());
        }
        chatCur.close();
    }

    //Event on receive password in ws
    public void onReceivePassword(JSONObject data) {
        Log.e(TAG,"onReceivePassword: "+data.toString());
        try {
            TaskExecutorManager.getInstance().submitPasswordT("receivepassword:chatId"+data.getLong("chatId")+":"+data.getLong("reqId"),
                    ()->{
                        if(decryptRequestPassword(data.getLong("chatId"), data.getString("psw"))) {
                            JSONObject postData=new JSONObject();
                            postData.put("chatId",data.getLong("chatId"));

                            hc.hcapi.apiReq("getRequestPasswordAuto",postData);
                        }
                        return null;
                    }
            );

        } catch (Exception e) {
            Log.e(TAG,"Error in onReceivePassword",e);
        }
    }

    //Event on check password in ws (to send available password)
    public void onCheckPassword(JSONObject data) {
        Log.e(TAG,"onCheckPassword: "+data.toString());
        try {
            TaskExecutorManager.getInstance().submitPasswordT("checkpassword:chatId"+data.getLong("chatId")+":"+data.getLong("reqId"),
                    ()->{
                        X509EncodedKeySpec spec = new X509EncodedKeySpec(hexToBytes(data.getString("publicKey")));
                        KeyFactory kf = KeyFactory.getInstance("RSA");
                        sendPasswordChat(data.getLong("chatId"),data.getLong("reqId"),kf.generatePublic(spec)).get();
                        return null;
                    }
            );

        } catch (Exception e) {
            Log.e(TAG,"Error in onReceivePassword",e);
        }
    }

    public static byte[] hexToBytes(String hex) {
        if (hex.length() % 2 != 0) {
            throw new IllegalArgumentException("Hex string must have even length");
        }
        int len = hex.length();
        byte[] data = new byte[len / 2];
        for (int i = 0; i < len; i += 2) {
            int hi = Character.digit(hex.charAt(i), 16);
            int lo = Character.digit(hex.charAt(i + 1), 16);
            if (hi < 0 || lo < 0) {
                throw new IllegalArgumentException(
                        "Invalid hex character at position " + i + " or " + (i + 1)
                );
            }
            data[i / 2] = (byte) ((hi << 4) + lo);
        }
        return data;
    }

    public static String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder(bytes.length * 2);
        for (byte b : bytes) {
            sb.append(String.format("%02x", b & 0xFF));
        }
        return sb.toString();
    }
}
