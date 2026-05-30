package halwarsing.net.halchatandroid.encryption;


import java.security.SecureRandom;
import java.util.Arrays;

public class PasswordGenerator {

    // Исключили неоднозначные "O0oIl1|`"
    private static final char[] LOWER = filterAmbiguous("abcdefghijklmnopqrstuvwxyz");
    private static final char[] UPPER = filterAmbiguous("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    private static final char[] DIGIT = filterAmbiguous("0123456789");
    private static final char[] SYMS  = filterAmbiguous("!#$%&()*+,-./:;<=>?@[]^_{|}~");

    // Общий пул символов
    private static final char[] POOL;

    static {
        // Собираем общий пул на этапе инициализации класса
        String combined = new String(LOWER) + new String(UPPER) + new String(DIGIT) + new String(SYMS);
        POOL = combined.toCharArray();
    }

    // CSPRNG генератор (читает /dev/urandom на Android)
    private static final SecureRandom CRYPTO = new SecureRandom();

    /**
     * Фильтр для исключения неоднозначных символов
     */
    private static char[] filterAmbiguous(String source) {
        String ambig = "O0oIl1|`";
        StringBuilder sb = new StringBuilder();
        for (char c : source.toCharArray()) {
            if (ambig.indexOf(c) == -1) {
                sb.append(c);
            }
        }
        return sb.toString().toCharArray();
    }

    /**
     * Генерирует пароль в виде массива символов
     */
    public static char[] generatePassword(int length) {
        if (length < 4) throw new IllegalArgumentException("Длина должна быть минимум 4");

        char[] result = new char[length];

        // 1. Гарантируем минимум по 1 символу из каждого класса
        result[0] = LOWER[CRYPTO.nextInt(LOWER.length)];
        result[1] = UPPER[CRYPTO.nextInt(UPPER.length)];
        result[2] = DIGIT[CRYPTO.nextInt(DIGIT.length)];
        result[3] = SYMS[CRYPTO.nextInt(SYMS.length)];

        // 2. Остальные — из общего пула
        for (int i = 4; i < length; i++) {
            result[i] = POOL[CRYPTO.nextInt(POOL.length)];
        }

        // 3. Крипто-перетасовка Фишера–Йетса (Fisher-Yates)
        for (int i = result.length - 1; i > 0; i--) {
            int j = CRYPTO.nextInt(i + 1); // secureIndex из коробки
            // Swap
            char temp = result[i];
            result[i] = result[j];
            result[j] = temp;
        }

        return result;
    }

    /**
     * Метод для ручной очистки пароля из оперативной памяти после использования.
     */
    public static void clearPassword(char[] password) {
        if (password != null) {
            Arrays.fill(password, '\0');
        }
    }
}