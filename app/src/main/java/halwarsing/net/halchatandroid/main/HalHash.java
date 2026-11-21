package halwarsing.net.halchatandroid.main;

import java.nio.charset.StandardCharsets;

//Класс хэширования Халварсинга, Bin2Hash считается в HalChatCore/HalHash.c, результат равен HalHash.js
public class HalHash {
    private static final byte[] HEX_ARRAY = "0123456789abcdef".getBytes(StandardCharsets.US_ASCII);
    private static final String version="0.0.2C";
    private static final String TAG="HH";
    //0.0.2B - optimized for Android;
    //0.0.2C - HalHash.c через JNI (hal_chatcore), Bin2Hex в нижнем регистре как в JS

    static {
        System.loadLibrary("hal_chatcore");
    }

    private static native String nativeBin2Hash(byte[] bytes,int length,int count,long maxIteration);

    public String Str2Hash(String str,int length,int count,long maxIteration) {
        return this.Bin2Hash(str.getBytes(StandardCharsets.UTF_8),length,count,maxIteration);
    }

    public String Str2Hash(String str,int length,int count) {
        return Str2Hash(str,length,count,100000);
    }

    public String Str2Hash(String str,int length) {
        return Str2Hash(str,length,64,100000);
    }

    public String Bin2Hex(byte[] bytes) {
        byte[] hexChars = new byte[bytes.length * 2];
        for (int j = 0; j < bytes.length; j++) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = HEX_ARRAY[v >>> 4];
            hexChars[j * 2 + 1] = HEX_ARRAY[v & 0x0F];
        }
        return new String(hexChars, StandardCharsets.UTF_8);
    }

    /**
     * Хэш байтов через HalHash_Bin2Hash
     *
     * @param bytes Входные байты
     * @param length Длина хэша в hex-символах
     * @param count Количество кусков
     * @param maxIteration Предел итераций подгонки длины
     * @return Хэш (hex в нижнем регистре)
     */
    public String Bin2Hash(byte[] bytes,int length,int count,long maxIteration) {
        //В C эти параметры беззнаковые: отрицательные стали бы огромными числами
        if(length<0||count<0||maxIteration<0)throw new IllegalArgumentException("length, count и maxIteration не могут быть отрицательными");
        return nativeBin2Hash(bytes,length,count,maxIteration);
    }

    public String Bin2Hash(byte[] bytes,int length) {
        return Bin2Hash(bytes,length,64,100000);
    }
}
