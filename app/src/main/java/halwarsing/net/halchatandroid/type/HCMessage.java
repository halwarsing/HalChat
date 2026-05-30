package halwarsing.net.halchatandroid.type;

import android.graphics.Bitmap;

import org.json.JSONArray;
import org.json.JSONObject;

//Класс сообщения (вся информация)
public class HCMessage {
    public long uid,msgId,chatId,fromId,time,answerMsg,commentMsg;
    public JSONArray attachments;
    public String message,soundMsg,dataBot,recordMic,decryptedMessage;
    public byte[] encryptId;
    public boolean isDelete,isSended,isReceived,isFrom,isDecrypted;
    public int type;
    public String icon;
    public boolean isHalEnc;
    public JSONObject data;
    public long shareId;
    public long pixelId;
    public boolean isPinned;
    public long v;

    public HCMessage(long uid, long msgId, long chatId, long fromId, long time, long answerMsg, long commentMsg, String message,
                     JSONArray attachments, String soundMsg, String dataBot, String recordMic, byte[] encryptId,
                     boolean isDelete, boolean isSended, boolean isReceived, int type, boolean isFrom,boolean isHalEnc,
                     JSONObject data, long shareId, long pixelId, boolean isPinned, long v) {
        this.uid=uid;
        this.msgId=msgId;
        this.chatId=chatId;
        this.fromId=fromId;
        this.time=time;
        this.answerMsg=answerMsg;
        this.commentMsg=commentMsg;
        this.message=message;
        this.attachments=attachments;
        this.soundMsg=soundMsg;
        this.dataBot=dataBot;
        this.recordMic=recordMic;
        this.encryptId=encryptId;
        this.isDelete=isDelete;
        this.isSended=isSended;
        this.isReceived=isReceived;
        this.type=type;
        this.isFrom=isFrom;
        this.decryptedMessage=message;
        this.isDecrypted=false;
        this.isHalEnc=isHalEnc;
        this.icon=null;
        this.data=data;
        this.shareId=shareId;
        this.pixelId=pixelId;
        this.isPinned=isPinned;
        this.v=v;
    }

    public void setDecryptedMessage(String msg) {
        this.decryptedMessage=msg;
        this.isDecrypted=true;
    }

    public void setFromIcon(String fileId) {
        this.icon=fileId;
    }

    public long getMsgUID() {
        return this.msgId;
    }
}
