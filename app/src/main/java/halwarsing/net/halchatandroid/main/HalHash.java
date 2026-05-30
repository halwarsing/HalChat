package halwarsing.net.halchatandroid.main;

import android.util.Log;

import java.math.BigInteger;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Objects;

//Класс хэширования Халварсинга
public class HalHash {
    private static final byte[] HEX_ARRAY = "0123456789ABCDEF".getBytes(StandardCharsets.US_ASCII);
    private static final String version="0.0.2B";
    private static final String TAG="HH";
    private static final BigInteger TWO=BigInteger.valueOf(2L);
    private static final double LOG2_10 = 0.3010299956639812;
    //0.0.2B - optimized for Android;

    private static final BigInteger[] TEN_POW_CACHE = new BigInteger[500];

    static {
        TEN_POW_CACHE[0]=BigInteger.ONE;
        for(int i=1;i< TEN_POW_CACHE.length;i++) {
            TEN_POW_CACHE[i]=TEN_POW_CACHE[i-1].multiply(BigInteger.TEN);
        }
    }

    public static BigInteger getTenPow(int exp) {
        if(exp>=0&&exp< TEN_POW_CACHE.length) return TEN_POW_CACHE[exp];
        return BigInteger.TEN.pow(exp);
    }

    public static int getDecimalLengthFast(BigInteger number) {
        if(number.equals(BigInteger.ZERO))return 1;
        int digits=(int)(number.bitLength()*LOG2_10)+1;

        if(number.compareTo(getTenPow(digits-1))<0) {
            digits--;
        }
        return digits;
    }


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

    public String array_toHex(List<BigInteger> arr) {
        StringBuilder out=new StringBuilder();
        for (int i=0;i<arr.size();i++) {
            out.append(arr.get(i).toString(16));
        }
        return out.toString();
    }

    public static int calculateHexLength(List<BigInteger> arr) {
        int totalLength = 0;

        for (BigInteger num : arr) {
            if (num.equals(BigInteger.ZERO)) {
                totalLength += 1;
            } else {
                totalLength+=(num.bitLength()+3)/4;
            }
        }

        return totalLength;
    }

    public static int getDecimalLength(BigInteger number) {
        BigInteger ten = BigInteger.TEN;
        int length = 1;

        while (number.compareTo(ten) >= 0) {
            number = number.divide(ten);
            length++;
        }

        return length;
    }

    public BigInteger[] getHalfInt(BigInteger i) {

        if (i.equals(BigInteger.ZERO)) {
            return new BigInteger[]{BigInteger.ZERO, BigInteger.ZERO};
        }

        int digits=getDecimalLengthFast(i);
        int halfDigits = (digits+1) / 2;

        BigInteger divisor = getTenPow(halfDigits);
        return i.divideAndRemainder(divisor);
    }

    public String Bin2Hash(byte[] bytes,int length,int count,long maxIteration) {
        count= Math.min(length, count);
        count= Math.min(bytes.length, count);
        int sizeD=bytes.length/count;
        List<BigInteger> d=new ArrayList<>();
        BigInteger tempi,tempb;
        int i;
        BigInteger g=BigInteger.valueOf(count).multiply(BigInteger.valueOf(length));

        for (i = 0; i < count; i++) {
            BigInteger segment = new BigInteger(1, Arrays.copyOfRange(bytes, i * sizeD, (i + 1) * sizeD));
            d.add(segment.multiply(g));
        }
        if (bytes.length % count > 0) {
            int start = count * sizeD - sizeD;
            BigInteger remainingSegment = new BigInteger(1, Arrays.copyOfRange(bytes, start, bytes.length));
            d.set(count - 1, remainingSegment.multiply(g));
        }

        int p=(length-count)/2;
        int b;
        for (i=0;i<p;i++) {
            d.add(BigInteger.valueOf(i+2));
            tempi=g.multiply(BigInteger.valueOf(i+1));
            for (b=0;b<count;b++) {
                d.set(d.size()-1,d.get(b).multiply(d.get(d.size()-1)).add(tempi));
            }
        }
        count=d.size();
        List<BigInteger> tempa=new ArrayList<>(d);

        BigInteger[] halfs0,halfs1;
        halfs0=this.getHalfInt(g);
        g=halfs0[0].add(halfs0[1]);

        for (i=0;i<count;i++) {
            tempb=tempa.get(i);
            tempi=tempb.multiply(tempb).add(tempb.multiply(g));
            for (b=0;b<count;b++) {
                d.set(b,d.get(b).add(tempi));
            }
        }

        int lc=count-1;
        long mc=0;
        int hl;

        while (true) {
            hl=HalHash.calculateHexLength(d);
            if (hl < length) {
                for (i = 0; i < count; i++) {
                    if (i == lc) {
                        tempi=d.get(i);
                        d.set(i, tempi.multiply(TWO).add(g));
                    } else {
                        tempi=d.get(i);
                        tempb=d.get(i+1);
                        halfs0=this.getHalfInt(tempi);
                        halfs1=this.getHalfInt(tempb);

                        BigInteger valV=halfs0[0].add(halfs0[1]);
                        BigInteger valZ=halfs1[0].add(halfs1[1]);

                        int lenV=getDecimalLengthFast(valV);
                        int lenZ=getDecimalLengthFast(valZ);

                        BigInteger concatVZ=valV.multiply(getTenPow(lenZ)).add(valZ);
                        BigInteger concatZV=valZ.multiply(getTenPow(lenV)).add(valV);

                        d.set(i,concatVZ);
                        d.set(i+1,concatZV);
                    }
                }
            } else if (hl > length) {
                if (mc > maxIteration) {
                    return this.array_toHex(d).substring(0, length);
                }
                for (i = 0; i < count; i++) {
                    if (i == lc) {
                        tempi=d.get(i);
                        halfs0=this.getHalfInt(tempi);
                        d.set(i, halfs0[0].add(halfs0[1]).add(g));
                    } else {
                        tempi=d.get(i);
                        tempb=d.get(i+1);
                        halfs0=this.getHalfInt(tempi);
                        halfs1=this.getHalfInt(tempb);
                        d.set(i, halfs0[0].add(halfs1[1]));
                        d.set(i + 1, halfs1[0].add(halfs0[1]));
                    }
                }
            } else {
                break;
            }
            mc++;
        }

        return array_toHex(d);
    }

    public String Bin2Hash(byte[] bytes,int length) {
        return Bin2Hash(bytes,length,64,100000);
    }
}
