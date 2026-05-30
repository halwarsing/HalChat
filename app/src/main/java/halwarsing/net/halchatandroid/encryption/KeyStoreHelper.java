package halwarsing.net.halchatandroid.encryption;

import android.security.keystore.KeyGenParameterSpec;
import android.security.keystore.KeyProperties;

import java.security.KeyStore;
import java.security.PrivateKey;
import java.security.PublicKey;

import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;

public class KeyStoreHelper {
    private static final String KEY_ALIAS="HalChat-";

    public static void createKey(long chatId) throws Exception {
        KeyGenerator keyGenerator=KeyGenerator.getInstance(
                KeyProperties.KEY_ALGORITHM_AES,
                "AndroidKeyStore"
        );

        KeyGenParameterSpec keyGenParameterSpec=new KeyGenParameterSpec.Builder(
                KEY_ALIAS+chatId,
                KeyProperties.PURPOSE_ENCRYPT|KeyProperties.PURPOSE_DECRYPT
        )
                .setBlockModes(KeyProperties.BLOCK_MODE_GCM)
                .setEncryptionPaddings(KeyProperties.ENCRYPTION_PADDING_NONE)
                .setRandomizedEncryptionRequired(true)
                .build();

        keyGenerator.init(keyGenParameterSpec);
        keyGenerator.generateKey();
    }

    public static SecretKey getSecretKey(long chatId) throws Exception {
        KeyStore keyStore=KeyStore.getInstance("AndroidKeyStore");
        keyStore.load(null);
        SecretKey secretKey=(SecretKey) keyStore.getKey(KEY_ALIAS+chatId,null);
        if(secretKey==null) {
            createKey(chatId);
            return getSecretKey(chatId);
        }
        return secretKey;
    }
}
