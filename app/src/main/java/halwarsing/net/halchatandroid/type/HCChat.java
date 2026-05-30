package halwarsing.net.halchatandroid.type;

//Класс чата (вся информация)
public class HCChat {
    public long uid,chatUID,created;
    public String id,name,icon,password;
    public int publicType,chatType;
    public boolean fromMe,isAllowMessages,isDelete,isAllowComments;
    public HCChat(long uid,long chatUID,long created,int publicType,int chatType,String id,String name,String icon,String password,boolean fromMe,boolean isAllowMessages,boolean isDelete,boolean isAllowComments) {
        this.uid=uid;
        this.chatUID=chatUID;
        this.created=created;
        this.publicType=publicType;
        this.chatType=chatType;
        this.id=id;
        this.name=name;
        this.icon=icon;
        this.password=password;
        this.fromMe=fromMe;
        this.isAllowMessages=isAllowMessages;
        this.isDelete=isDelete;
        this.isAllowComments=isAllowComments;
    }
}
