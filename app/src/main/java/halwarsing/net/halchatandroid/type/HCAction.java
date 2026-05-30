package halwarsing.net.halchatandroid.type;

import android.util.Log;

import org.json.JSONException;
import org.json.JSONObject;

public class HCAction {
    private static final String TAG="HCAction";
    public long uid,fromId,fromMsg,fromChat,toId,time;
    public int type;
    public String data;
    public boolean isEnd;
    public JSONObject jdata=null;
    public HCMessage newmsg=null;

    public HCAction(long uid,int type,long fromId,long fromMsg,long fromChat,long toId,long time,String data,boolean isEnd,JSONObject jdata) {
        this.uid=uid;
        this.type=type;
        this.fromId=fromId;
        this.fromMsg=fromMsg;
        this.fromChat=fromChat;
        this.toId=toId;
        this.time=time;
        this.data=data;
        this.isEnd=isEnd;
        this.jdata=jdata;
    }

    public HCAction(long uid,int type,long fromId,long fromMsg,long fromChat,long toId,long time,String data,boolean isEnd) {
        this.uid=uid;
        this.type=type;
        this.fromId=fromId;
        this.fromMsg=fromMsg;
        this.fromChat=fromChat;
        this.toId=toId;
        this.time=time;
        this.data=data;
        this.isEnd=isEnd;
    }

    public static HCAction getFromJSON(JSONObject jdata) {
        long uid,fromId,fromMsg,fromChat,toId,time;
        int type;
        String data;
        boolean isEnd;
        HCMessage newmsg;

        try {
            uid = jdata.getLong("uid");
            fromId = jdata.getLong("fromId");
            fromMsg = jdata.getLong("fromMsg");
            fromChat = jdata.getLong("fromChat");
            toId = jdata.getLong("toId");
            time = jdata.getLong("time");
            type = jdata.getInt("type");
            data=jdata.getString("data");
            isEnd = jdata.getInt("isEnd") == 1;
            return new HCAction(uid, type, fromId, fromMsg, fromChat, toId, time, data, isEnd, jdata);
        } catch (JSONException e) {
            Log.e(TAG,"getFromJSON",e);
        }
        return null;
    }

    public static HCAction getFromJSONAPI(JSONObject jdata) {

        long uid,fromId,fromMsg,fromChat,toId,time;
        int type;
        String data;
        boolean isEnd;
        HCMessage newmsg;

        try {
            uid = jdata.getLong("uid");
            fromId = jdata.getLong("fromId");
            fromMsg = jdata.getLong("fromIdMsg");
            fromChat = jdata.getLong("fromIdChat");
            toId = jdata.getLong("toId");
            time = jdata.getLong("time");
            type = jdata.getInt("type");
            data=jdata.getString("data");
            isEnd = jdata.getInt("isEnd") == 1;
            return new HCAction(uid, type, fromId, fromMsg, fromChat, toId, time, data, isEnd, jdata);
        } catch (JSONException e) {
            Log.e(TAG,"getFromJSON",e);
        }
        return null;
    }

}
