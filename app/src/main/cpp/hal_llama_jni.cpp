//
// JNI binding for llama.cpp.
// Java class: halwarsing.net.halchatandroid.localai.LLamaNative
//
// The API repeats what HalChatWeb uses from wllama (see WLLamaLLMWorker.js and HalChatModelCustomCache.js):
// - loadModel(files,params)      <-> wllama.loadModel(modelFiles,loadParams)
// - chatCompletion(messages,...) <-> wllama.createChatCompletion(params) with stream:true and onData
// - stop(handle)                 <-> abortController.abort()
// Params are JSON strings with the same keys as in wllama, so the new keys can be added here without
// changing the Java signature.
//
// One handle = one model. Use loadModel/chatCompletion from one thread; stop() can be called from any thread.
//
#include <android/log.h>
#include <jni.h>
#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "chat.h"
#include "common.h"
#include "llama.h"
#include "sampling.h"

using json=nlohmann::ordered_json;

#define LOG_TAG "HalLLama"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

//PERF: timing logs, remove before release (HAL_PERF 0 or delete all LOGP)
#define HAL_PERF 0
#if HAL_PERF
#define LOGP(...) __android_log_print(ANDROID_LOG_WARN,"HalLLamaPerf",__VA_ARGS__)
#else
#define LOGP(...)
#endif

#define LLAMA_JNI(ret,name) extern "C" JNIEXPORT ret JNICALL Java_halwarsing_net_halchatandroid_localai_LLamaNative_##name

//Results of chatCompletion
#define RESULT_DONE 0 //EOS or a stop string
#define RESULT_STOPPED 1 //stop() was called
#define RESULT_LENGTH 2 //max_tokens or the context is full
#define RESULT_ERROR -1

//Java <-> UTF-8
//GetStringUTFChars/NewStringUTF use modified UTF-8 an emoji is 6 bytes there), so convert by hand.

static std::string jstringToUtf8(JNIEnv* env,jstring str) {
    std::string out;
    if (str==nullptr){return out;}
    jsize length=env->GetStringLength(str);
    const jchar* chars=env->GetStringChars(str,nullptr);
    for (jsize i=0;i<length;i++) {
        unsigned int c=chars[i];
        if (c>=0xD800&&c<=0xDBFF&&i+1<length&&chars[i+1]>=0xDC00&&chars[i+1]<=0xDFFF) {
            c=0x10000+((c-0xD800)<<10)+(chars[i+1]-0xDC00);
            i++;
        }
        if (c<0x80) {
            out+=(char)c;
        } else if (c<0x800) {
            out+=(char)(0xC0|(c>>6));
            out+=(char)(0x80|(c&0x3F));
        } else if (c<0x10000) {
            out+=(char)(0xE0|(c>>12));
            out+=(char)(0x80|((c>>6)&0x3F));
            out+=(char)(0x80|(c&0x3F));
        } else {
            out+=(char)(0xF0|(c>>18));
            out+=(char)(0x80|((c>>12)&0x3F));
            out+=(char)(0x80|((c>>6)&0x3F));
            out+=(char)(0x80|(c&0x3F));
        }
    }
    env->ReleaseStringChars(str,chars);
    return out;
}

static jstring utf8ToJstring(JNIEnv* env,const std::string& utf8) {
    std::u16string out;
    const unsigned char* b=(const unsigned char*)utf8.data();
    size_t length=utf8.size(),i=0;
    while (i<length) {
        unsigned int c;
        size_t len;
        if (b[i]<0x80){c=b[i];len=1;}
        else if ((b[i]&0xE0)==0xC0&&i+1<length){c=((b[i]&0x1F)<<6)|(b[i+1]&0x3F);len=2;}
        else if ((b[i]&0xF0)==0xE0&&i+2<length){c=((b[i]&0x0F)<<12)|((b[i+1]&0x3F)<<6)|(b[i+2]&0x3F);len=3;}
        else if ((b[i]&0xF8)==0xF0&&i+3<length){c=((b[i]&0x07)<<18)|((b[i+1]&0x3F)<<12)|((b[i+2]&0x3F)<<6)|(b[i+3]&0x3F);len=4;}
        else {c=0xFFFD;len=1;}
        if (c>=0x10000) {
            c-=0x10000;
            out+=(char16_t)(0xD800+(c>>10));
            out+=(char16_t)(0xDC00+(c&0x3FF));
        } else {
            out+=(char16_t)c;
        }
        i+=len;
    }
    return env->NewString((const jchar*)out.data(),(jsize)out.size());
}

//The length of the text without a character that is not finished yet at the end.
//A token can be a part of a UTF-8 character, so the text is sent to Java only by whole characters.
static size_t utf8CompleteLength(const std::string& text) {
    size_t length=text.size();
    for (size_t back=1;back<=3&&back<=length;back++) {
        unsigned char c=(unsigned char)text[length-back];
        if ((c&0xC0)==0x80){continue;} //the middle of a character
        size_t need=c>=0xF0?4:c>=0xE0?3:c>=0xC0?2:1;
        return need>back?length-back:length;
    }
    return length;
}

//JSON

template<typename T>
static T jsonValue(const json& j,const char* key,T defaultValue) {
    auto it=j.find(key);
    if (it==j.end()||it->is_null()){return defaultValue;}
    return it->get<T>();
}

//cache_type_k/cache_type_v of wllama
static ggml_type cacheTypeFromName(const std::string& name,ggml_type defaultType) {
    if (name=="f32"){return GGML_TYPE_F32;}
    if (name=="f16"){return GGML_TYPE_F16;}
    if (name=="bf16"){return GGML_TYPE_BF16;}
    if (name=="q8_0"){return GGML_TYPE_Q8_0;}
    if (name=="q5_1"){return GGML_TYPE_Q5_1;}
    if (name=="q5_0"){return GGML_TYPE_Q5_0;}
    if (name=="q4_1"){return GGML_TYPE_Q4_1;}
    if (name=="q4_0"){return GGML_TYPE_Q4_0;}
    if (name=="iq4_nl"){return GGML_TYPE_IQ4_NL;}
    if(name=="q1_0"){return GGML_TYPE_Q1_0;}
    if(name=="q2_0"){return GGML_TYPE_Q2_0;}
    return defaultType;
}

static void androidLog(enum ggml_log_level level,const char* text,void* userData) {
    int priority=ANDROID_LOG_DEBUG;
    if (level==GGML_LOG_LEVEL_ERROR){priority=ANDROID_LOG_ERROR;}
    else if (level==GGML_LOG_LEVEL_WARN){priority=ANDROID_LOG_WARN;}
    else if (level==GGML_LOG_LEVEL_INFO){priority=ANDROID_LOG_INFO;}
    __android_log_print(priority,LOG_TAG,"%s",text);
}

static std::once_flag backendInitFlag;

//Frees the sampler at any exit of chatCompletion
struct SamplerGuard {
    common_sampler* sampler;
    ~SamplerGuard(){if (sampler!=nullptr){common_sampler_free(sampler);}}
};

struct LLamaSession {
    llama_model* model=nullptr;
    llama_context* ctx=nullptr;
    llama_batch batch{};
    bool isBatchInit=false;
    common_chat_templates_ptr templates;
    std::vector<llama_adapter_lora*> loras;
    std::vector<float> loraScales;

    int nCtx=0;
    int nBatch=0;

    //Defaults from the params of loadModel (the request can override them)
    std::string reasoningFormat="deepseek";
    json templateKwargs=json::object();
    bool isCtxShift=true;

    //The tokens that are in the KV cache now (sequence 0). Used by cache_prompt and ctx_shift.
    std::vector<llama_token> kvTokens;

    std::atomic_bool isStop{false};
    std::string lastError;

    ~LLamaSession(){unload();}

    static bool abortCallback(void* data) {
        return ((LLamaSession*)data)->isStop.load(std::memory_order_relaxed);
    }

    void setError(const std::string& error) {
        lastError=error;
        LOGE("%s",error.c_str());
    }

    void unload() {
        templates.reset();
        if (ctx!=nullptr) {
            //The adapters are freed together with the model
            llama_set_adapters_lora(ctx,nullptr,0,nullptr);
        }
        loras.clear();
        loraScales.clear();
        if (isBatchInit) {
            llama_batch_free(batch);
            isBatchInit=false;
        }
        if (ctx!=nullptr) {
            llama_free(ctx);
            ctx=nullptr;
        }
        if (model!=nullptr) {
            llama_model_free(model);
            model=nullptr;
        }
        kvTokens.clear();
        nCtx=0;
        nBatch=0;
    }

    //Params (the keys of wllama): n_ctx, n_batch, n_threads, n_gpu_layers, flash_attn, offload_kqv,
    //cache_type_k, cache_type_v, lora_adapters [{path,scale}], reasoning_format, default_template_kwargs, ctx_shift.
    //The other keys of wllama (jinja, cont_batching, warmup, reasoning...) are ignored for now.
    int load(const std::vector<std::string>& files,const json& params) {
        unload();
        isStop=false;
        lastError.clear();
        if (files.empty()){setError("No model files");return -1;}

        llama_model_params modelParams=llama_model_default_params();
        modelParams.n_gpu_layers=jsonValue<int>(params,"n_gpu_layers",0);
        if (files.size()==1) {
            model=llama_model_load_from_file(files[0].c_str(),modelParams);
        } else {
            //A model that is split into several files, the order is important
            std::vector<const char*> paths;
            for (const std::string& file:files){paths.push_back(file.c_str());}
            model=llama_model_load_from_splits(paths.data(),paths.size(),modelParams);
        }
        if (model==nullptr){setError("llama.cpp cannot load the model (see Logcat: HalLLama)");return -2;}

        int trainCtx=llama_model_n_ctx_train(model);
        nCtx=jsonValue<int>(params,"n_ctx",4096);
        if (trainCtx>0&&nCtx>trainCtx){nCtx=trainCtx;}
        nBatch=std::min(jsonValue<int>(params,"n_batch",512),nCtx);

        int threads=jsonValue<int>(params,"n_threads",0);
        if (threads<=0){threads=std::max(1L,sysconf(_SC_NPROCESSORS_ONLN)/2);}

        llama_context_params ctxParams=llama_context_default_params();
        ctxParams.n_ctx=nCtx;
        ctxParams.n_batch=nBatch;
        ctxParams.n_ubatch=nBatch;
        ctxParams.n_threads=threads;
        ctxParams.n_threads_batch=threads;
        ctxParams.offload_kqv=jsonValue<bool>(params,"offload_kqv",true);
        ctxParams.type_k=cacheTypeFromName(jsonValue<std::string>(params,"cache_type_k","f16"),GGML_TYPE_F16);
        ctxParams.type_v=cacheTypeFromName(jsonValue<std::string>(params,"cache_type_v","f16"),GGML_TYPE_F16);
        if (params.contains("flash_attn")&&params["flash_attn"].is_boolean()) {
            ctxParams.flash_attn_type=params["flash_attn"].get<bool>()?LLAMA_FLASH_ATTN_TYPE_ENABLED:LLAMA_FLASH_ATTN_TYPE_DISABLED;
        }
        ctxParams.abort_callback=abortCallback;
        ctxParams.abort_callback_data=this;
        ctx=llama_init_from_model(model,ctxParams);
        if (ctx==nullptr){setError("Cannot make the context, try to decrease n_ctx");unload();return -3;}

        batch=llama_batch_init(nBatch,0,1);
        isBatchInit=true;

        //LoRA
        auto adapters=params.find("lora_adapters");
        if (adapters!=params.end()&&adapters->is_array()) {
            for (const json& item:*adapters) {
                std::string path=jsonValue<std::string>(item,"path","");
                llama_adapter_lora* adapter=llama_adapter_lora_init(model,path.c_str());
                if (adapter==nullptr){setError("Cannot load the LoRA adapter: "+path);unload();return -4;}
                loras.push_back(adapter);
                loraScales.push_back(jsonValue<float>(item,"scale",1.0f));
            }
            if (!loras.empty()&&llama_set_adapters_lora(ctx,loras.data(),loras.size(),loraScales.data())!=0) {
                setError("llama.cpp rejected the LoRA adapters");
                unload();
                return -5;
            }
        }

        templates=common_chat_templates_init(model,"");
        reasoningFormat=jsonValue<std::string>(params,"reasoning_format","deepseek");
        templateKwargs=params.contains("default_template_kwargs")?params["default_template_kwargs"]:json::object();
        isCtxShift=jsonValue<bool>(params,"ctx_shift",true);

        LOGI("Model is loaded: n_ctx=%d n_batch=%d threads=%d lora=%d",nCtx,nBatch,threads,(int)loras.size());
        return 0;
    }

    //Decodes tokens[from..] to sequence 0 by the pieces of n_batch. Returns 0, RESULT_STOPPED or RESULT_ERROR.
    int decodePrompt(const std::vector<llama_token>& tokens,size_t from) {
        size_t i,j,count;
        for (i=from;i<tokens.size();i+=nBatch) {
            if (isStop){return RESULT_STOPPED;}
            count=std::min((size_t)nBatch,tokens.size()-i);
            common_batch_clear(batch);
            for (j=0;j<count;j++) {
                //Only the logits of the last token of the prompt are needed
                common_batch_add(batch,tokens[i+j],(llama_pos)(i+j),{0},i+j==tokens.size()-1);
            }
            int64_t tBatch=ggml_time_ms(); //PERF
            int r=llama_decode(ctx,batch);
            if (r!=0) {
                if (isStop){return RESULT_STOPPED;}
                setError("llama_decode failed on the prompt: "+std::to_string(r));
                return RESULT_ERROR;
            }
            for (j=0;j<count;j++){kvTokens.push_back(tokens[i+j]);}
            LOGP("prompt batch %zu/%zu: %zu tokens in %lld ms",i+count,tokens.size(),count,(long long)(ggml_time_ms()-tBatch)); //PERF
        }
        return 0;
    }

    //ctx_shift of wllama: when the context is full, the half of the old tokens is removed.
    //n_keep of the request is the number of the first tokens that are never removed.
    bool shiftContext(int nKeep) {
        llama_memory_t mem=llama_get_memory(ctx);
        if (!isCtxShift||!llama_memory_can_shift(mem)){return false;}
        int nPast=(int)kvTokens.size();
        int nDiscard=(nPast-nKeep)/2;
        if (nDiscard<=0){return false;}
        llama_memory_seq_rm(mem,0,nKeep,nKeep+nDiscard);
        llama_memory_seq_add(mem,0,nKeep+nDiscard,nPast,-nDiscard);
        kvTokens.erase(kvTokens.begin()+nKeep,kvTokens.begin()+nKeep+nDiscard);
        return true;
    }

    //messages: like in wllama [{role,content}]
    //params: max_tokens, temperature, top_k, top_p, min_p, penalty_repeat, penalty_present, penalty_freq, seed,
    //cache_prompt, n_keep, reasoning_format, chat_template_kwargs {enable_thinking...}
    //callback: LLamaNative.StreamCallback.onData(String content,String reasoning), it gets the deltas.
    int chatCompletion(JNIEnv* env,const json& messages,const json& params,jobject callback) {
        if (model==nullptr||ctx==nullptr){setError("The model is not loaded");return RESULT_ERROR;}
        isStop=false;
        lastError.clear();
        int64_t tStart=ggml_time_ms(); //PERF

        jclass callbackClass=env->GetObjectClass(callback);
        jmethodID onData=env->GetMethodID(callbackClass,"onData","(Ljava/lang/String;Ljava/lang/String;)V");
        env->DeleteLocalRef(callbackClass);
        if (onData==nullptr){env->ExceptionClear();setError("StreamCallback.onData(String,String) is not found");return RESULT_ERROR;}

        //Prompt from the chat template
        common_chat_templates_inputs inputs;
        inputs.messages=common_chat_msgs_parse_oaicompat(messages);
        inputs.use_jinja=true;
        inputs.reasoning_format=common_reasoning_format_from_name(jsonValue<std::string>(params,"reasoning_format",reasoningFormat));
        std::map<std::string,std::string> kwargs;
        for (auto& item:templateKwargs.items()){kwargs[item.key()]=item.value().dump();}
        if (params.contains("chat_template_kwargs")) {
            for (auto& item:params["chat_template_kwargs"].items()){kwargs[item.key()]=item.value().dump();}
        }
        inputs.chat_template_kwargs=kwargs;
        if (kwargs.count("enable_thinking")>0){inputs.enable_thinking=kwargs["enable_thinking"]!="false";}
        common_chat_params chat=common_chat_templates_apply(templates.get(),inputs);
        LOGP("template: %zu messages, %zu chars in %lld ms",inputs.messages.size(),chat.prompt.size(),(long long)(ggml_time_ms()-tStart)); //PERF

        int64_t tTokenize=ggml_time_ms(); //PERF
        const llama_vocab* vocab=llama_model_get_vocab(model);
        std::vector<llama_token> tokens=common_tokenize(vocab,chat.prompt,true,true);
        LOGP("tokenize: %zu tokens in %lld ms",tokens.size(),(long long)(ggml_time_ms()-tTokenize)); //PERF
        if (tokens.empty()){setError("The prompt is empty");return RESULT_ERROR;}
        if ((int)tokens.size()+1>=nCtx){setError("The prompt is too long for n_ctx="+std::to_string(nCtx));return RESULT_ERROR;}

        //Splits the text of the answer to content and reasoning_content
        common_chat_parser_params parser(chat);
        parser.reasoning_format=inputs.reasoning_format;
        parser.reasoning_in_content=inputs.reasoning_format==COMMON_REASONING_FORMAT_DEEPSEEK_LEGACY;
        parser.parse_tool_calls=false;
        if (!chat.parser.empty()){parser.parser.load(chat.parser);}

        //Sampler
        common_params_sampling sampling;
        sampling.temp=std::max(0.0f,jsonValue<float>(params,"temperature",sampling.temp));
        sampling.top_k=jsonValue<int>(params,"top_k",sampling.top_k);
        sampling.top_p=jsonValue<float>(params,"top_p",sampling.top_p);
        sampling.min_p=jsonValue<float>(params,"min_p",sampling.min_p);
        sampling.penalty_repeat=jsonValue<float>(params,"penalty_repeat",sampling.penalty_repeat);
        sampling.penalty_present=jsonValue<float>(params,"penalty_present",sampling.penalty_present);
        sampling.penalty_freq=jsonValue<float>(params,"penalty_freq",sampling.penalty_freq);
        sampling.seed=jsonValue<uint32_t>(params,"seed",sampling.seed);
        for (const std::string& text:chat.preserved_tokens) {
            std::vector<llama_token> preserved=common_tokenize(vocab,text,false,true);
            if (preserved.size()==1){sampling.preserved_tokens.insert(preserved[0]);}
        }
        SamplerGuard sampler{common_sampler_init(model,sampling)};
        if (sampler.sampler==nullptr){setError("Cannot make the sampler");return RESULT_ERROR;}

        //KV cache: cache_prompt keeps the common beginning of the previous prompt, so it is not calculated again
        llama_memory_t mem=llama_get_memory(ctx);
        size_t nCached=0;
        if (jsonValue<bool>(params,"cache_prompt",false)) {
            while (nCached<kvTokens.size()&&nCached<tokens.size()&&kvTokens[nCached]==tokens[nCached]){nCached++;}
            //The logits of the last token are needed, so at least one token must be decoded
            if (nCached==tokens.size()){nCached--;}
        }
        if (nCached==0||!llama_memory_seq_rm(mem,0,(llama_pos)nCached,-1)) {
            llama_memory_clear(mem,false);
            nCached=0;
        }
        kvTokens.resize(nCached);

        int64_t tPrompt=ggml_time_ms(); //PERF
        LOGP("prompt start: %zu tokens, %zu cached, n_batch=%d, %lld ms since start",tokens.size(),nCached,nBatch,(long long)(tPrompt-tStart)); //PERF
        int result=decodePrompt(tokens,nCached);
        int64_t promptMs=ggml_time_ms()-tPrompt; //PERF
        LOGP("prompt done: %zu tokens in %lld ms (%.2f tok/s)",tokens.size()-nCached,(long long)promptMs,promptMs>0?(tokens.size()-nCached)*1000.0/promptMs:0.0); //PERF
        if (result!=0) {
            //The state of the cache is not known after an error
            llama_memory_clear(mem,false);
            kvTokens.clear();
            return result;
        }

        //Generation
        int nKeep=jsonValue<int>(params,"n_keep",0);
        int maxTokens=jsonValue<int>(params,"max_tokens",-1);
        int count=0;
        std::string generated,pendingContent,pendingReasoning;
        common_chat_msg message;

        //Sends the deltas to Java. isFlush - the end of the answer, so send everything.
        auto send=[&](bool isFlush)->bool {
            std::string content,reasoning;
            if (isFlush) {
                content=pendingContent;
                reasoning=pendingReasoning;
                pendingContent.clear();
                pendingReasoning.clear();
            } else {
                size_t l=utf8CompleteLength(pendingContent);
                content=pendingContent.substr(0,l);
                pendingContent.erase(0,l);
                l=utf8CompleteLength(pendingReasoning);
                reasoning=pendingReasoning.substr(0,l);
                pendingReasoning.erase(0,l);
            }
            if (content.empty()&&reasoning.empty()){return true;}
            jstring jContent=utf8ToJstring(env,content);
            jstring jReasoning=utf8ToJstring(env,reasoning);
            env->CallVoidMethod(callback,onData,jContent,jReasoning);
            env->DeleteLocalRef(jContent);
            env->DeleteLocalRef(jReasoning);
            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
                env->ExceptionClear();
                setError("StreamCallback.onData threw an exception");
                return false;
            }
            return true;
        };

        //Parses the text of the answer and finds what is new since the previous call
        auto update=[&](bool isPartial) {
            common_chat_msg newMessage;
            try {
                newMessage=common_chat_parse(generated,isPartial,parser);
            } catch (const std::exception& e) {
                //The full text cannot be parsed, so use what the partial parser can get
                newMessage=common_chat_parse(generated,true,parser);
            }
            if (newMessage.empty()){return;}
            for (const common_chat_msg_diff& diff:common_chat_msg_diff::compute_diffs(message,newMessage)) {
                pendingContent+=diff.content_delta;
                pendingReasoning+=diff.reasoning_content_delta;
            }
            message=newMessage;
        };

        result=RESULT_DONE;
        int64_t tGen=ggml_time_ms(),sampleMs=0,decodeMs=0,parseMs=0,sendMs=0,tStep; //PERF
        while (true) {
            if (isStop){result=RESULT_STOPPED;break;}
            if (maxTokens>=0&&count>=maxTokens){result=RESULT_LENGTH;break;}
            if ((int)kvTokens.size()+1>=nCtx&&!shiftContext(nKeep)){result=RESULT_LENGTH;break;}

            tStep=ggml_time_ms(); //PERF
            llama_token token=common_sampler_sample(sampler.sampler,ctx,-1);
            common_sampler_accept(sampler.sampler,token,true);
            sampleMs+=ggml_time_ms()-tStep; //PERF
            if (count==0){LOGP("first token: %lld ms since start",(long long)(ggml_time_ms()-tStart));} //PERF
            if (llama_vocab_is_eog(vocab,token)){break;}

            generated+=common_token_to_piece(ctx,token,sampling.preserved_tokens.count(token)>0);

            //Stop strings of the template
            bool isStopString=false;
            for (const std::string& stop:chat.additional_stops) {
                if (!stop.empty()&&generated.size()>=stop.size()&&generated.compare(generated.size()-stop.size(),stop.size(),stop)==0) {
                    generated.resize(generated.size()-stop.size());
                    isStopString=true;
                    break;
                }
            }

            tStep=ggml_time_ms(); //PERF
            update(true);
            parseMs+=ggml_time_ms()-tStep; //PERF
            tStep=ggml_time_ms(); //PERF
            if (!send(false)){result=RESULT_ERROR;break;}
            sendMs+=ggml_time_ms()-tStep; //PERF
            if (isStopString){break;}

            common_batch_clear(batch);
            common_batch_add(batch,token,(llama_pos)kvTokens.size(),{0},true);
            tStep=ggml_time_ms(); //PERF
            int r=llama_decode(ctx,batch);
            int64_t stepMs=ggml_time_ms()-tStep; //PERF
            decodeMs+=stepMs; //PERF
            LOGP("token %d: decode %lld ms",count+1,(long long)stepMs); //PERF
            if (r!=0) {
                if (isStop){result=RESULT_STOPPED;}
                else {setError("llama_decode failed: "+std::to_string(r));result=RESULT_ERROR;}
                //The state of the cache is not known after an error
                llama_memory_clear(mem,false);
                kvTokens.clear();
                break;
            }
            kvTokens.push_back(token);
            count++;
        }

        if (result!=RESULT_ERROR) {
            update(false);
            if (!send(true)){result=RESULT_ERROR;}
        }
        int64_t genMs=ggml_time_ms()-tGen; //PERF
        LOGP("generation: %d tokens in %lld ms (%.2f tok/s), sample %lld, decode %lld, parse %lld, send %lld ms; total %lld ms, result %d",
             count,(long long)genMs,genMs>0?count*1000.0/genMs:0.0,(long long)sampleMs,(long long)decodeMs,(long long)parseMs,(long long)sendMs,(long long)(ggml_time_ms()-tStart),result); //PERF
        return result;
    }

    std::string modelInfo() const {
        if (model==nullptr){return "";}
        char description[256]={0};
        llama_model_desc(model,description,sizeof(description));
        return std::string(description)+" | "+std::to_string(llama_model_size(model)/(1024*1024))+" MiB | "
            +std::to_string(llama_model_n_params(model)/1000000)+"M params | n_ctx "+std::to_string(nCtx);
    }
};

static LLamaSession* fromHandle(jlong handle) {
    return (LLamaSession*)(intptr_t)handle;
}

//backends: the folder with libggml-cpu-*.so (nativeLibraryDir), they are chosen by the CPU at the runtime
LLAMA_JNI(void,init)(JNIEnv* env, jclass cls, jstring backendsDir) {
    std::string dir=jstringToUtf8(env,backendsDir);
    std::call_once(backendInitFlag,[&dir](){
        llama_log_set(androidLog,nullptr);
        ggml_backend_load_all_from_path(dir.c_str());
        llama_backend_init();
        LOGI("llama.cpp is initialized, backends: %zu, devices: %zu",ggml_backend_reg_count(),ggml_backend_dev_count());
        if (ggml_backend_dev_count()==0){LOGE("No ggml backend is loaded from %s, the libraries must be extracted",dir.c_str());}
    });
}

LLAMA_JNI(jlong,create)(JNIEnv* env, jclass cls) {
    return (jlong)(intptr_t)new LLamaSession();
}

LLAMA_JNI(void,destroy)(JNIEnv* env, jclass cls, jlong handle) {
    delete fromHandle(handle);
}

//0 - ok, <0 - error (getLastError)
LLAMA_JNI(jint,loadModel)(JNIEnv* env, jclass cls, jlong handle, jobjectArray files, jstring paramsJson) {
    LLamaSession* session=fromHandle(handle);
    try {
        std::vector<std::string> paths;
        jsize count=env->GetArrayLength(files);
        for (jsize i=0;i<count;i++) {
            jstring path=(jstring)env->GetObjectArrayElement(files,i);
            paths.push_back(jstringToUtf8(env,path));
            env->DeleteLocalRef(path);
        }
        std::string text=jstringToUtf8(env,paramsJson);
        return session->load(paths,json::parse(text.empty()?"{}":text));
    } catch (const std::exception& e) {
        session->setError(std::string("loadModel: ")+e.what());
        session->unload();
        return RESULT_ERROR;
    }
}

LLAMA_JNI(void,unloadModel)(JNIEnv* env, jclass cls, jlong handle) {
    fromHandle(handle)->unload();
}

//RESULT_DONE(0), RESULT_STOPPED(1), RESULT_LENGTH(2) or <0 - error (getLastError)
LLAMA_JNI(jint,chatCompletion)(JNIEnv* env, jclass cls, jlong handle, jstring messagesJson, jstring paramsJson, jobject callback) {
    LLamaSession* session=fromHandle(handle);
    try {
        std::string params=jstringToUtf8(env,paramsJson);
        return session->chatCompletion(env,json::parse(jstringToUtf8(env,messagesJson)),json::parse(params.empty()?"{}":params),callback);
    } catch (const std::exception& e) {
        session->setError(std::string("chatCompletion: ")+e.what());
        return RESULT_ERROR;
    }
}

LLAMA_JNI(void,stop)(JNIEnv* env, jclass cls, jlong handle) {
    fromHandle(handle)->isStop=true;
}

LLAMA_JNI(jstring,getModelInfo)(JNIEnv* env, jclass cls, jlong handle) {
    return utf8ToJstring(env,fromHandle(handle)->modelInfo());
}

LLAMA_JNI(jstring,getLastError)(JNIEnv* env, jclass cls, jlong handle) {
    return utf8ToJstring(env,fromHandle(handle)->lastError);
}
