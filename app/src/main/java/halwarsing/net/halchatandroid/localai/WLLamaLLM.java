package halwarsing.net.halchatandroid.localai;

import android.content.Context;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.List;
import java.util.Map;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import halwarsing.net.halchatandroid.halsm.LocalLLMHalSM;
import halwarsing.net.halchatandroid.main.HalChat;
import halwarsing.net.halchatandroid.main.HalChatModelLoader;

public class WLLamaLLM {
    public static final String name="WLLamaLLM";
    private static final String TAG="WLLamaLLM";
    private static final String version="0.0.2";
    private static final long streamMs=100;
    private static final int streamChars=24;
    private boolean loading=false;
    private long startedAt=0;
    private boolean firstChunk=false;
    private final HalChat hc;
    private final Context context;
    private final HalChatModelLoader modelLoader;
    private final ExecutorService llamaExecutor=Executors.newSingleThreadExecutor();
    private volatile long modelHandle=0L;

    public WLLamaLLM(HalChat hc, Context context) {
        startedAt= System.currentTimeMillis() / 1000L;
        this.hc=hc;
        this.context=context;
        this.modelLoader=new HalChatModelLoader(hc,context);
    }

    public CompletableFuture<Boolean> load(String plugin_id, long aiModel, long version, String dtype, Map<String,String> files, LocalLLMHalSM.AIProgressReporter aiProgress, List<List<Object>> lora_adapters, List<String> model_files) {
        CompletableFuture<Boolean> future=new CompletableFuture<>();

        llamaExecutor.execute(()->{
            Map<String,String> paths=modelLoader.loadModelFiles(aiModel,version,files,aiProgress);

            int i,l=model_files.size();
            String[] filesOut=new String[l];
            for(i=0;i<l;i++) {
                filesOut[i]=paths.get(model_files.get(i));
                if(filesOut[i]==null) {
                    Log.e(TAG,"Model file is not loaded: "+model_files.get(i));
                    future.complete(false);
                    return;
                }
            }

            JSONObject params=new JSONObject();
            try {
                params.put("n_ctx",4096);
                params.put("n_threads",4);
                params.put("flash_attn",true);
                params.put("cache_type_k","q8_0");
                params.put("cache_type_v","q8_0");
                params.put("default_template_kwargs",new JSONObject().put("enable_thinking",false));

                JSONArray loras=new JSONArray();
                for(List<Object> z:lora_adapters) {
                    String path=paths.get((String)z.get(0));
                    if(path==null) {
                        Log.w(TAG,"LoRA adapter is not loaded: "+z.get(0));
                        continue;
                    }
                    loras.put(new JSONObject().put("path",path).put("scale",((Number)z.get(1)).doubleValue()));
                }
                params.put("lora_adapters",loras);
            } catch (JSONException e) {
                Log.e(TAG,"Error JSON load(): ",e);
                future.complete(false);
                return;
            }

            LLamaNative.init(context.getApplicationInfo().nativeLibraryDir);
            modelHandle=LLamaNative.create();

            if(LLamaNative.loadModel(modelHandle,filesOut,params.toString())!=0) {
                Log.e(TAG,"load failed: "+LLamaNative.getLastError(modelHandle));
                LLamaNative.destroy(modelHandle);
                modelHandle=0;
                future.complete(false);
                return;
            }

            future.complete(true);
        });

        return future;
    }

    public void generateChatStream(JSONArray messages, JSONObject opt, WLLamaLLMInterface eventInterface) {
        stop();

        long tQueue=System.currentTimeMillis(); //PERF
        llamaExecutor.execute(()->{
            Log.w("HalPerf","generate: waited in queue "+(System.currentTimeMillis()-tQueue)+" ms"); //PERF
            if(modelHandle==0) {
                Log.e(TAG,"generate: the model is not loaded");
                return;
            }

            JSONObject params=new JSONObject();
            try {
                params.put("max_tokens",opt.optInt("maxNewTokens",512));
                params.put("temperature",opt.optDouble("temperature",0.25));
                params.put("top_k",opt.optInt("top_k",40));
                params.put("top_p",opt.optDouble("top_p",0.9));
                params.put("min_p",0.0);
                params.put("penalty_repeat",opt.optDouble("repetition_penalty",1.0));
                params.put("penalty_present",0.0);
                params.put("penalty_freq",0.0);
                params.put("cache_prompt",false);
            } catch (JSONException e) {
                Log.e(TAG,"Error JSON generateChatStream(): ",e);
                return;
            }

            StringBuilder itog=new StringBuilder();
            StringBuilder pendingDelta=new StringBuilder();
            long[] lastSend={0};

            long tCompletion=System.currentTimeMillis(); //PERF
            int rc=LLamaNative.chatCompletion(modelHandle,messages.toString(),params.toString(),(content,reasoning)->{
                itog.append(reasoning).append(content);
                pendingDelta.append(reasoning).append(content);

                long now=System.currentTimeMillis();
                if(now-lastSend[0]>=streamMs||pendingDelta.length()>=streamChars) {
                    lastSend[0]=now;
                    eventInterface.ongeneratestream(itog.toString(),pendingDelta.toString());
                    pendingDelta.setLength(0);
                }
            });

            Log.w("HalPerf","generate: chatCompletion "+(System.currentTimeMillis()-tCompletion)+" ms, rc="+rc+", "+itog.length()+" chars"); //PERF
            if(rc!=LLamaNative.RESULT_DONE&&rc!=LLamaNative.RESULT_LENGTH) {
                if(rc<0)Log.e(TAG,"generate failed: "+LLamaNative.getLastError(modelHandle));
                return;
            }

            if(pendingDelta.length()>0) {
                eventInterface.ongeneratestream(itog.toString(),pendingDelta.toString());
            }

            eventInterface.ongenerate(itog.toString());
        });
    }

    public void stop() {
        if(modelHandle!=0) {
            LLamaNative.stop(modelHandle);
        }
    }

    public void close() {
        stop();
        llamaExecutor.execute(()->{
            if(modelHandle!=0) {
                LLamaNative.destroy(modelHandle);
                modelHandle=0;
            }
        });
        llamaExecutor.shutdown();
    }

    public interface WLLamaLLMInterface {
        void ongeneratestream(String text, String delta);
        void ongenerate(String text);
    }
}
