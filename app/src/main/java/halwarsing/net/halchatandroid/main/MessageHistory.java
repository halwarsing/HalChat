package halwarsing.net.halchatandroid.main;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import halwarsing.net.halchatandroid.type.HCMessage;

final class MessageHistory {
    private static final Comparator<HCMessage> BY_MESSAGE_ID =
            Comparator.comparingLong(HCMessage::getMsgUID);

    private MessageHistory() {
    }

    static ArrayList<HCMessage> sortedUnique(List<HCMessage> messages) {
        ArrayList<HCMessage> result = new ArrayList<>();
        Set<Long> seenIds = new HashSet<>();

        for (HCMessage message : messages) {
            if (message != null && seenIds.add(message.getMsgUID())) {
                result.add(message);
            }
        }

        result.sort(BY_MESSAGE_ID);
        return result;
    }

    static HCMessage oldestRealMessage(List<HCMessage> messages) {
        for (HCMessage message : messages) {
            if (message.type != -1) {
                return message;
            }
        }
        return null;
    }

    static HCMessage newestRealMessage(List<HCMessage> messages) {
        for (int i = messages.size() - 1; i >= 0; i--) {
            HCMessage message = messages.get(i);
            if (message.type != -1) {
                return message;
            }
        }
        return null;
    }
}
