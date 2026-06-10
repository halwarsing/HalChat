package halwarsing.net.halchatandroid.main;

import android.util.Log;

import androidx.annotation.NonNull;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import okhttp3.Dns;

public class WhitelistDNS implements Dns {
    private static final String TAG="WLDNS";
    //Домены экосистемы HalNet
    private static final List<String> hndomains = Arrays.asList("halwarsing.net", "halchat.halwarsing.net", "halch.at", "haldrive.halwarsing.net", "voice.halch.at", "voice.halwarsing.net");
    private static boolean isWLEnabled = false;
    private static long lastCheckTime=0;
    private static final long CACHE_TTL_MS = 60000; //задежка 60с

    public static synchronized void invalidateCache() {
        lastCheckTime=0;
        Log.e(TAG,"Connection Error: DNS Cache reset");
    }

    protected static synchronized boolean isWhiteListActive() {
        long currentTime = System.currentTimeMillis();

        if (currentTime - lastCheckTime < CACHE_TTL_MS) {
            return isWLEnabled;
        }

        int timeout = 2000;

        lastCheckTime=currentTime;

        //Доступен ли основной сервер?
        boolean isMainServerUp = isTcpPortOpen("halwarsing.net", 443, timeout);
        if (isMainServerUp) {
            isWLEnabled=false;
            return false;
        }

        //Работает ли ip?
        isWLEnabled=isTcpPortOpen("158.160.241.255", 443, timeout);
        return isWLEnabled;
    }

    private static boolean isTcpPortOpen(String host, int port, int timeoutMs) {
        try (Socket socket = new Socket()) {
            socket.connect(new InetSocketAddress(host, port), timeoutMs);
            return true;
        } catch (IOException e) {
            return false;
        }
    }

    @NonNull
    @Override
    public List<InetAddress> lookup(@NonNull String hostname) throws UnknownHostException {
        if (hndomains.contains(hostname) && isWhiteListActive()) {
            Log.e(TAG,"ENABLE Z");
            return Arrays.asList(InetAddress.getByName("158.160.241.255"));
        }
        return Dns.SYSTEM.lookup(hostname);
    }
}
