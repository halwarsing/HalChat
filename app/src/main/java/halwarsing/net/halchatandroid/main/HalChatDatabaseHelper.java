package halwarsing.net.halchatandroid.main;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.os.Build;
import android.util.Log;

//Инициализация sql баз данных, создание новых и обновление старых
public class HalChatDatabaseHelper extends SQLiteOpenHelper {
    private static final String DATABASE_NAME = "halchat.db";
    private static final int DATABASE_VERSION = 10;
    private static final String TAG = "HCDBC";
    SQLiteDatabase db;
    String[] UIDNames=new String[]{"msg","chats","chatUsers","chatActions","actions"};

    public HalChatDatabaseHelper(Context context) {
        super(context,DATABASE_NAME,null,DATABASE_VERSION);
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        this.db=db;
        Log.d(TAG,"Create Database");
        Log.e(TAG,"OnCreate DB");
        try {
            String createSessionsTable = "CREATE TABLE `sessions` ( `uid` INTEGER primary key AUTOINCREMENT , `id` VARCHAR(100) NOT NULL , `fromId` BIGINT(21) NOT NULL , `selected` TINYINT(1) NOT NULL DEFAULT '1')";
            db.execSQL(createSessionsTable);
            String createFilesTable = "CREATE TABLE `files` ( `uid` INTEGER primary key AUTOINCREMENT, `id` VARCHAR(100) NOT NULL , " +
                    "`path` TEXT NOT NULL , `name` TEXT NOT NULL , `fromId` BIGINT(21) NOT NULL, `updated` BIGINT(21) NOT NULL, `isFolder` TINYINT(1) NOT NULL DEFAULT '0')";
            db.execSQL(createFilesTable);
            String createUsersTable="CREATE TABLE `users` (`uid` INTEGER primary key AUTOINCREMENT, `id` BIGINT(21) NOT NULL," +
                    " `nickname` VARCHAR(50) NOT NULL, `icon` VARCHAR(100) NOT NULL, `isBot` TYNYINT(1) NOT NULL DEFAULT '0')";
            db.execSQL(createUsersTable);
            String createGroupChatsUsersTable = "CREATE TABLE `groupChatsUsers` ( `uid` INTEGER primary key AUTOINCREMENT, `id` BIGINT(21) NOT NULL," +
                    " `chatId` BIGINT(21) NOT NULL ," +
                    " `toId` BIGINT(21) NOT NULL , `permissions` TYNYINT NOT NULL, `isJoin` TINYINT(1) NOT NULL)";
            db.execSQL(createGroupChatsUsersTable);
            String createChatsTable = "CREATE TABLE `groupChats` (" +
                    "  `uid` INTEGER primary key AUTOINCREMENT," +
                    "  `chatUID` bigint(21) NOT NULL,"+
                    "  `id` varchar(50) NOT NULL," +
                    "  `name` varchar(100) NOT NULL," +
                    "  `icon` varchar(100) NOT NULL," +
                    "  `fromMe` tinyint(1) NOT NULL DEFAULT '0'," +
                    "  `created` bigint(21) NOT NULL," +
                    "  `publicType` tinyint NOT NULL DEFAULT '0'," +
                    "  `isAllowMessages` tinyint(1) NOT NULL DEFAULT '1'," +
                    "  `isDelete` tinyint(1) NOT NULL DEFAULT '0'," +
                    "  `chatType` tinyint NOT NULL DEFAULT '0'," +
                    "  `isAllowComments` tinyint(1) NOT NULL DEFAULT '1'," +
                    "  `password` varchar(100) NOT NULL DEFAULT '-1',"+
                    "  `lastMsgId` bigint(21) NOT NULL DEFAULT '-1',"+
                    "  `isEnd` tinyint(1) NOT NULL DEFAULT '0'"+
                    ")";
            db.execSQL(createChatsTable);
            String createMessagesTable = "CREATE TABLE `groupChatsMessages` (" +
                    "  `uid` INTEGER primary key AUTOINCREMENT," +
                    "  `msgId` bigint(21) NOT NULL,"+
                    "  `chatId` bigint(21) NOT NULL," +
                    "  `fromId` bigint(21) NOT NULL," +
                    "  `message` text NOT NULL," +
                    "  `encryptId` binary(8) NOT NULL," +
                    "  `attachments` text NOT NULL," +
                    "  `time` bigint(21) NOT NULL," +
                    "  `isDelete` tinyint(1) NOT NULL DEFAULT '0'," +
                    "  `answerMsg` bigint(21) NOT NULL DEFAULT '-1'," +
                    "  `commentMsg` bigint(21) NOT NULL DEFAULT '-1'," +
                    "  `type` smallint NOT NULL DEFAULT '0'," +
                    "  `soundMsg` varchar(100) NOT NULL DEFAULT '-1'," +
                    "  `dataBot` TEXT DEFAULT NULL," +
                    "  `recordMic` varchar(100) NOT NULL DEFAULT '-1'," +
                    "  `isSended` tinyint(1) NOT NULL DEFAULT '1',"+
                    "  `isReceived` tinyint(1) NOT NULL DEFAULT '0',"+
                    "  `isHalEnc` tinyint(1) NOT NULL DEFAULT '1',"+
                    "  `data` TEXT DEFAULT NULL,"+
                    "  `shareId` bigint(21) NOT NULL DEFAULT '0',"+
                    "  `pixelId` bigint(21) NOT NULL DEFAULT '0',"+
                    "  `isPinned` tinyint(1) NOT NULL DEFAULT '0',"+
                    "  `v` int(12) NOT NULL DEFAULT '0'"+
                    ")";
            db.execSQL(createMessagesTable);

            String createUIDsTable="CREATE TABLE `HCUID` (`uid` INTEGER primary key AUTOINCREMENT, `name` varchar(20) NOT NULL, `value` bigint(21) NOT NULL)";
            db.execSQL(createUIDsTable);

            createSendMessages(db);

            addAppSettings(db);

            //UIDs
            String name;
            for (long i=0;i<UIDNames.length;i++) {
                name=UIDNames[(int) i];
                addLastUid(i+1,name);
            }

            addRequestsPasswordDB(db);

            //Create IDX
            createIDX(db);

            //Emoji & Pixels
            createEmojiNPixels(db);

            Log.d(TAG,"Database tables created");
        } catch (Exception e) {
            Log.e(TAG,"Error creating",e);
        }
    }

    @Override
    public void onConfigure(SQLiteDatabase db) {
        super.onConfigure(db);

        db.enableWriteAheadLogging();
    }

    private void addAppSettings(SQLiteDatabase rdb) {
        //App settings
        String appSettingsTable="CREATE TABLE `SettingsApp` (" +
                "   `uid` INTEGER primary key AUTOINCREMENT,"+
                "   `fromId` bigint(21) NOT NULL,"+
                "   `key` VARCHAR(50) NOT NULL,"+
                "   `value` text NOT NULL,"+
                "   `isDelete` tinyint(1) NOT NULL DEFAULT '0'"+
                ")";
        rdb.execSQL(appSettingsTable);
    }


    private void addLastUid(long uid,String name) {
        db.execSQL("INSERT INTO `HCUID` (`uid`, `name`, `value`) VALUES (?, ?, -1)",new String[]{String.valueOf(uid),name});
    }

    //Необходимо для хранения приватных ключей сквозного шифрования для запроса пароля от чата
    private void addRequestsPasswordDB(SQLiteDatabase rdb) {
        String createRP="CREATE TABLE IF NOT EXISTS `requestsPassword` (`uid` INTEGER primary key AUTOINCREMENT, `chatId` bigint(21) NOT NULL, `psw` TEXT NOT NULL)";
        rdb.execSQL(createRP);
    }

    private void addEndChat(SQLiteDatabase rdb) {
        String addColumn="ALTER TABLE `groupChats` ADD `isEnd` TINYINT(1) NOT NULL DEFAULT '0'";
        rdb.execSQL(addColumn);
    }

    private void addNewMessageV2(SQLiteDatabase rdb) {
        rdb.execSQL("ALTER TABLE `groupChatsMessages` ADD COLUMN `data` TEXT DEFAULT NULL;");
        rdb.execSQL("ALTER TABLE `groupChatsMessages` ADD COLUMN `shareId` INTEGER NOT NULL DEFAULT '0';");
        rdb.execSQL("ALTER TABLE `groupChatsMessages` ADD COLUMN `pixelId` INTEGER NOT NULL DEFAULT '0';");
        rdb.execSQL("ALTER TABLE `groupChatsMessages` ADD COLUMN `isPinned` INTEGER NOT NULL DEFAULT '0';");
        rdb.execSQL("ALTER TABLE `groupChatsMessages` ADD COLUMN `v` INTEGER NOT NULL DEFAULT '0';");
    }

    private void createIDX(SQLiteDatabase rdb) {
        rdb.execSQL("CREATE INDEX IF NOT EXISTS idx_groupChats_isDelete ON groupChats(isDelete);");
        rdb.execSQL("CREATE INDEX IF NOT EXISTS idx_messages_chatId_isDelete ON groupChatsMessages(chatId, isDelete, msgId);");

        // 1. Для поиска чата по его ID
        rdb.execSQL("CREATE UNIQUE INDEX IF NOT EXISTS idx_groupChats_chatUID ON groupChats(chatUID);");

        // 2. Для поиска конкретного сообщения
        rdb.execSQL("CREATE UNIQUE INDEX IF NOT EXISTS idx_groupChatsMessages_msgId ON groupChatsMessages(msgId);");

        // 3. Для проверки приватных чатов
        rdb.execSQL("CREATE INDEX IF NOT EXISTS idx_groupChatsUsers_toId ON groupChatsUsers(toId);");

        // 4. Оптимизация для загрузки комментов
        rdb.execSQL("CREATE INDEX IF NOT EXISTS idx_groupChatsMessages_comments ON groupChatsMessages(chatId, commentMsg, isDelete);");
    }

    private void createEmojiNPixels(SQLiteDatabase rdb) {
        //emoji
        rdb.execSQL("CREATE TABLE IF NOT EXISTS `emoji` (" +
                "`uid` INTEGER primary key AUTOINCREMENT," +
                "`emojiId` INTEGER NOT NULL,"+
                "`fromPack` INTEGER NOT NULL,"+
                "`image` VARCHAR(100) NOT NULL,"+
                "`image64` VARCHAR(100) NOT NULL,"+
                "`value` VARCHAR(4) NOT NULL,"+
                "UNIQUE(emojiId,fromPack)"+
                ")");

        rdb.execSQL("CREATE TABLE IF NOT EXISTS `emoji_packs` ("+
                "`uid` INTEGER primary key AUTOINCREMENT,"+
                "`packId` INTEGER NOT NULL,"+
                "`icon` VARCHAR(100) NOT NULL,"+
                "`name` VARCHAR(64) NOT NULL,"+
                "UNIQUE(packId)"+
                ")");

        //pixels
        rdb.execSQL("CREATE TABLE IF NOT EXISTS `pixels` (" +
                "`uid` INTEGER primary key AUTOINCREMENT," +
                "`pixelId` INTEGER NOT NULL,"+
                "`fromPack` INTEGER NOT NULL,"+
                "`image` VARCHAR(100) NOT NULL,"+
                "`value` VARCHAR(4) NOT NULL,"+
                "UNIQUE(pixelId,fromPack)"+
                ")");

        rdb.execSQL("CREATE TABLE IF NOT EXISTS `pixels_packs` ("+
                "`uid` INTEGER primary key AUTOINCREMENT,"+
                "`packId` INTEGER NOT NULL,"+
                "`icon` VARCHAR(100) NOT NULL,"+
                "`name` VARCHAR(64) NOT NULL,"+
                "UNIQUE(packId)"+
                ")");
    }

    private void createSendMessages(SQLiteDatabase rdb) {
        rdb.execSQL("CREATE TABLE IF NOT EXISTS `sendMessages` (" +
                "`uid` INTEGER primary key AUTOINCREMENT,"+
                "`chatId` INTEGER NOT NULL,"+
                "`fromId` INTEGER NOT NULL,"+
                "`answerMsg` INTEGER NOT NULL,"+
                "`commentMsg` INTEGER NOT NULL,"+
                "`message` text NOT NULL,"+
                "`soundMsg` VARCHAR(100) NOT NULL,"+
                "`dataBot` TEXT NOT NULL,"+
                "`recordMic` VARCHAR(100) NOT NULL,"+
                "`encryptId` binary(8) NOT NULL,"+
                "`type` SMALLINT NOT NULL,"+
                "`data` TEXT DEFAULT NULL,"+
                "`shareId` INTEGER NOT NULL,"+
                "`pixelId` INTEGER NOT NULL,"+
                "`v` int(12) NOT NULL"+
                ")");
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        try {
            //onCreate(db);
            if(oldVersion<2) {
                addRequestsPasswordDB(db);
            }
            if(oldVersion<3) {
                addEndChat(db);
            }
            if(oldVersion<4) {
                addAppSettings(db);
            }
            if(oldVersion<5) {
                addNewMessageV2(db);
            }
            if(oldVersion<7) {
                createIDX(db);
            }
            if(oldVersion<9) {
                createEmojiNPixels(db);
            }
            if(oldVersion<10) {
                createSendMessages(db);
            }
        } catch (Exception e) {
            Log.e(TAG,"Error upgrading",e);
        }
    }

    @Override
    public void onDowngrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        //super.onDowngrade(db, oldVersion, newVersion);
    }
}