package halwarsing.net.halchatandroid.type;

public interface HalChatI {
    boolean onNewMessage(HCMessage message);
    void onLoadMessage(HCMessage message);
    void onDeleteMessage(long id);
    void onEditMessage(HCMessage message);
}
