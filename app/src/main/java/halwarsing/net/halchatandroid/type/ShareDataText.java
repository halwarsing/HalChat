package halwarsing.net.halchatandroid.type;

public class ShareDataText extends ShareData {
    private String data;
    public ShareDataText(String text) {
        this.data=text;
    }

    public String getData() {
        return data;
    }
}
