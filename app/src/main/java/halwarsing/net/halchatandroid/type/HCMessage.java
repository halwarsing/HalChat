package halwarsing.net.halchatandroid.type;

import android.graphics.Bitmap;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

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
    public List<String> decryptedPollVariants;
    public long shareId;
    public long pixelId;
    public boolean isPinned;
    public long v;
    public List<MessageReaction> reactions;
    public long selectedReaction;
    public boolean hasReactionData;
    public long selectedPollVariant;
    public boolean hasPollResult;

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
        this.decryptedPollVariants=null;
        this.shareId=shareId;
        this.pixelId=pixelId;
        this.isPinned=isPinned;
        this.v=v;
        this.reactions=Collections.emptyList();
        this.selectedReaction=-1;
        this.hasReactionData=false;
        this.selectedPollVariant=-1;
        this.hasPollResult=false;
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

    public List<String> getPollVariants() {
        return decryptedPollVariants;
    }

    public void setPollVariants(List<String> variants) {
        decryptedPollVariants=variants==null
                ? null
                : Collections.unmodifiableList(new ArrayList<>(variants));
    }

    public void setReactions(List<MessageReaction> reactions, long selectedReaction) {
        ArrayList<MessageReaction> sortedReactions = new ArrayList<>();
        if(reactions != null) {
            for(MessageReaction reaction : reactions) {
                if(reaction != null && reaction.count > 0) {
                    sortedReactions.add(reaction);
                }
            }
        }
        sortedReactions.sort(MessageReaction.DISPLAY_ORDER);
        this.reactions=Collections.unmodifiableList(sortedReactions);
        this.selectedReaction=selectedReaction;
        this.hasReactionData=true;
    }

    public void applyReaction(long emojiId) {
        long oldReaction=selectedReaction;
        long newReaction=oldReaction==emojiId?-1:emojiId;
        ArrayList<MessageReaction> updatedReactions=new ArrayList<>();
        boolean newReactionExists=false;

        for(MessageReaction reaction : reactions) {
            long count=reaction.count;
            if(reaction.emojiId==oldReaction)count--;
            if(reaction.emojiId==newReaction) {
                count++;
                newReactionExists=true;
            }
            if(count>0)updatedReactions.add(new MessageReaction(reaction.emojiId,count));
        }

        if(newReaction!=-1&&!newReactionExists) {
            updatedReactions.add(new MessageReaction(newReaction,1));
        }
        setReactions(updatedReactions,newReaction);
    }

    public JSONObject getPlugins() {
        if(dataBot==null||dataBot.length()<3||dataBot.equals("-1"))return new JSONObject();
        try {
            JSONObject plugins=new JSONObject(dataBot).optJSONObject("plugins");
            return plugins==null?new JSONObject():plugins;
        } catch (JSONException e) {
            return new JSONObject();
        }
    }

    public Map<String, Object> toMap() {
        Map<String,Object> out=new HashMap<>();

        out.put("uid",msgId);
        out.put("fromChat",chatId);
        out.put("fromId",fromId);

        //TODO добавить передачу данных о пользователе
        out.put("fromIcon",icon);
        //out.put("fromNickname",nickname);
        out.put("message",decryptedMessage);
        out.put("msgText",decryptedMessage);

        List<String> latts=new ArrayList<>();
        for (int i=0;i<attachments.length();i++) {
            latts.add(attachments.optString(i,""));
        }
        out.put("attachments",latts);

        out.put("time",time);
        out.put("receive_message_sound",soundMsg);
        out.put("soundMsg",soundMsg);
        out.put("answerMsg",answerMsg);
        out.put("type",type);
        out.put("commentMsg",commentMsg);
        out.put("recordMic",recordMic);
        out.put("v",v);
        out.put("data",data);
        out.put("pixelId",pixelId);
        out.put("dataBot",dataBot);
        out.put("isPinned",isPinned);

        return out;
    }
}
