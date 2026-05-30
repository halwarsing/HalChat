package halwarsing.net.halchatandroid.type;

public class Pixel {
    public long uid;
    public long pixelId;
    public String value;
    public long fromPack;
    public String image;

    public Pixel(long uid, long pixelId, String value, long fromPack, String image) {
        this.uid=uid;
        this.pixelId=pixelId;
        this.value=value;
        this.fromPack=fromPack;
        this.image=image;
    }
}
