package halwarsing.net.halchatandroid.main;

public class UpdateChatSetting {
    protected String name;
    protected String valueStr;
    protected int valueInt;

    protected UpdateChatSetting(String name) {
        this.name=name;
    }

    protected void setValue(String value) {
        this.valueStr=value;
    }

    protected void setValue(int value) {
        this.valueInt=value;
    }
}
