#ifndef SC_BR_CHAT_MGR_H
#define SC_BR_CHAT_MGR_H

#include "Player.h"
#include "Constantes.h"

class ChatMgr
{
    ChatMgr(){};
    ~ChatMgr(){};

public:
    static ChatMgr *instance()
    {
        static ChatMgr *instance = new ChatMgr();
        return instance;
    }

    void MensajeEntrada(Player *player, BRMensaje tipo, std::string extra = "");

private:
    inline ChatHandler Chat(Player *player) { return ChatHandler(player->GetSession()); };
};

#define sChatMgr ChatMgr::instance()

#endif