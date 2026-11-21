package halwarsing.net.halchatandroid.encryption;

import android.util.Base64;
import android.util.Log;

import com.goterl.lazysodium.LazySodiumAndroid;
import com.goterl.lazysodium.SodiumAndroid;

import org.bouncycastle.crypto.digests.SHA256Digest;
import org.bouncycastle.crypto.generators.HKDFBytesGenerator;
import org.bouncycastle.crypto.params.HKDFParameters;

import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.LinkedHashMap;
import java.util.Map;

/* v - версия шифрования сообщения
0-1 - HalEncryption (HalHash), только старые сообщения
2 - XChaCha20-Poly1305 IETF, ключ чата HKDF-SHA256 (HalChatEncryptor2.js / HalChatDecryptor2.js)
*/
//Шифрование сообщений чата v>=2
public class HalChatCipher {
    public static final String VERSION="0.0.3";
    public static final int VER_MSG=2;
    private static final String TAG="HCACIPHER";
    private static final int NONCE_LEN=24;
    private static final int TAG_LEN=16;
    private static final int KEY_LEN=32;
    private static final int KEY_CACHE_LIMIT=200;

    private static final LazySodiumAndroid ls=new LazySodiumAndroid(new SodiumAndroid());

    //LRU как keyCache в JS
    private static final Map<String,byte[]> keyCache=new LinkedHashMap<String,byte[]>(16,0.75f,true) {
        @Override
        protected boolean removeEldestEntry(Map.Entry<String,byte[]> eldest) {return size()>KEY_CACHE_LIMIT;}
    };

    /**
     * Ключ чата из пароля через HKDF-SHA256, как deriveChatKey в JS
     *
     * @param password Пароль чата
     * @param chatId UID чата
     * @param v Версия шифрования
     * @return Ключ 32 байта
     */
    private static byte[] deriveChatKey(String password,long chatId,long v) {
        HKDFBytesGenerator hkdf=new HKDFBytesGenerator(new SHA256Digest());
        hkdf.init(new HKDFParameters(password.getBytes(StandardCharsets.UTF_8),
                ("HalChat chat "+chatId).getBytes(StandardCharsets.UTF_8),
                ("HalChat msg v"+v).getBytes(StandardCharsets.UTF_8)));
        byte[] key=new byte[KEY_LEN];
        hkdf.generateBytes(key,0,KEY_LEN);
        return key;
    }

    /**
     * Ключ чата из кэша, при отсутствии создаётся
     *
     * @param password Пароль чата
     * @param chatId UID чата
     * @param v Версия шифрования
     * @return Ключ 32 байта
     */
    private static byte[] getChatKey(String password,long chatId,long v) {
        String id=v+"|"+chatId+"|"+password;
        synchronized(keyCache) {
            byte[] key=keyCache.get(id);
            if(key!=null)return key;
        }
        byte[] key=deriveChatKey(password,chatId,v);
        synchronized(keyCache) {keyCache.put(id,key);}
        return key;
    }

    /**
     * Очищает кэш ключей чатов
     *
     * @return void
     */
    public static void clearChatKeys() {
        synchronized(keyCache) {keyCache.clear();}
    }

    /**
     * Дополнительные данные AEAD: привязка шифротекста к чату, отправителю и сообщению
     *
     * @param v Версия шифрования
     * @param chatId UID чата
     * @param uid ID отправителя
     * @param encryptId encryptId сообщения (hex)
     * @return Байты v|chatId|uid|encryptId
     */
    private static byte[] buildAad(long v,long chatId,long uid,String encryptId) {
        return (v+"|"+chatId+"|"+uid+"|"+encryptId).getBytes(StandardCharsets.UTF_8);
    }

    /**
     * Шифрует текст сообщения текущей версией VER_MSG
     *
     * @param text Текст сообщения
     * @param password Пароль чата
     * @param chatId UID чата
     * @param uid ID отправителя (текущий пользователь)
     * @param encryptId encryptId сообщения (hex)
     * @return base64 (nonce | шифротекст | tag)
     * @throws IllegalArgumentException Если пароль чата неизвестен
     */
    public static String encrypt(String text,String password,long chatId,long uid,String encryptId) {
        if(password==null)throw new IllegalArgumentException("Нет пароля чата");
        byte[] key=getChatKey(password,chatId,VER_MSG);
        byte[] aad=buildAad(VER_MSG,chatId,uid,encryptId);
        byte[] msg=text.getBytes(StandardCharsets.UTF_8);
        byte[] nonce=ls.randomBytesBuf(NONCE_LEN);
        byte[] sealed=new byte[msg.length+TAG_LEN];
        if(!ls.cryptoAeadXChaCha20Poly1305IetfEncrypt(sealed,null,msg,msg.length,aad,aad.length,null,nonce,key)) {
            throw new IllegalStateException("Не удалось зашифровать сообщение");
        }
        byte[] out=new byte[NONCE_LEN+sealed.length];
        System.arraycopy(nonce,0,out,0,NONCE_LEN);
        System.arraycopy(sealed,0,out,NONCE_LEN,sealed.length);
        return Base64.encodeToString(out,Base64.NO_WRAP);
    }

    /**
     * Расшифровывает текст сообщения v>=2
     *
     * @param data64 base64 (nonce | шифротекст | tag)
     * @param password Пароль чата
     * @param chatId UID чата
     * @param v Версия шифрования сообщения
     * @param uid ID отправителя (fromId)
     * @param encryptId encryptId сообщения (hex)
     * @return Текст или null, если данные повреждены, пароль неверный или сообщение подменено
     */
    public static String decrypt(String data64,String password,long chatId,long v,long uid,String encryptId) {
        try {
            byte[] data=Base64.decode(data64,Base64.DEFAULT);
            if(data.length<NONCE_LEN+TAG_LEN)return null;
            byte[] key=getChatKey(password,chatId,v);
            byte[] aad=buildAad(v,chatId,uid,encryptId);
            byte[] nonce=Arrays.copyOfRange(data,0,NONCE_LEN);
            byte[] sealed=Arrays.copyOfRange(data,NONCE_LEN,data.length);
            byte[] plain=new byte[sealed.length-TAG_LEN];
            if(!ls.cryptoAeadXChaCha20Poly1305IetfDecrypt(plain,null,null,sealed,sealed.length,aad,aad.length,nonce,key))return null;
            return new String(plain,StandardCharsets.UTF_8);
        } catch(Exception e) {
            Log.w(TAG,"decrypt",e);
            return null;
        }
    }
}
