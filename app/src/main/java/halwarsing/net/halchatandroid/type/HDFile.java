package halwarsing.net.halchatandroid.type;

import org.json.JSONException;
import org.json.JSONObject;

public class HDFile {
    public long uid;
    public String id,path,name;
    public long fromId,updated;
    public boolean isFolder;
    public String mimeType,fileType;
    public JSONObject mimeData;

    public HDFile(long uid,String id,String path,String name,long fromId,long updated,boolean isFolder,String mimeType,String fileType,String mimeData) throws JSONException {
        this.uid=uid;
        this.id=id;
        this.path=path;
        this.name=name;
        this.fromId=fromId;
        this.updated=updated;
        this.isFolder=isFolder;
        this.mimeType=mimeType;
        this.fileType=fileType;
        this.mimeData=mimeData!=null&&mimeData.startsWith("{")&&mimeData.endsWith("}")?new JSONObject(mimeData):null;
    }
}
