package halwarsing.net.halchatandroid.type;

public class HCFile {
    public String id,name;
    public int icon;
    public boolean disabled;
    public HCFile(String id,int icon,String name) {
        this.id=id;
        this.icon=icon;
        this.name=name;
        this.disabled=false;
    }
}
