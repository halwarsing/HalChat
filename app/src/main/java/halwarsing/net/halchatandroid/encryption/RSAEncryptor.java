package halwarsing.net.halchatandroid.encryption;

import android.util.Base64;

import java.nio.charset.StandardCharsets;
import java.security.KeyFactory;
import java.security.PublicKey;
import java.security.spec.MGF1ParameterSpec;
import java.security.spec.X509EncodedKeySpec;

import javax.crypto.Cipher;
import javax.crypto.spec.OAEPParameterSpec;
import javax.crypto.spec.PSource;

//Шифрование паролей для последующей отправки в оффлайн хранилище.
public class RSAEncryptor {
    private static final String PEM_PUBLIC_KEY="-----BEGIN PUBLIC KEY-----\n" +
            "MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA1WGYzLDQTDIN7dq3fyDV\n" +
            "FxfMw1zlRti2OIUltu4Zeieosa7LkU1aH5++U3G/UoTgbqQTqjcuvO/w+MEB0bIF\n" +
            "Y4zzwi2JJa/r3DCPaFhaNRSvXcW52TM21FM6CgupW57gm0mD8LO5yrnKKvYtC/6f\n" +
            "hiyybNmPKfFhXIRWsIwYACDp5HATJCY7sSA8BwfF8nM62o1kaZEMP2dhajxZD6Fi\n" +
            "st2lg4LM0GZWiPBNgQ6GTGdqGmIS7rPhJkk5fsLvKcZ5Y8uLKqx+CiP4aZ5B0vvb\n" +
            "g32lNOt6UxxdR4fNT42c8Bmiw6rs3CUg04My3hW/UVGD9lCsRzUNaJwVGaC5gQ7v\n" +
            "VqrMAbV7viZeW01kD1NWrAdXBzWm1oWIubG0OqFVFMSW2LCDL9llosaJtZgLllrf\n" +
            "nrptQg4Rkt+lONwE10ztlaJ6pt/Y4swxigyesphaNgJQc8BtG0/ak44okilcF75T\n" +
            "csWL8yMR3NuxP2iezEolNYTdtVuZ3qBuO/SNWqF+a9gk40ukdatfXwiaRHtFDGmC\n" +
            "/c0PeMjKCblDE+b/JdqUofFPwFNZS7vs8CPjUUrVH8OFH7s5Cwa2pwh9N2cjUiAw\n" +
            "QkU1PtJKKGwsb2aSiGUFVqWd6y1UsvZutpHcLr7fRECqhv8FrhEU+Gea2Bz6/wV7\n" +
            "FgAbNb54PwzU+eV33ESPcLsCAwEAAQ==\n" +
            "-----END PUBLIC KEY-----";

    public static String encryptPassword(String password) {
        try {
            String pemContents = PEM_PUBLIC_KEY
                    .replace("-----BEGIN PUBLIC KEY-----", "")
                    .replace("-----END PUBLIC KEY-----", "")
                    .replaceAll("\\s+", "");

            byte[] binaryDer = Base64.decode(pemContents, Base64.DEFAULT);

            X509EncodedKeySpec keySpec = new X509EncodedKeySpec(binaryDer);
            KeyFactory keyFactory = KeyFactory.getInstance("RSA");
            PublicKey publicKey = keyFactory.generatePublic(keySpec);

            Cipher cipher = Cipher.getInstance("RSA/ECB/OAEPPadding");
            OAEPParameterSpec oaepParams = new OAEPParameterSpec(
                    "SHA-256",
                    "MGF1",
                    MGF1ParameterSpec.SHA256,
                    PSource.PSpecified.DEFAULT
            );
            cipher.init(Cipher.ENCRYPT_MODE, publicKey, oaepParams);

            byte[] encryptedBytes = cipher.doFinal(password.getBytes(StandardCharsets.UTF_8));

            return Base64.encodeToString(encryptedBytes, Base64.NO_WRAP);

        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }
}
