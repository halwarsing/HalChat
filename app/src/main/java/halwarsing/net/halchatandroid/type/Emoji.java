package halwarsing.net.halchatandroid.type;

public class Emoji {
    public long uid;
    public long emojiId;
    public long fromPack;
    public String image;
    public String image64;
    public String value;

    public Emoji(long uid,long emojiId,long fromPack,String image,String image64,String value) {
        this.uid=uid;
        this.emojiId=emojiId;
        this.fromPack=fromPack;
        this.image=image;
        this.image64=image64;
        this.value=value;
    }
}
