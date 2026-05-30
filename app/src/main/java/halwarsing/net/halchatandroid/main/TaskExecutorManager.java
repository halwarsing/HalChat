package halwarsing.net.halchatandroid.main;

import java.util.concurrent.*;
import java.util.*;

public class TaskExecutorManager {

    private static final int CORE_POOL_SIZE = Math.max(2, Runtime.getRuntime().availableProcessors());
    private static final int DOWNLOAD_THREADS = 4;
    private static final int DECRYPT_THREADS = 2;
    private static final int AUDIO_THREADS = 2;
    //transferring passwords between users
    private static final int PASSWORD_T_THREADS = 2;
    //Synchronization chats
    private static final int CHATS_SYNC_THREADS = 2;
    //HalNet and HalChat Users sync
    private static final int USERS_SYNC_THREADS = 1;
    //Uploading files
    private static final int UPLOAD_THREADS = 2;

    //Send
    private static final int SEND_THREADS = 2;

    //API (HTTP)
    private static final int API_THREADS = 3;

    //Decrypt messages in ChatActivity (HIGH PRIORITY)
    private static final int DECRYPT_CHATACTIVITY_THREADS = CORE_POOL_SIZE;

    private final ThreadPoolExecutor downloadExecutor;
    private final ThreadPoolExecutor decryptExecutor;
    private final ThreadPoolExecutor audioExecutor;
    private final ThreadPoolExecutor passwordTExecutor;
    private final ThreadPoolExecutor chatsSync;
    private final ThreadPoolExecutor usersSync;
    private final ThreadPoolExecutor uploadExecutor;
    private final ThreadPoolExecutor sendExecutor;
    private final ThreadPoolExecutor apiExecutor;
    private final ThreadPoolExecutor chatActivityDecrypt;

    private final Map<String, List<Future<?>>> taskMap = new ConcurrentHashMap<>();
    private final Map<String, List<CompletableFuture<?>>> completableFutureMap=new ConcurrentHashMap<>();

    private static TaskExecutorManager instance;

    private TaskExecutorManager() {
        downloadExecutor = createExecutor(DOWNLOAD_THREADS);
        decryptExecutor = createExecutor(DECRYPT_THREADS);
        audioExecutor = createExecutor(AUDIO_THREADS);
        passwordTExecutor=createExecutor(PASSWORD_T_THREADS);
        chatsSync=createExecutor(CHATS_SYNC_THREADS);
        usersSync=createExecutor(USERS_SYNC_THREADS);
        uploadExecutor=createExecutor(UPLOAD_THREADS);
        sendExecutor=createExecutor(SEND_THREADS);
        apiExecutor=createExecutor(API_THREADS);
        chatActivityDecrypt=createExecutor(DECRYPT_CHATACTIVITY_THREADS);
    }

    private ThreadPoolExecutor createExecutor(int threads) {
        return new ThreadPoolExecutor(
                threads,
                threads,
                10L, TimeUnit.MINUTES,
                new LinkedBlockingQueue<>()
        );
    }

    public static synchronized TaskExecutorManager getInstance() {
        if (instance == null) {
            instance = new TaskExecutorManager();
        }
        return instance;
    }

    public <T> CompletableFuture<T> submitDownload(String tag, Callable<T> task) {
        return submitCompletableTask(downloadExecutor, tag, task);
    }

    public <T> Future<T> submitUpload(String tag, Callable<T> task) {
        if(taskMap.containsKey(tag)&&!taskMap.get(tag).isEmpty())return (Future<T>) taskMap.get(tag).get(0);
        return submitTask(uploadExecutor, tag, task);
    }

    public <T> Future<T> submitDecrypt(String tag, Callable<T> task) {
        return submitTask(decryptExecutor, tag, task);
    }

    public <T> Future<T> submitAudio(String tag, Callable<T> task) {
        return submitTask(audioExecutor, tag, task);
    }

    public <T> Future<T> submitPasswordT(String tag, Callable<T> task) {
        return submitTask(passwordTExecutor, tag, task);
    }

    public <T> Future<T> submitChatSync(String tag, Callable<T> task) {
        if(taskMap.containsKey(tag)&&!taskMap.get(tag).isEmpty())return (Future<T>) taskMap.get(tag).get(0);
        return submitTask(chatsSync, tag, task);
    }

    public <T> CompletableFuture<T> submitCompletableUserSync(String tag, Callable<T> task) {
        return submitCompletableTask(usersSync, tag, task);
    }

    public <T> Future<T> submitSend(String tag, Callable<T> task) {
        return submitTask(sendExecutor, tag, task);
    }

    public <T> Future<T> submitAPI(String tag, Callable<T> task) {
        return submitTask(apiExecutor, tag, task);
    }

    private <T> Future<T> submitTask(ThreadPoolExecutor executor, String tag, Callable<T> task) {
        Future<T> future = executor.submit(task);
        taskMap.computeIfAbsent(tag, k -> Collections.synchronizedList(new ArrayList<>())).add(future);
        return future;
    }

    public <T> Future<T> submitDecryptChatActivity(String tag, Callable<T> task) {
        return submitTask(chatActivityDecrypt, tag, task);
    }

    private <T> CompletableFuture<T> submitCompletableTask(ThreadPoolExecutor executor, String tag, Callable<T> task) {
        List<CompletableFuture<?>> existing = completableFutureMap.get(tag);
        if (existing != null && !existing.isEmpty()) {
            @SuppressWarnings("unchecked")
            CompletableFuture<T> existingFuture = (CompletableFuture<T>) existing.get(0);
            return existingFuture;
        }

        CompletableFuture<T> completableFuture = CompletableFuture.supplyAsync(() -> {
            try {
                return task.call();
            } catch (Exception e) {
                throw new CompletionException(e);
            }
        }, executor);

        completableFutureMap.computeIfAbsent(tag, k -> Collections.synchronizedList(new ArrayList<>())).add(completableFuture);
        return completableFuture;
    }

    // Отмена всех задач по тегу
    public void cancelTasks(String tag) {
        List<Future<?>> futures = taskMap.remove(tag);
        if (futures != null) {
            for (Future<?> f : futures) {
                f.cancel(true);
            }
        }
    }

    // Полная остановка всех потоков
    public void shutdownAll() {
        downloadExecutor.shutdownNow();
        decryptExecutor.shutdownNow();
        audioExecutor.shutdownNow();
        passwordTExecutor.shutdownNow();
        chatsSync.shutdownNow();
        usersSync.shutdownNow();
        uploadExecutor.shutdownNow();
        sendExecutor.shutdownNow();
        apiExecutor.shutdownNow();
        taskMap.clear();
    }
}