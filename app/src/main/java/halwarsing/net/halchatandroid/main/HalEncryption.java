package halwarsing.net.halchatandroid.main;

import android.util.Log;

//Класс шифрования Халварсинга
public class HalEncryption {
    private static final HalHash hh=new HalHash();
    private static final String version="0.0.2";
    private static final String TAG="HE";

    //hex
    public static byte hexCharsToByte(char c1,char c2) {
        int high=Character.digit(c1,16);
        int low=Character.digit(c2,16);
        return (byte)((high<<4) + low);
    }

    public byte shift_forward(byte a,byte b) {
        int o=a+b;
        return (byte)((o>255)?o-256:o);
    }

    public byte shift_back(byte a,byte b) {
        int o=a-b;
        return (byte)((o<0)?o+256:o);
    }

    public byte[] encode(byte[] data,String passw,int hashLength,int hashCount,long maxIteration,String secretData) {return this.encodeByHash(data,hh.Str2Hash(passw,hashLength,hashCount,maxIteration),hashCount,maxIteration,secretData);}

    public byte[] encode(byte[] data,String passw,int hashLength) {return encode(data,passw,hashLength,64,100000,"");}

    public byte[] encode(byte[] data,String passw) {return encode(data,passw,256,64,100000,"");}

    public byte[] encodeByHash(byte[] data,String passw,int hashCount,long maxIteration,String secretData) {
        byte[] out=new byte[data.length];
        int a=0;
        int b=passw.length()-1;
        String newPassw=hh.Str2Hash(passw+passw+secretData,passw.length(),hashCount,maxIteration);
        for (int i=0;i<data.length;i++) {
            out[i] = shift_forward(data[i], hexCharsToByte(newPassw.charAt(a),newPassw.charAt(a+1)));
            a++;
            if (a>=b){newPassw=hh.Str2Hash(passw+newPassw,passw.length(),hashCount,maxIteration);a=0;}
        }
        return out;
    }

    public byte[] encodeByHash(byte[] data,String passw){return encodeByHash(data,passw,64,100000,"");}

    public byte[] decode(byte[] data,String passw,int hashLength,int hashCount,long maxIteration,String secretData) {return decodeByHash(data,hh.Str2Hash(passw,hashLength,hashCount,maxIteration),hashCount,maxIteration,secretData);}

    public byte[] decode(byte[] data,String passw,int hashLength){return decode(data,passw,hashLength,64,100000,"");}

    public byte[] decode(byte[] data,String passw){return decode(data,passw,256,64,100000,"");}

    public byte[] decodeByHash(byte[] data,String passw,int hashCount,long maxIteration,String secretData) {
        byte[] out=new byte[data.length];
        int a=0;
        int b=passw.length()-1;
        String newPassw=hh.Str2Hash(passw+passw+secretData,passw.length(),hashCount,maxIteration);
        for (int i=0;i<data.length;i++) {
            out[i]=this.shift_back(data[i],hexCharsToByte(newPassw.charAt(a),newPassw.charAt(a+1)));
            a++;
            if (a>=b){newPassw=hh.Str2Hash(passw+newPassw,passw.length(),hashCount,maxIteration);a=0;}
        }
        return out;
    }

    public byte[] decodeByHash(byte[] data,String passw,int hashCount) {
        return decodeByHash(data,passw,hashCount,100000,"");
    }

    public byte[] decodeByHash(byte[] data,String passw) {return decodeByHash(data,passw,64,100000,"");}
}
