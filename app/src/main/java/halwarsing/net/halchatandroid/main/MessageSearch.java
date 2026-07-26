












package halwarsing.net.halchatandroid.main;

import java.util.Locale;
import java.util.List;
import java.util.Map;

final class MessageSearch {
    private MessageSearch() {
    }

    static String normalizeQuery(CharSequence query) {
        return query == null ? "" : query.toString().trim().toLowerCase(Locale.ROOT);
    }

    static String normalizeText(String text) {
        return text == null ? "" : text.toLowerCase(Locale.ROOT);
    }

    static boolean matchesNormalizedQuery(String text, String normalizedQuery) {
        return !normalizedQuery.isEmpty()
                && normalizeText(text).contains(normalizedQuery);
    }

    static boolean normalizedTextMatches(String normalizedText,String normalizedQuery) {
        return normalizedText!=null
                && !normalizedQuery.isEmpty()
                && normalizedText.contains(normalizedQuery);
    }

    static void appendMatchingIds(
            List<Long> resultIds,
            Map<Long,String> indexedBatch,
            String normalizedQuery
    ) {
        for(Map.Entry<Long,String> entry:indexedBatch.entrySet()) {
            if(normalizedTextMatches(entry.getValue(),normalizedQuery)
                    && !resultIds.contains(entry.getKey())) {
                resultIds.add(entry.getKey());
            }
        }
    }

    static boolean matches(String text, CharSequence query) {
        return matchesNormalizedQuery(text, normalizeQuery(query));
    }
}
