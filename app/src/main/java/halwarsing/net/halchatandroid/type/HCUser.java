package halwarsing.net.halchatandroid.type;

public class HCUser {
    public long uid,id,chatId,toId;
    public byte permissions;
    public boolean isJoin;
    public HNUser user;

    public HCUser(long uid,long id,long chatId,long toId,byte permissions,boolean isJoin,HNUser user) {
        this.uid=uid;
        this.id=id;
        this.chatId=chatId;
        this.toId=toId;
        this.permissions=permissions;
        this.isJoin=isJoin;
        this.user=user;
    }
}
