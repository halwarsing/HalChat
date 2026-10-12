package halwarsing.net.halchatandroid.halsm;

import android.util.Log;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import halwarsing.net.halchatandroid.localai.WLLamaLLM;
import halwarsing.net.halchatandroid.main.HalChat;
import halwarsing.net.halchatandroid.main.HalChatApp;
import halwarsing.net.halchatandroid.main.HalChatModelLoader;
import halwarsing.net.halchatandroid.main.HalChatPlugins;
import halwarsing.net.halchatandroid.type.HCMessage;

public class LocalLLMHalSM {
    private static final String version="0.0.5";
    private static final String TAG="LocalLLMHalSM";
    private static final Map<String, List<Long>> events=new HashMap<>();
    private static final Map<String, List<Method>> externEvents=new HashMap<>();
    public static volatile WLLamaLLM wllm=null;
    private static Runnable pendingRun=null;


    static {
        events.put("generate",new ArrayList<>());
        events.put("generate_stream",new ArrayList<>());
        events.put("load",new ArrayList<>());
        events.put("progressload",new ArrayList<>());

        externEvents.put("generate",new ArrayList<>());
        externEvents.put("generate_stream",new ArrayList<>());
        externEvents.put("load",new ArrayList<>());
        externEvents.put("progressload",new ArrayList<>());
    }

    public static Map<String,Object> initializeVars() {
        return new HashMap<>();
    }

    public static void stopLoadScreen() {
        HalChatPlugins.Main.pluginsInterface.stopLoadScreen();
    }

    public static void createLoadScreen(JSONObject options) {
        HalChatPlugins.Main.pluginsInterface.createLoadScreen(options);
    }

    public static void updateLoadScreen(JSONObject progress) {
        HalChatPlugins.Main.pluginsInterface.updateLoadScreen(progress);
    }

    public static long load(long hsmc, long args, long vars) {
        Map<String,Object> nargs=HalChatPlugins.Main.hsme.parseArgs(args,"-ssd",new String[]{"-","plugin_id","model_name","files","dtype","rag_model_name","rag_files","rag_dtype","externData","type","lora_adapters","model_files","aiModel"},"ssdsbsaai");
        if(nargs==null)return HalChatPlugins.Main.hsme.nullVar;

        String plugin_id=(String)nargs.get("plugin_id");
        String model_name=(String)nargs.get("model_name");
        Map<String,String> files=(Map<String,String>)nargs.get("files");
        String dtype=(String)nargs.getOrDefault("dtype","q4");
        String rag_model_name=(String)nargs.getOrDefault("rag_model_name",null);
        Map<String,String> rag_files=(Map<String,String>)nargs.getOrDefault("rag_files",new HashMap<>());
        String rag_dtype=(String)nargs.getOrDefault("rag_dtype","q4");
        boolean externData=(boolean)nargs.getOrDefault("externData",false);
        String type=(String)nargs.getOrDefault("type","TextGeneration");
        List<List<Object>> lora_adapters=(List<List<Object>>)nargs.getOrDefault("lora_adapters",new ArrayList<>());
        List<String> model_files=(List<String>)nargs.getOrDefault("model_files",new ArrayList<>());
        long aiModel=(long)nargs.getOrDefault("aiModel",0L);

        long ragFileCount=lora_adapters==null?0:lora_adapters.size();
        long modelFileCount=model_files==null?0:model_files.size();
        long totalFileCount=ragFileCount+modelFileCount;
        String historyKey=plugin_id+":"+model_name+":"+type;

        try {
            JSONObject lsj=new JSONObject();
            lsj.put("fileCount",totalFileCount);
            lsj.put("historyKey",historyKey);
            createLoadScreen(lsj);

            if(ragFileCount>0) {
                //TODO ADD RAG
            }

            final AIProgressReporter modelProgress=new AIProgressReporter(totalFileCount, ragFileCount, modelFileCount, "model");

            switch (type) {
                case "MultiModal":
                    break;
                case "WLLamaLLM":
                    if(wllm!=null)wllm.close();
                    wllm=new WLLamaLLM(HalChatPlugins.Main.hc,HalChatPlugins.Main.context);

                    WLLamaLLM current=wllm;
                    current.load(plugin_id,aiModel,1,dtype,files,modelProgress,lora_adapters,model_files).thenAccept(ok->{
                        if(current!=wllm)return;
                        if(ok) {
                            LocalLLMHalSM.runEvent("load",new ArrayList<>());
                            LocalLLMHalSM.runExternEvent("load");
                        }

                        stopLoadScreen();
                    });

                    if(pendingRun!=null) {
                        pendingRun.run();
                        pendingRun=null;
                    }
                    break;
                default:
                    break;
            }

            
        } catch (JSONException e) {
            Log.e(TAG,"Error JSON load(): ",e);
        }

        return HalChatPlugins.Main.hsme.nullVar;
    }

    public static long loadGPU(long hsmc, long args, long vars) {
        return LocalLLMHalSM.load(hsmc,args,vars);
    }

    public static long run(long hsmc, long args, long vars) {
        Map<String,Object> nargs=HalChatPlugins.Main.hsme.parseArgs(args,"-ssfifib",new String[]{"-","promptSystem","prompt","temperature","top_k","top_p","max_new_tokens","is_load_history","context",
                "AIName","chat_template","load_messages_count","repetition_penalty"},"assif");
        if(nargs==null) {
            Log.e(TAG,"run(): wrong arguments");
            return HalChatPlugins.Main.hsme.nullVar;
        }

        String promptSystem=(String)nargs.get("promptSystem");
        String prompt=(String)nargs.get("prompt");
        boolean is_load_history=(boolean)nargs.get("is_load_history");
        Object context=nargs.get("context");
        String AIName=nargs.get("AIName")==null?"assistant":(String)nargs.get("AIName");
        long load_messages_count=nargs.get("load_messages_count")==null?100:(long)nargs.get("load_messages_count");
        double repetition_penalty=nargs.get("repetition_penalty")==null?1.1:(double)nargs.get("repetition_penalty");

        long tRun=System.currentTimeMillis(); //PERF
        JSONArray messages_ai=new JSONArray();
        JSONObject opt=new JSONObject();
        try {
            messages_ai.put(new JSONObject().put("role","system").put("content",promptSystem));

            if(is_load_history) {
                List<HCMessage> hcmsgs=HalChatPlugins.Main.getLastMessages((int)load_messages_count);

                if(hcmsgs.isEmpty()) {
                    messages_ai.put(new JSONObject().put("role","user").put("content",prompt));
                }

                for(int i=hcmsgs.size()-1;i>=0;i--) {
                    HCMessage hcmsg=hcmsgs.get(i);
                    JSONObject plugins=hcmsg.getPlugins();

                    if("ignore".equals(plugins.optString("LocalLLM")))continue;

                    if(plugins.has("LocalBotMsg")) {
                        messages_ai.put(new JSONObject().put("role","assistant").put("name",AIName).put("content",hcmsg.decryptedMessage));
                    } else {
                        messages_ai.put(new JSONObject().put("role","user").put("content",hcmsg.decryptedMessage));
                    }
                }
            } else {
                messages_ai.put(new JSONObject().put("role","user").put("content",context!=null?prompt:"Мой вопрос: "+prompt));
            }

            opt.put("maxNewTokens",nargs.get("max_new_tokens"));
            opt.put("temperature",nargs.get("temperature"));
            opt.put("top_k",nargs.get("top_k"));
            opt.put("top_p",nargs.get("top_p"));
            opt.put("repetition_penalty",repetition_penalty);
        } catch (JSONException e) {
            Log.e(TAG,"Error JSON run(): ",e);
            return HalChatPlugins.Main.hsme.nullVar;
        }
        Log.w("HalPerf","run: "+messages_ai.length()+" messages, "+messages_ai.toString().length()+" chars, history="+is_load_history+" ("+load_messages_count+"), max_new_tokens="+nargs.get("max_new_tokens")+", built in "+(System.currentTimeMillis()-tRun)+" ms"); //PERF

        Runnable generate=()->wllm.generateChatStream(messages_ai,opt,new WLLamaLLM.WLLamaLLMInterface() {
            @Override
            public void ongeneratestream(String text, String delta) {
                runEvent("generate_stream",Collections.singletonList(text));
                runExternEvent("generate_stream",text);
            }

            @Override
            public void ongenerate(String text) {
                runEvent("generate",Collections.singletonList(text));
                runExternEvent("generate",text);

                HalChatPlugins plugins=HalChatPlugins.Main;
                if(plugins!=null)plugins.pluginsInterface.onGenerateDone();
            }
        });

        if(wllm==null) {
            pendingRun=generate;
        } else {
            generate.run();
        }

        return HalChatPlugins.Main.hsme.nullVar;
    }

    public static long stop(long hsmc, long args, long vars) {
        if(wllm!=null) {
            wllm.stop();
        }

        return HalChatPlugins.Main.hsme.nullVar;
    }

    public static long addEvent(long hsmc, long args, long vars) {
        Map<String,Object> nargs=HalChatPlugins.Main.hsme.parseArgs(args,"-sl",new String[]{"-","name","func"});
        if(nargs==null)return HalChatPlugins.Main.hsme.nullVar;

        String name=(String)nargs.get("name");
        long func=(long)nargs.get("func");

        if(events.containsKey(name)) {
            events.get(name).add(func);
        }

        return HalChatPlugins.Main.hsme.nullVar;
    }

    private static void runEvent(String name, List<?> args) {
        HalChatPlugins plugins=HalChatPlugins.Main;
        if(plugins==null||!events.containsKey(name))return;

        plugins.post(()->{
            long hsmargs=HalSMExecutable.getHalSMArguments(args);

            for(long funcVal : events.get(name)) {
                HalSMNative.runLocalFunction(funcVal,hsmargs,plugins.hsme.nullDict);
            }
        });
    }

    public static void close() {
        pendingRun=null;
        if(wllm!=null) {
            wllm.close();
            wllm=null;
        }

        for(List<Long> a:events.values())a.clear();
    }

    public static long addExternEvent(String name, Method func) {
        if(!externEvents.containsKey(name))return -1;
        externEvents.get(name).add(func);
        return externEvents.get(name).size()-1;
    }

    private static boolean runExternEvent(String name, Object... args) {
        if(!externEvents.containsKey(name))return false;

        int i,l;
        List<Method> a=externEvents.get(name);
        l=a.size();

        for(i=0;i<l;i++) {
            try {
                a.get(i).invoke(null,args);
            } catch (IllegalAccessException|InvocationTargetException e) {
                Log.e(TAG,"Error run externEvent Function",e);
            }
        }

        return true;
    }

    public static class AIProgressReporter {
        private final long totalFileCount;
        private final long offset;
        private final long batchFileCount;
        private final String scope;

        public AIProgressReporter(long totalFileCount, long offset, long batchFileCount, String scope) {
            this.totalFileCount=totalFileCount;
            this.offset=offset;
            this.batchFileCount=batchFileCount;
            this.scope=scope;
        }

        public void progress(String phase, float pct, long loaded, long total, boolean fromCache, String filename, int fileIndex, int fileCount, String scope) {
            JSONObject data=new JSONObject();
            long globalIndex=phase.equals("initializing") ?offset+Math.max(0,batchFileCount-1) :offset+fileIndex;

            try {
                data.put("phase",phase);
                data.put("pct",pct);
                data.put("loaded",loaded);
                data.put("total",total);
                data.put("fromCache",fromCache);
                data.put("filename",filename);
                data.put("fileIndex",globalIndex);
                data.put("fileCount",fileCount);
                data.put("scope",scope);
            } catch (JSONException e) {
                Log.e(TAG,"Error progress: ",e);
            }

            updateLoadScreen(data);
        }
    }
}
