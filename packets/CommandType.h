// 🚀 Aero Protocol Command Types
enum AeroCommandType {
    CHECK_ONLINE = 0,       // /check [user]
    LOGOUT = 1,             // /logout
    CHAT_HISTORY = 2,       // /history
    DELETE_ACCOUNT = 3,     // /delete_account
    ONLINE_USERS = 4,       // /online_users
    CLEAR_CHAT = 5,         // /clear_chat
    HELP = 6,               // /help
    WHOAMI = 7,             // /whoami
    SEND_FILE = 8,          // /sendfile
    BROADCAST = 9,          // /broadcast <message>
    BLOCK_USER = 10,        // /block [user]
    UNBLOCK_USER = 11,      // /unblock [user]
    USER_PROFILE = 12,      // /profile
    CHANGE_PASSWORD = 13    // /changepass <newpassword>
};
