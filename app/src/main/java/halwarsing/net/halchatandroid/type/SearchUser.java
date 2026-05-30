package halwarsing.net.halchatandroid.type;

import android.graphics.Bitmap;

public class SearchUser {
    public HNUser user;
    public Bitmap icon;

    public SearchUser(HNUser user, Bitmap icon) {
        this.user=user;
        this.icon=icon;
    }
}
