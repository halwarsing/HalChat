package halwarsing.net.halchatandroid.type;

import java.util.Comparator;

public class MessageReaction {
    public static final Comparator<MessageReaction> DISPLAY_ORDER =
            Comparator.comparingLong((MessageReaction reaction) -> reaction.count)
                    .reversed()
                    .thenComparingLong(reaction -> reaction.emojiId);

    public final long emojiId;
    public final long count;

    public MessageReaction(long emojiId, long count) {
        this.emojiId = emojiId;
        this.count = count;
    }
}
