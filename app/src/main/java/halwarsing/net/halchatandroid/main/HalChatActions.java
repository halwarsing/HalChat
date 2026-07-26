package halwarsing.net.halchatandroid.main;

import android.database.sqlite.SQLiteDatabase;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.concurrent.CompletableFuture;

import halwarsing.net.halchatandroid.type.HCAction;

public class HalChatActions {
    private static final String TAG="HCActions";
    private final SQLiteDatabase db;
    private final String codeUser;
    private final long userId;
    private final HalChat hc;

    public HalChatActions(SQLiteDatabase db,String codeUser,long userId,HalChat hc) {
        this.db=db;
        this.codeUser=codeUser;
        this.userId=userId;
        this.hc=hc;
    }


    protected void syncActions() {
        try {
            JSONObject postData=new JSONObject();
            postData.put("lastAction",Math.max(hc.uidSystem.getUID("actions"),0));
            hc.hcapi.apiReq("getActions",postData).thenAccept(data->{
                try {
                    if(data.getLong("errorCode")==0) {
                        JSONArray actions=data.getJSONArray("actions");

                        if(actions.length()==0)return;

                        for(int i=0;i<actions.length();i++) {
                            processAction(HCAction.getFromJSONAPI(actions.getJSONObject(i))).join();
                        }

                        hc.uidSystem.setUID("actions",
                                Math.max(
                                        actions.getJSONObject(0).getLong("uid"),
                                        actions.getJSONObject(actions.length()-1).getLong("uid")
                                )
                        );
                    } else {
                        Log.e(TAG,"getActionsError: "+data.getLong("errorCode")+";"+data.getString("error"));
                    }
                } catch (Exception e) {
                    Log.e(TAG,"getActions",e);
                }
            });
        } catch (JSONException e) {
            Log.e(TAG,"syncActions",e);
        }
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
