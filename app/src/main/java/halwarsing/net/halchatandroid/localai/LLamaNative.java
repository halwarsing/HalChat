package halwarsing.net.halchatandroid.localai;

/**
 * Raw JNI of llama.cpp (hal_llama_jni.cpp). It repeats what HalChatWeb uses from wllama:
 * loadModel - wllama.loadModel(modelFiles,loadParams), chatCompletion - wllama.createChatCompletion(params) with
 * stream:true, stop - abortController.abort().
 * <p>
 * The params are JSON strings with the same keys as in wllama:
 * <ul>
 * <li>loadModel: n_ctx, n_batch, n_threads, n_gpu_layers, flash_attn, offload_kqv, cache_type_k, cache_type_v,
 * lora_adapters [{path,scale}], reasoning_format, default_template_kwargs {enable_thinking}, ctx_shift</li>
 * <li>chatCompletion: max_tokens, temperature, top_k, top_p, min_p, penalty_repeat, penalty_present, penalty_freq,
 * seed, cache_prompt, n_keep, reasoning_format, chat_template_kwargs</li>
 * </ul>
 * The messages are a JSON array like in wllama: [{"role":"system","content":"..."},{"role":"user","content":"..."}].
 * <p>
 * One handle = one model. Call loadModel/chatCompletion from one thread (they block); stop can be called from any thread.
 */
public final class LLamaNative {
    /** onData(chunk) of wllama: the deltas of the answer (delta.content and delta.reasoning_content). */
    public interface StreamCallback {
        void onData(String content, String reasoning);
    }

    /** The results of {@link #chatCompletion}, a negative number is an error (see {@link #getLastError}). */
    public static final int RESULT_DONE = 0;
    public static final int RESULT_STOPPED = 1;
    public static final int RESULT_LENGTH = 2;

    static {
        System.loadLibrary("hal_llama");
    }

    private LLamaNative() {
    }

    /**
     * Loads the backends of ggml. backendsDir is context.getApplicationInfo().nativeLibraryDir (the libraries must be
     * extracted: android:extractNativeLibs="true" / useLegacyPackaging). It is safe to call it again.
     */
    public static native void init(String backendsDir);

    public static native long create();

    public static native void destroy(long handle);

    /** files is one file or the parts of one model (in the right order). 0 - ok, a negative number - error. */
    public static native int loadModel(long handle, String[] files, String paramsJson);

    public static native void unloadModel(long handle);

    /** It blocks until the answer is ready, the deltas come to callback from the same thread. */
    public static native int chatCompletion(long handle, String messagesJson, String paramsJson, StreamCallback callback);

    public static native void stop(long handle);

    public static native String getModelInfo(long handle);

    public static native String getLastError(long handle);
}
