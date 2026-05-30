package halwarsing.net.halchatandroid.type;

import android.graphics.Bitmap;

public class HNUser {
    public long uid, id;
    public String nickname,icon,fullPathIcon;
    public boolean isBot;
    public Bitmap iconImage;

    public HNUser(long uid,long id,String nickname,String icon,boolean isBot,String fullPathIcon) {
        this.uid=uid;
        this.id=id;
        this.nickname=nickname;
        this.icon=icon;
        this.isBot=isBot;
        this.fullPathIcon=fullPathIcon;
    }

    public void loadIcon(Bitmap iconImage) {
        this.iconImage=iconImage;
    }
}
