package halwarsing.net.halchatandroid.type;

public class EmojiPack {
    public long uid;
    public long packId;
    public String icon;
    public String name;

    public EmojiPack(long uid, long packId, String icon, String name) {
        this.uid=uid;
        this.packId=packId;
        this.icon=icon;
        this.name=name;
    }
}
