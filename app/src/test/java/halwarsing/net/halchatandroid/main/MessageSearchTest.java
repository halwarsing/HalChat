package halwarsing.net.halchatandroid.main;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import org.junit.Test;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

public class MessageSearchTest {
    @Test
    public void searchIgnoresCaseAndOuterSpaces() {
        assertTrue(MessageSearch.matches("Привет, HalChat!", "  halchat "));
        assertTrue(MessageSearch.matches("НОВОЕ сообщение", "новое"));
    }

    @Test
    public void blankOrMissingTextDoesNotMatch() {
        assertFalse(MessageSearch.matches("Сообщение", "   "));
        assertFalse(MessageSearch.matches(null, "сообщение"));
    }

    @Test
    public void normalizedIndexTextCanBeReusedForSeveralQueries() {
        String indexedText=MessageSearch.normalizeText("Быстрый поиск сообщений");

        assertTrue(MessageSearch.normalizedTextMatches(indexedText,"быстрый"));
        assertTrue(MessageSearch.normalizedTextMatches(indexedText,"сообщ"));
        assertFalse(MessageSearch.normalizedTextMatches(indexedText,"медленный"));
    }

    @Test
    public void indexBatchDoesNotShiftAlreadySelectedResult() {
        List<Long> results=new ArrayList<>(Arrays.asList(90L,80L,70L,60L,50L,40L));
        long selectedMessageId=results.get(2);
        Map<Long,String> nextBatch=new LinkedHashMap<>();
        nextBatch.put(30L,"искомое сообщение");
        nextBatch.put(20L,"искомое сообщение");
        nextBatch.put(10L,"искомое сообщение");

        MessageSearch.appendMatchingIds(results,nextBatch,"искомое");

        assertEquals(9,results.size());
        assertEquals(selectedMessageId,results.get(2).longValue());
    }
}
