#ifndef SC_BR_CHAT_MGR_H
#define SC_BR_CHAT_MGR_H

#include "Chat.h"

class BRChatMgr
{
    BRTitulosMgr(){};
    ~BRTitulosMgr(){};

public:
    static BRChatMgr *instance()
    {
        static BRChatMgr *instance = new BRChatMgr();
        return instance;
    }

private:
    ChatHandler Chat(Player* player) { return ChatHandler(player->GetSession()); };
};

#define sBRChatMgr BRChatMgr::instance()

#endif