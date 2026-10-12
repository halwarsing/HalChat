package halwarsing.net.halchatandroid.main;

import android.database.sqlite.SQLiteDatabase;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.atomic.AtomicBoolean;

import halwarsing.net.halchatandroid.type.HCAction;

public class HalChatActions {
    private static final String TAG="HCActions";
    private final SQLiteDatabase db;
    private final String codeUser;
    private final long userId;
    private final HalChat hc;
    private final AtomicBoolean syncInProgress=new AtomicBoolean(false);
    private final AtomicBoolean syncAgain=new AtomicBoolean(false);

    public HalChatActions(SQLiteDatabase db,String codeUser,long userId,HalChat hc) {
        this.db=db;
        this.codeUser=codeUser;
        this.userId=userId;
        this.hc=hc;
    }


    protected void syncActions() {
        if(!syncInProgress.compareAndSet(false,true)) {
            syncAgain.set(true);
            return;
        }

        try {
            long lastAction=hc.uidSystem.getUID("actions");

            JSONObject postData=new JSONObject();
            postData.put("lastAction",lastAction<0?-1:lastAction);
            hc.hcapi.apiReq("getActions",postData).whenComplete((data,error)->{
                try {
                    if(error!=null) {
                        Log.e(TAG,"Unable to sync actions",error);
                        return;
                    }
                    if(data.getLong("errorCode")==0) {
                        JSONArray actions=data.optJSONArray("actions");
                        if(actions==null)actions=new JSONArray();

                        long newestActionId=getNewestActionId(actions);

                        if(actions.length()==0)return;

                        for(int i=0;i<actions.length();i++) {
                            HCAction action=HCAction.getFromJSONAPI(actions.getJSONObject(i));
                            if(action!=null)processAction(action).join();
                        }

                        if(newestActionId>lastAction) {
                            hc.uidSystem.setUID("actions",newestActionId);
                        }
                    } else {
                        Log.e(TAG,"getActionsError: "+data.getLong("errorCode")+";"+data.getString("error"));
                    }
                } catch (Exception e) {
                    Log.e(TAG,"getActions",e);
                } finally {
                    syncInProgress.set(false);
                    if(syncAgain.getAndSet(false))syncActions();
                }
            });
        } catch (Exception e) {
            syncInProgress.set(false);
            if(syncAgain.getAndSet(false))syncActions();
            Log.e(TAG,"syncActions",e);
        }
    }

    private long getNewestActionId(JSONArray actions) {
        long newest=-1;
        for(int i=0;i<actions.length();i++) {
            JSONObject action=actions.optJSONObject(i);
            if(action!=null)newest=Math.max(newest,action.optLong("uid",-1));
        }
        return newest;
    }



    protected CompletableFuture<Void> processAction(HCAction action) {
        Log.e(TAG,"Action: "+action.type+";"+action.uid);
        CompletableFuture<?> actionFuture=CompletableFuture.completedFuture(null);
        switch (action.type) {
            case 0:
                //Delete message
                hc.chatGroupChats.deleteMessageById(action.fromMsg);
                break;
            case 1:
                //Edit message
                if(action.newmsg==null) {
                    actionFuture=hc.chatGroupChats.updateMessage(action.fromChat,action.fromMsg);
                }
                break;
            case 3:
                //Invite User
                if(action.toId==hc.idUser) {
                    hc.chatGroupChats.addNewChat(action.fromChat);
                }
                break;
            case 5:
                //Update Chat Bot Info
                break;
            case 7:
                //Read message
                hc.chatGroupChats.markMessageRead(action.fromChat,action.fromMsg);
                break;
            case 10:
                //exit chat
                if(action.fromId==hc.idUser) {
                    hc.chatGroupChats.deleteChat(action.fromChat);
                } else {
                    hc.chatGroupChats.deleteChatUser(action.fromChat,action.fromId);
                }
                break;
            case 11:
                //Delete Chat
                hc.chatGroupChats.deleteChat(action.fromChat);
                break;
            case 12:
                //Block user
                break;
            case 13:
                //Pin message
                actionFuture=hc.chatGroupChats.updateMessage(action.fromChat,action.fromMsg);
                break;
            case 14:
                //Unpin message
                actionFuture=hc.chatGroupChats.updateMessage(action.fromChat,action.fromMsg);
                break;
            case 15:
                //Join Voice
                break;
            case 16:
                //Exit Voice
                break;
            case 17:
                //Message reaction changed
                if(action.newmsg==null) {
                    actionFuture=hc.chatGroupChats.updateMessage(action.fromChat,action.fromMsg);
                }
                break;
        }
        return actionFuture.thenApply(ignored->null);
    }
}
