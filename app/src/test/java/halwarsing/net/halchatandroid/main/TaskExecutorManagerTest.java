package halwarsing.net.halchatandroid.main;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertSame;

import org.junit.Test;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

public class TaskExecutorManagerTest {

    @Test
    public void uniqueTaskIsSharedOnlyWhileItIsRunning() throws Exception {
        TaskExecutorManager manager = TaskExecutorManager.getInstance();
        String tag = "test-unique-task-" + System.nanoTime();
        AtomicInteger calls = new AtomicInteger();
        CountDownLatch started = new CountDownLatch(1);
        CountDownLatch release = new CountDownLatch(1);

        Future<Integer> first = manager.submitChatSync(tag, () -> {
            started.countDown();
            release.await(5, TimeUnit.SECONDS);
            return calls.incrementAndGet();
        });

        started.await(5, TimeUnit.SECONDS);
        Future<Integer> duplicate = manager.submitChatSync(tag, calls::incrementAndGet);

        assertSame(first, duplicate);
        release.countDown();
        assertEquals(1, (int) first.get(5, TimeUnit.SECONDS));

        Future<Integer> next = manager.submitChatSync(tag, calls::incrementAndGet);
        assertEquals(2, (int) next.get(5, TimeUnit.SECONDS));
    }

    @Test
    public void completedUploadDoesNotKeepItsTagForever() throws Exception {
        TaskExecutorManager manager = TaskExecutorManager.getInstance();
        String tag = "test-upload-task-" + System.nanoTime();
        AtomicInteger calls = new AtomicInteger();

        assertEquals(
                1,
                (int) manager.submitUpload(tag, calls::incrementAndGet)
                        .get(5, TimeUnit.SECONDS)
        );
        assertEquals(
                2,
                (int) manager.submitUpload(tag, calls::incrementAndGet)
                        .get(5, TimeUnit.SECONDS)
        );
    }

    @Test
    public void completableTaskIsDeduplicatedAndReleased() throws Exception {
        TaskExecutorManager manager = TaskExecutorManager.getInstance();
        String tag = "test-completable-task-" + System.nanoTime();
        AtomicInteger calls = new AtomicInteger();
        CountDownLatch started = new CountDownLatch(1);
        CountDownLatch release = new CountDownLatch(1);

        CompletableFuture<Integer> first = manager.submitCompletableUserSync(tag, () -> {
            started.countDown();
            release.await(5, TimeUnit.SECONDS);
            return calls.incrementAndGet();
        });

        started.await(5, TimeUnit.SECONDS);
        CompletableFuture<Integer> duplicate =
                manager.submitCompletableUserSync(tag, calls::incrementAndGet);

        assertSame(first, duplicate);
        release.countDown();
        assertEquals(1, (int) first.get(5, TimeUnit.SECONDS));
        assertEquals(
                2,
                (int) manager.submitCompletableUserSync(tag, calls::incrementAndGet)
                        .get(5, TimeUnit.SECONDS)
        );
    }
}
