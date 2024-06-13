#ifndef SC_BR_CHAT_MGR_H
#define SC_BR_CHAT_MGR_H

#include "Chat.h"
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

    /**
     * @brief Muestra los mensajes de error al intentar unirse a la cola del evento.
     *
     * @param player Jugador que intenta unirse a la cola del evento.
     * @param tipo Tipo de mensaje de error.
     */
    void MensajeEntrada(Player *player, BRMensaje tipo);

private:
    inline ChatHandler Chat(Player *player) { return ChatHandler(player->GetSession()); };
};

/**
 * @brief Se encarga de gestionar los anuncios de chat del evento.
 *
 */
#define sChatMgr ChatMgr::instance()

#endif