package halwarsing.net.halchatandroid.main;

import android.app.Application;

import org.bouncycastle.jce.provider.BouncyCastleProvider;

import java.security.Security;

//Процесс с главным классом
public class HalChatApp extends Application {
    private HalChat hc;
    @Override
    public void onCreate() {
        super.onCreate();
        Security.addProvider(new BouncyCastleProvider());
        hc=new HalChat();
        hc.init(this.getApplicationContext());
    }

    public HalChat getHalChat() {
        return hc;
    }
}
