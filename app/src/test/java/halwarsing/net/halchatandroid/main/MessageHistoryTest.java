package halwarsing.net.halchatandroid.main;

import static org.junit.Assert.assertEquals;

import org.junit.Test;

import java.util.Arrays;
import java.util.List;

import halwarsing.net.halchatandroid.type.HCMessage;

public class MessageHistoryTest {
    @Test
    public void sortedUniqueKeepsOneMessagePerIdInAscendingOrder() {
        List<HCMessage> result=MessageHistory.sortedUnique(Arrays.asList(
                message(30,0),
                message(10,0),
                message(20,0),
                message(20,0)
        ));

        assertEquals(3,result.size());
        assertEquals(10,result.get(0).msgId);
        assertEquals(20,result.get(1).msgId);
        assertEquals(30,result.get(2).msgId);
    }

    @Test
    public void historyBoundsIgnoreSyntheticStartMessage() {
        HCMessage start=message(0,-1);
        HCMessage old=message(100,0);
        HCMessage newest=message(300,0);
        List<HCMessage> messages=Arrays.asList(start,old,message(200,0),newest);

        assertEquals(old,MessageHistory.oldestRealMessage(messages));
        assertEquals(newest,MessageHistory.newestRealMessage(messages));
    }

    private static HCMessage message(long msgId,int type) {
        return new HCMessage(
                -1,msgId,1,2,3,-1,-1,"",
                null,"-1",null,"-1",new byte[0],
                false,true,true,type,false,false,
                null,0,0,false,1
        );
    }
}
