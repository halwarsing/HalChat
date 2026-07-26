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

    //Ordered processing of incoming WebSocket events
    private static final int REALTIME_THREADS = 1;

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
    private final ThreadPoolExecutor realtimeExecutor;
    private final ThreadPoolExecutor chatActivityDecrypt;

    private final Map<String, List<Future<?>>> taskMap = new ConcurrentHashMap<>();
    private final Map<String, CompletableFuture<?>> completableFutureMap=new ConcurrentHashMap<>();
    private final Map<String, Future<?>> uniqueTaskMap = new ConcurrentHashMap<>();

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
        realtimeExecutor=createExecutor(REALTIME_THREADS);
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
        return submitUniqueTask(uploadExecutor, tag, task);
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
        return submitUniqueTask(chatsSync, tag, task);
    }

    public <T> CompletableFuture<T> submitCompletableChatSync(String tag, Callable<T> task) {
        return submitCompletableTask(chatsSync, tag, task);
    }

    public <T> CompletableFuture<T> submitCompletableChatSyncTask(Callable<T> task) {
        CompletableFuture<T> future=new CompletableFuture<>();
        try {
            chatsSync.execute(()->{
                try {
                    future.complete(task.call());
                } catch(Exception error) {
                    future.completeExceptionally(error);
                }
            });
        } catch(RejectedExecutionException error) {
            future.completeExceptionally(error);
        }
        return future;
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

    public <T> Future<T> submitRealtime(String tag, Callable<T> task) {
        return submitTask(realtimeExecutor, tag, task);
    }

    private <T> Future<T> submitTask(ThreadPoolExecutor executor, String tag, Callable<T> task) {
        FutureTask<T> future = createTrackedFuture(tag, task);
        List<Future<?>> tasks = taskMap.computeIfAbsent(
                tag,
                k -> Collections.synchronizedList(new ArrayList<>())
        );
        tasks.add(future);
        executeTrackedFuture(executor, tag, future);
        return future;
    }

    public <T> Future<T> submitDecryptChatActivity(String tag, Callable<T> task) {
        return submitTask(chatActivityDecrypt, tag, task);
    }

    @SuppressWarnings("unchecked")
    private <T> Future<T> submitUniqueTask(ThreadPoolExecutor executor, String tag, Callable<T> task) {
        while (true) {
            Future<?> existing = uniqueTaskMap.get(tag);
            if (existing != null && !existing.isDone() && !existing.isCancelled()) {
                return (Future<T>) existing;
            }

            FutureTask<T> future = new FutureTask<>(task) {
                @Override
                protected void done() {
                    uniqueTaskMap.remove(tag, this);
                }
            };

            boolean installed = existing == null
                    ? uniqueTaskMap.putIfAbsent(tag, future) == null
                    : uniqueTaskMap.replace(tag, existing, future);

            if (installed) {
                try {
                    executor.execute(future);
                } catch (RejectedExecutionException error) {
                    uniqueTaskMap.remove(tag, future);
                    throw error;
                }
                return future;
            }
        }
    }

    private <T> FutureTask<T> createTrackedFuture(String tag, Callable<T> task) {
        return new FutureTask<>(task) {
            @Override
            protected void done() {
                removeTrackedFuture(tag, this);
            }
        };
    }

    private void executeTrackedFuture(ThreadPoolExecutor executor, String tag, FutureTask<?> future) {
        try {
            executor.execute(future);
        } catch (RejectedExecutionException error) {
            removeTrackedFuture(tag, future);
            throw error;
        }
    }

    private void removeTrackedFuture(String tag, Future<?> future) {
        List<Future<?>> tasks = taskMap.get(tag);
        if (tasks == null) {
            return;
        }

        synchronized (tasks) {
            tasks.remove(future);
            if (tasks.isEmpty()) {
                taskMap.remove(tag, tasks);
            }
        }
    }

    @SuppressWarnings("unchecked")
    private <T> CompletableFuture<T> submitCompletableTask(ThreadPoolExecutor executor, String tag, Callable<T> task) {
        while (true) {
            CompletableFuture<?> existing = completableFutureMap.get(tag);
            if (existing != null && !existing.isDone() && !existing.isCancelled()) {
                return (CompletableFuture<T>) existing;
            }

            CompletableFuture<T> future = new CompletableFuture<>();
            boolean installed = existing == null
                    ? completableFutureMap.putIfAbsent(tag, future) == null
                    : completableFutureMap.replace(tag, existing, future);

            if (installed) {
                try {
                    executor.execute(() -> {
                        try {
                            if (!future.isCancelled()) {
                                future.complete(task.call());
                            }
                        } catch (Exception error) {
                            future.completeExceptionally(error);
                        } finally {
                            completableFutureMap.remove(tag, future);
                        }
                    });
                } catch (RejectedExecutionException error) {
                    completableFutureMap.remove(tag, future);
                    future.completeExceptionally(error);
                }
                return future;
            }
        }
    }

    // Отмена всех задач по тегу
    public void cancelTasks(String tag) {
        Future<?> uniqueFuture = uniqueTaskMap.remove(tag);
        if (uniqueFuture != null) {
            uniqueFuture.cancel(true);
        }

        List<Future<?>> futures = taskMap.remove(tag);
        if (futures != null) {
            synchronized (futures) {
                for (Future<?> f : futures) {
                    f.cancel(true);
                }
            }
        }

        CompletableFuture<?> completableFuture = completableFutureMap.remove(tag);
        if (completableFuture != null) {
            completableFuture.cancel(true);
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
        realtimeExecutor.shutdownNow();
        chatActivityDecrypt.shutdownNow();
        taskMap.clear();
        completableFutureMap.clear();
        uniqueTaskMap.clear();
    }
}
