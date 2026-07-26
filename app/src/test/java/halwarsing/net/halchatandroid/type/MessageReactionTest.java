package halwarsing.net.halchatandroid.type;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import org.junit.Test;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class MessageReactionTest {
    @Test
    public void displayOrderSortsByCountDescendingThenEmojiIdAscending() {
        List<MessageReaction> reactions=new ArrayList<>(Arrays.asList(
                new MessageReaction(9,2),
                new MessageReaction(7,5),
                new MessageReaction(3,5),
                new MessageReaction(1,1)
        ));

        reactions.sort(MessageReaction.DISPLAY_ORDER);

        assertEquals(3,reactions.get(0).emojiId);
        assertEquals(7,reactions.get(1).emojiId);
        assertEquals(9,reactions.get(2).emojiId);
        assertEquals(1,reactions.get(3).emojiId);
    }

    @Test
    public void messageStoresSortedPositiveReactionsAndOwnSelection() {
        HCMessage message=new HCMessage(
                -1,10,20,30,40,-1,-1,"",null,
                "-1","-1","-1",new byte[0],false,true,true,
                0,false,true,null,0,0,false,1
        );

        message.setReactions(Arrays.asList(
                new MessageReaction(8,1),
                new MessageReaction(4,0),
                new MessageReaction(5,3)
        ),5);

        assertEquals(2,message.reactions.size());
        assertEquals(5,message.reactions.get(0).emojiId);
        assertEquals(8,message.reactions.get(1).emojiId);
        assertEquals(5,message.selectedReaction);
        assertTrue(message.hasReactionData);
    }

    @Test
    public void optimisticReactionAddsSwitchesAndRemovesOwnVote() {
        HCMessage message=new HCMessage(
                -1,10,20,30,40,-1,-1,"",null,
                "-1","-1","-1",new byte[0],false,true,true,
                0,false,true,null,0,0,false,1
        );
        message.setReactions(Arrays.asList(
                new MessageReaction(5,3),
                new MessageReaction(8,1)
        ),-1);

        message.applyReaction(8);
        assertEquals(2,message.reactions.get(1).count);
        assertEquals(8,message.selectedReaction);

        message.applyReaction(5);
        assertEquals(4,message.reactions.get(0).count);
        assertEquals(5,message.selectedReaction);

        message.applyReaction(5);
        assertEquals(3,message.reactions.get(0).count);
        assertEquals(-1,message.selectedReaction);
    }

    @Test
    public void pollUsesDecryptedVariantsForImmediateDisplay() throws Exception {
        HCMessage message=new HCMessage(
                -1,10,20,30,40,-1,-1,"",null,
                "-1","-1","-1",new byte[0],false,true,true,
                4,false,true,null,0,0,false,1
        );
        message.decryptedPollVariants=Arrays.asList("Первый вариант","Второй вариант");

        assertEquals("Первый вариант",message.getPollVariants().get(0));
        assertEquals("Второй вариант",message.getPollVariants().get(1));

        message.decryptedPollVariants=null;
        assertNull(message.getPollVariants());
    }
}
