package halwarsing.net.halchatandroid.main;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.util.Log;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.halsm.HalSMExecutable;
import halwarsing.net.halchatandroid.halsm.HalSMNative;
import halwarsing.net.halchatandroid.type.HCMessage;
import kotlin.jvm.functions.Function3;

public class HalChatModule {
    private static final String version="0.0.2";
    private static final String TAG="HCModule";

    private static long sendMessage(long hsmc, long args, long vars) {
        Map<String,Object> nargs=HalChatPlugins.Main.hsme.parseArgs(args,"-sassiis",new String[]{"-","text","attachments","encryptId","soundMsg","answerMsg","commentMsg","pluginData"});
        if(nargs==null) {return HalChatPlugins.Main.hsme.falseVar;}

        String text=(String)nargs.get("text");
        List<String> attachments=(List<String>)nargs.get("attachments");
        String soundMsg=(String)nargs.get("soundMsg");
        String pluginData=(String)nargs.get("pluginData");

        HalChatPlugins.Main.pluginsInterface.sendMessagePlugin(text,attachments,pluginData,soundMsg);

        return HalChatPlugins.Main.hsme.trueVar;
    }

    private static long editMessage(long hsmc, long args, long vars) {
        Map<String,Object> nargs=HalChatPlugins.Main.hsme.parseArgs(args,"-is",new String[]{"-","msgId","text"});
        if(nargs==null) {return HalChatPlugins.Main.hsme.falseVar;}

        long msgId=(long)nargs.get("msgId");
        String text=(String)nargs.get("text");

        HalChatPlugins.Main.pluginsInterface.editMessagePlugin(msgId, text);

        return HalChatPlugins.Main.hsme.trueVar;
    }

    private static long editLocalMessage(long hsmc, long args, long vars) {
        Map<String,Object> nargs=HalChatPlugins.Main.hsme.parseArgs(args,"-is",new String[]{"-","msgId","text"});
        if(nargs==null) {return HalChatPlugins.Main.hsme.falseVar;}

        long msgId=(long)nargs.get("msgId");
        String text=(String)nargs.get("text");

        HalChatPlugins.Main.pluginsInterface.editLocalMessagePlugin(msgId, text);

        return HalChatPlugins.Main.hsme.trueVar;
    }

    private static Map<String,Object> initializeVars() {
        return new HashMap<>();
    }

    private static long addEvent(long hsmc, long args, long vars) {
        Map<String,Object> nargs=HalChatPlugins.Main.hsme.parseArgs(args,"-sl",new String[]{"-","name","func","namePluginData"},"s");
        if(nargs==null) {return HalChatPlugins.Main.hsme.falseVar;}

        String name=(String)nargs.get("name");
        long func=(long)nargs.get("func");

        if(nargs.containsKey("namePluginData")) {
            return HalSMNative.variableInit(HalChatPlugins.Main.hsme.addEvent(name,func,(String)nargs.get("namePluginData")),HalSMExecutable.variableTypes.get("int"));
        }
        return HalSMNative.variableInit(HalChatPlugins.Main.hsme.addEvent(name,func),HalSMExecutable.variableTypes.get("int"));
    }

    //TODO: Next Update (UPGRADE & DESIGN)
    private static long addContentMessage(long hsmc, long args, long vars) {
        return HalChatPlugins.Main.hsme.falseVar;
    }

    //TODO: Next Update (UPGRADE & DESIGN)
    private static long makeHTMLEvent(long hsmc, long args, long vars) {
        return HalChatPlugins.Main.hsme.falseVar;
    }

    //TODO: Next Update (UPGRADE & DESIGN)
    private static long runHTMLEvent(long hsmc, long args, long vars) {
        return HalChatPlugins.Main.hsme.falseVar;
    }

    //TODO: Next Update (UPGRADE & DESIGN)
    private static long openWindow(long hsmc, long args, long vars) {
        return HalChatPlugins.Main.hsme.falseVar;
    }

    //TODO: Next Update (UPGRADE & DESIGN)
    private static long addStyle(long hsmc, long args, long vars) {
        return HalChatPlugins.Main.hsme.falseVar;
    }

    //TODO: Next Update (UPGRADE & DESIGN)
    private static long eventData(long hsmc, long args, long vars) {
        return HalChatPlugins.Main.hsme.falseVar;
    }

    private static long showMessage(long hsmc, long args, long vars) {
        Map<String,Object> nargs=HalChatPlugins.Main.hsme.parseArgs(args,"-s",new String[]{"-","text"});
        if(nargs==null) {return HalChatPlugins.Main.hsme.falseVar;}

        String text=(String)nargs.get("text");
        HalChatPlugins.Main.pluginsInterface.showMessage(text);

        return HalChatPlugins.Main.hsme.nullVar;
    }

    private static long getLastMessages(long hsmc, long args, long vars) {
        Map<String,Object> nargs=HalChatPlugins.Main.hsme.parseArgs(args,"-il",new String[]{"-","maxCount","onLoadMessages"});
        if(nargs==null) {return HalChatPlugins.Main.hsme.falseVar;}

        long maxCount=(long)nargs.get("maxCount");
        long onLoadMessages=(long)nargs.get("onLoadMessages");
        HalChatPlugins plugins=HalChatPlugins.Main;

        TaskExecutorManager.getInstance().submitDecryptChatActivity("lastMsgsPlugin:chatId:"+plugins.chatId,()->{
            List<HCMessage> msgs=plugins.getLastMessages((int)maxCount);
            List<Map<String,Object>> newmsgs=new ArrayList<>();

            for(int i=0;i<msgs.size();i++) {
                newmsgs.add(msgs.get(i).toMap());
            }

            List<List<Map<String,Object>>> zargs=new ArrayList<>();
            zargs.add(newmsgs);

            plugins.post(()->HalSMNative.runLocalFunction(onLoadMessages,HalSMExecutable.getHalSMArguments(zargs),plugins.hsme.nullDict));
            return null;
        });

        return HalChatPlugins.Main.hsme.nullVar;
    }

    private static long inviteUser(long hsmc, long args, long vars) {
        Map<String,Object> nargs=HalChatPlugins.Main.hsme.parseArgs(args,"-i",new String[]{"-","userId"});
        if(nargs==null) {return HalChatPlugins.Main.hsme.falseVar;}

        long userId=(long)nargs.get("userId");

        if(userId>0) {
            HalChatPlugins.Main.hc.hnUsers.getUserByUserId(userId).thenAccept(user->{
                if(user!=null) {
                    new AlertDialog.Builder(HalChatPlugins.Main.context)
                            .setTitle("HalChatPlugins")
                            .setMessage("Плагин хочет пригласить пользователя ~"+user.nickname+" в чат, вы разрешаете?")
                            .setPositiveButton(R.string.hdw_yes, (dialogInterface, i) -> {
                                try {
                                    JSONObject apiargs=new JSONObject();
                                    apiargs.put("userId",userId);
                                    apiargs.put("chatId",HalChatPlugins.Main.chatId);
                                    HalChatPlugins.Main.hc.hnapi.apiReq("inviteUserRSA",apiargs).thenAccept(data->{
                                        if(data.optLong("errorCode",-1)==0) {
                                            Log.e(TAG,"Пользователь успешно приглашён в чат");
                                        }
                                    });
                                } catch (JSONException e) {
                                    Log.e(TAG,"Error json in inviteUser HCModule: ",e);
                                }
                            })
                            .setNegativeButton(R.string.hdw_no,null).show();
                }
            });
        }

        return HalChatPlugins.Main.hsme.nullVar;
    }

    private static long setCustomIcon(long hsmc, long args, long vars) {
        Map<String,Object> nargs=HalChatPlugins.Main.hsme.parseArgs(args,"-ss",new String[]{"-","obj","icon"});
        if(nargs==null) {return HalChatPlugins.Main.hsme.falseVar;}

        final String obj=(String)nargs.get("obj");
        final String icon=(String)nargs.get("icon");

        assert icon != null;
        assert obj !=null;
        if(!icon.matches("^[a-zA-Z0-9]+$")||icon.length()!=100) {
            return HalChatPlugins.Main.hsme.nullVar;
        }

        final String iconUrl="https://haldrive.halwarsing.net/file/"+icon;

        switch(obj) {
            case "SendBtnPluginDisabled":
                HalChatPlugins.Main.pluginsInterface.setPluginDisabledIcon(iconUrl);
                break;
        }

        return HalChatPlugins.Main.hsme.nullVar;
    }

    private static long disableUserSend(long hsmc, long args, long vars) {
        HalChatPlugins.Main.pluginsInterface.disableUserSend();
        return HalChatPlugins.Main.hsme.nullVar;
    }

    private static long enableUserSend(long hsmc, long args, long vars) {
        HalChatPlugins.Main.pluginsInterface.enableUserSend();
        return HalChatPlugins.Main.hsme.nullVar;
    }

    private static long setMessageInput(long hsmc, long args, long vars) {
        Map<String,Object> nargs=HalChatPlugins.Main.hsme.parseArgs(args,"-s",new String[]{"-","text"});
        if(nargs==null)return HalChatPlugins.Main.hsme.nullVar;

        String text=(String)nargs.get("text");

        HalChatPlugins.Main.pluginsInterface.setMessageInput(text);
        return HalChatPlugins.Main.hsme.nullVar;
    }

    private static long getMessageInput(long hsmc, long args, long vars) {
        return HalSMExecutable.javaToHalSMVar(HalChatPlugins.Main.pluginsInterface.getMessageInput());
    }

    /* Storage */
    private static long setStorageData(long hsmc, long args, long vars) {
        Map<String,Object> nargs=HalChatPlugins.Main.hsme.parseArgs(args,"-ss",new String[]{"-","key","value"});
        if(nargs==null)return HalChatPlugins.Main.hsme.nullVar;

        String key=(String)nargs.get("key");
        String value=(String)nargs.get("value");
        String pluginId=HalChatPlugins.Main.hsme.getPluginId(hsmc);

        HalChatPlugins.Main.setStorageData(pluginId,key,value);

        return HalChatPlugins.Main.hsme.nullVar;
    }

    private static long getStorageData(long hsmc, long args, long vars) {
        Map<String,Object> nargs=HalChatPlugins.Main.hsme.parseArgs(args,"-s",new String[]{"-","key"});
        if(nargs==null)return HalChatPlugins.Main.hsme.nullVar;

        String key=(String)nargs.get("key");
        String pluginId=HalChatPlugins.Main.hsme.getPluginId(hsmc);

        return HalSMExecutable.javaToHalSMVar(HalChatPlugins.Main.getStorageData(pluginId,key));
    }

    /* Chat Info */
    private static long getChatDescription(long hsmc, long args, long vars) {
        return HalSMExecutable.javaToHalSMVar(HalChatPlugins.Main.description);
    }

    private static long isStartChat(long hsmc, long args, long vars) {
        return HalSMExecutable.javaToHalSMVar(HalChatPlugins.Main.is_start_chat);
    }

    private static long isMobileDevice(long hsmc, long args, long vars) {
        return HalSMExecutable.javaToHalSMVar(true);
    }

    /* Plugin Data */
    private static long getPluginData(long hsmc, long args, long vars) {
        return HalSMExecutable.javaToHalSMVar(HalChatPlugins.Main.hsme.getPluginData(HalChatPlugins.Main.hsme.getPluginId(hsmc)));
    }
}
