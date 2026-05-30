package halwarsing.net.halchatandroid.type;

public class HDFile {
    public long uid;
    public String id,path,name;
    public long fromId,updated;
    public boolean isFolder;

    public HDFile(long uid,String id,String path,String name,long fromId,long updated,boolean isFolder) {
        this.uid=uid;
        this.id=id;
        this.path=path;
        this.name=name;
        this.fromId=fromId;
        this.updated=updated;
        this.isFolder=isFolder;
    }
}
