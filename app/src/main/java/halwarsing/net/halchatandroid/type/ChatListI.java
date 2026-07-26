package halwarsing.net.halchatandroid.type;

public interface ChatListI {
    void onNewChat(HCChat chat);
    void onNewMessage(HCChat chat, HCMessage message);
    void onDeleteChat(HCChat chat);
    void onEnterChat(HCChat chat);
    default void onChatUpdated(HCChat chat) {}

}
