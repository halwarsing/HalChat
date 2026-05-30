package halwarsing.net.halchatandroid.encryption;

import android.util.Log;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;

public class AESGCMHelper {
    private static final int GCM_TAG_LENGTH=128;
    private static final int IV_LENGTH = 12;

    //Кэш Cipher
    private static final ThreadLocal<Cipher> CIPHER_THREAD_LOCAL = ThreadLocal.withInitial(() -> {
        try {
            return Cipher.getInstance("AES/GCM/NoPadding");
        } catch (Exception e) {
            throw new RuntimeException("Не удалось инициализировать Cipher", e);
        }
    });

    public static String encrypt(String text,long chatId) throws Exception {
        SecretKey secretKey=KeyStoreHelper.getSecretKey(chatId);

        Cipher cipher=Cipher.getInstance("AES/GCM/NoPadding");
        cipher.init(Cipher.ENCRYPT_MODE,secretKey);

        byte[] iv=cipher.getIV();

        byte[] ciphertext=cipher.doFinal(text.getBytes(StandardCharsets.UTF_8));

        ByteBuffer byteBuffer=ByteBuffer.allocate(iv.length+ciphertext.length);
        byteBuffer.put(iv);
        byteBuffer.put(ciphertext);

        byte[] enc=byteBuffer.array();

        StringBuilder out=new StringBuilder();
        for(int i=0;i<enc.length;i++) {
            out.append(String.format("%02x", enc[i]));
        }
        return out.toString();
    }

    public static String decrypt(byte[] data,long chatId) throws Exception {
        if(data.length<IV_LENGTH) {
            throw new IllegalArgumentException("Small array length");
        }

        SecretKey secretKey=KeyStoreHelper.getSecretKey(chatId);

        Cipher cipher=CIPHER_THREAD_LOCAL.get();

        GCMParameterSpec spec=new GCMParameterSpec(GCM_TAG_LENGTH, data, 0 ,IV_LENGTH);
        cipher.init(Cipher.DECRYPT_MODE,secretKey,spec);

        int ciphertextLength = data.length - IV_LENGTH;
        byte[] text = cipher.doFinal(data, IV_LENGTH, ciphertextLength);

        return new String(text,StandardCharsets.UTF_8);
    }
}
