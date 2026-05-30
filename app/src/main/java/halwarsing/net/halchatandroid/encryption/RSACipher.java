package halwarsing.net.halchatandroid.encryption;

import static android.content.Context.MODE_PRIVATE;

import android.content.Context;
import android.content.SharedPreferences;
import android.security.keystore.KeyGenParameterSpec;
import android.security.keystore.KeyProperties;
import android.util.Base64;
import android.util.Log;

import java.nio.charset.StandardCharsets;
import java.security.KeyFactory;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.KeyStore;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.spec.MGF1ParameterSpec;
import java.security.spec.PKCS8EncodedKeySpec;
import java.security.spec.RSAKeyGenParameterSpec;
import java.security.spec.X509EncodedKeySpec;

import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.spec.OAEPParameterSpec;
import javax.crypto.spec.PSource;

/**
 * Удобный класс для работы с RSA/OAEP-SHA256 через AndroidKeyStore.
 * Позволяет генерировать ключи, шифровать и дешифровать строки.
 */
public class RSACipher {
    private static final String TAG = "RSACipher";
    //SHA1 only
    //private static final String ANDROID_KEYSTORE = "AndroidKeyStore";
    //private static final String ANDROID_KEYSTORE = "BC";
    private static final String KEY_ALIAS_PREFIX = "HalChatRSA-";
    //private static final String TRANSFORMATION = "RSA/ECB/OAEPWithSHA-256AndMGF1Padding";
    private static final String TRANSFORMATION = "RSA/ECB/OAEPPadding";
    private static final String sharedname="halchatrsa";
    private Context context;

    // Параметры OAEP с SHA-256 и MGF1
    private static final OAEPParameterSpec OAEP_PARAMS = new OAEPParameterSpec(
            "SHA-256", // message digest
            "MGF1",    // mask generation function
            new MGF1ParameterSpec("SHA-256"),
            PSource.PSpecified.DEFAULT
    );


    private PublicKey publicKey;
    private PrivateKey privateKey;

    public PrivateKey getPrivateKey() {
        return privateKey;
    }

    /**
     * Удаляет ключи из AndroidKeyStore.
     */
    public static void deletePrivateKeyRSA(Context context, long chatUID) throws Exception {
        /*KeyStore ks = KeyStore.getInstance(ANDROID_KEYSTORE);
        ks.load(null);
        ks.deleteEntry(KEY_ALIAS_PREFIX + chatUID);*/
        SharedPreferences ks=context.getSharedPreferences(sharedname,MODE_PRIVATE);
        SharedPreferences.Editor edit=ks.edit();
        edit.remove(String.valueOf(chatUID));
        edit.apply();
    }

    /**
     * Проверяет наличие приватного ключа в AndroidKeyStore.
     */
    public static boolean hasPrivateKeyRSA(Context context,long chatUID) throws Exception {
        /*KeyStore ks = KeyStore.getInstance(ANDROID_KEYSTORE);
        ks.load(null);
        return ks.containsAlias(KEY_ALIAS_PREFIX + chatUID);*/
        SharedPreferences ks=context.getSharedPreferences(sharedname,MODE_PRIVATE);
        return ks.contains(String.valueOf(chatUID));
    }

    /**
     * Конструктор: генерирует ключи и загружает их.
     */
    public RSACipher(Context context,long chatUID) throws Exception {
        this.context=context;
        /*KeyStore ks = KeyStore.getInstance(ANDROID_KEYSTORE);
        ks.load(null);
        String alias = KEY_ALIAS_PREFIX + chatUID;

        // Если ключа ещё нет — генерируем
        if (!ks.containsAlias(alias)) {
            KeyPairGenerator kpg = KeyPairGenerator.getInstance(
                    KeyProperties.KEY_ALGORITHM_RSA, ANDROID_KEYSTORE
            );
            KeyGenParameterSpec spec = new KeyGenParameterSpec.Builder(
                    alias,
                    KeyProperties.PURPOSE_ENCRYPT | KeyProperties.PURPOSE_DECRYPT
            )
                    .setKeySize(2048)
                    .setDigests(KeyProperties.DIGEST_SHA256)
                    .setEncryptionPaddings(KeyProperties.ENCRYPTION_PADDING_RSA_OAEP)
                    .build();
            kpg.initialize(spec);
            kpg.generateKeyPair();
        }

        // Теперь безопасно загружаем пару
        KeyStore.PrivateKeyEntry entry =
                (KeyStore.PrivateKeyEntry) ks.getEntry(alias, null);
        this.privateKey = entry.getPrivateKey();
        // PublicKey можно взять из самоподписанного сертификата
        this.publicKey  = entry.getCertificate().getPublicKey();*/

        SharedPreferences ks=context.getSharedPreferences(sharedname,MODE_PRIVATE);
        if(!ks.contains(String.valueOf(chatUID))) {
            KeyPairGenerator kpg = KeyPairGenerator.getInstance(
                    KeyProperties.KEY_ALGORITHM_RSA
            );


            kpg.initialize(new RSAKeyGenParameterSpec(2048,RSAKeyGenParameterSpec.F4));
            KeyPair kp=kpg.generateKeyPair();

            this.publicKey=kp.getPublic();
            this.privateKey=kp.getPrivate();

            SharedPreferences.Editor edit=ks.edit();
            edit.putString(String.valueOf(chatUID),Base64.encodeToString(kp.getPrivate().getEncoded(),Base64.NO_WRAP));
            edit.apply();
        } else {
            KeyFactory kf   = KeyFactory.getInstance("RSA");
            this.privateKey= kf.generatePrivate(new PKCS8EncodedKeySpec(Base64.decode(ks.getString(String.valueOf(chatUID),""),Base64.NO_WRAP)));
        }
    }


    /**
     * Конструктор с готовыми ключами.
     */
    public RSACipher(PublicKey publicKey, PrivateKey privateKey) {
        this.publicKey = publicKey;
        this.privateKey = privateKey;
    }

    /**
     * Шифрует строку plain и возвращает результат в Base64 без переносов.
     */
    public String encrypt(String plain) throws Exception {
        Cipher cipher = Cipher.getInstance(TRANSFORMATION);
        cipher.init(Cipher.ENCRYPT_MODE, publicKey, OAEP_PARAMS);
        byte[] ciphertext = cipher.doFinal(plain.getBytes(StandardCharsets.UTF_8));
        return Base64.encodeToString(ciphertext, Base64.NO_WRAP);
    }

    /**
     * Дешифрует Base64-шифротекст и возвращает исходную строку.
     * Использует приватный ключ из AndroidKeyStore без явного указания OAEP параметров.
     */
    public String decrypt(String base64Cipher) throws Exception {
        byte[] ct = Base64.decode(base64Cipher, Base64.NO_WRAP);
        Cipher c = Cipher.getInstance(TRANSFORMATION);
        c.init(Cipher.DECRYPT_MODE, privateKey,OAEP_PARAMS);
        return new String(c.doFinal(ct), StandardCharsets.UTF_8);
    }

    public PublicKey getPublicKey() {
        return publicKey;
    }

    /**
     * Возвращает публичный ключ в формате Base64 (без переносов).
     */
    public String getPublicKeyBase64() {
        return Base64.encodeToString(publicKey.getEncoded(), Base64.NO_WRAP);
    }

    /**
     * Возвращает публичный ключ в PEM формате (PKCS#8).
     */
    public String getPublicKeyPem() {
        String b64 = getPublicKeyBase64();
        return "-----BEGIN PUBLIC KEY-----\n" + chunkString(b64, 64) + "-----END PUBLIC KEY-----";
    }

    /**
     * Импорт публичного ключа из строки Base64 или PEM.
     */
    public static PublicKey stringToPublicKey(String keyStr) {
        try {
            String pem = keyStr
                    .replace("-----BEGIN PUBLIC KEY-----", "")
                    .replace("-----END PUBLIC KEY-----", "")
                    .replaceAll("\\s+", "");
            byte[] keyBytes = Base64.decode(pem, Base64.DEFAULT);
            X509EncodedKeySpec spec = new X509EncodedKeySpec(keyBytes);
            return KeyFactory.getInstance("RSA").generatePublic(spec);
        } catch (Exception e) {
            Log.e(TAG, "Invalid public key format", e);
            return null;
        }
    }

    // Помогает разбить строку на строки по указанной длине
    private String chunkString(String str, int chunkSize) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < str.length(); i += chunkSize) {
            sb.append(str, i, Math.min(str.length(), i + chunkSize));
            sb.append("\n");
        }
        return sb.toString();
    }
}
