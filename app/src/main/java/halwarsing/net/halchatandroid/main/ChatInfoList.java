package halwarsing.net.halchatandroid.main;

import halwarsing.net.halchatandroid.type.HCMessage;

//Информация чата для списка
public class ChatInfoList {
    private String name;
    private HalChatGroupChats.HCEncryptedMessage lastMessage;
    private String icon;
    private String time;
    private long uid;
    private int secTime;

    public ChatInfoList(long uid, String name, HalChatGroupChats.HCEncryptedMessage lastMessage, String icon, String time, int secTime) {
        this.uid=uid;
        this.name=name;
        this.lastMessage=lastMessage;
        this.icon=icon;
        this.time=time;
        this.secTime=secTime;
    }

    public String getName() {
        return name;
    }

    public String getIcon() {
        return icon;
    }

    public HalChatGroupChats.HCEncryptedMessage getLastMessage() {
        return lastMessage;
    }

    public String getTime() {
        return time;
    }

    public long getUid() {
        return uid;
    }

    public long getSecTime(){return secTime;}
}
