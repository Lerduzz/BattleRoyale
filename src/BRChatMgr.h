#ifndef SC_BR_CHAT_MGR_H
#define SC_BR_CHAT_MGR_H

#include "Chat.h"
#include "Player.h"

typedef std::map<uint32, Player*> BR_ListaChat;

class BRChatMgr
{
    BRChatMgr(){};
    ~BRChatMgr(){};

public:
    static BRChatMgr *instance()
    {
        static BRChatMgr *instance = new BRChatMgr();
        return instance;
    }

    void NotificarTiempoInicial(uint32 tiempo, BR_ListaChat lista, std::string mapa = "Lugar: Desconocido")
    {
        if (lista.size())
        {
            std::ostringstream notificacion;
            switch (tiempo)
            {
                case 0:
                {
                    notificacion << "|cff00ff00¡Que comience la batalla de |cffDA70D6" << mapa.c_str() << "|cff00ff00!";
                    AnunciarRondaIniciada(mapa, lista.size());
                    break;
                }
                case 5:
                case 10:
                case 15:
                {
                    notificacion << "|cff00ff00En |cffDA70D6" << tiempo << "|cff00ff00 segundos... |cffff0000¡INICIAN LAS HOSTILIDADES!";
                    break;
                }
                case 20:
                {
                    notificacion << "|cff00ff00¡YA PUEDES SALTAR CUANDO QUIERAS! |cffff0000¡REVISA TUS ALAS!";
                    break;
                }
                case 25:
                case 30:
                case 35:
                case 40:
                {
                    notificacion << "|cff00ff00Faltan |cffDA70D6" << (tiempo - 20) << "|cff00ff00 segundos para llegar. |cffff0000¡EQUIPA TUS ALAS!";
                    break;
                }
                case 45:
                {
                    notificacion << "|cff00ff00La nave se mueve. |cffff0000¡QUÉDATE EN ELLA HASTA LLEGAR!";
                    break;
                }
                default:
                {
                    if (tiempo > 45 && tiempo <= 75)
                    {
                        notificacion << "|cff00ff00Faltan |cffDA70D6" << (tiempo - 45) << "|cff00ff00 segundos para encender motores. |cffff0000¡NO TE TIRES!";
                    }
                    else
                    {
                        notificacion << "|cffff0000ERROR: Mensaje desconocido.|r";
                    }
                    break;
                }
            }
            for (BR_ListaChat::iterator it = lista.begin(); it != lista.end(); ++it)
            {
                it->second->GetSession()->SendNotification(notificacion.str().c_str());
            }
        }
    };

private:
    ChatHandler Chat(Player* player) { return ChatHandler(player->GetSession()); };
    void AnunciarRondaIniciada(std::string mapa, uint32 cantidad)
    {
        std::ostringstream msg;
        msg << "|cff4CFF00BattleRoyale::|r Ronda iniciada en |cffDA70D6" << mapa.c_str() << "|r con |cff4CFF00" << cantidad << "|r jugadores.";
        sWorld->SendServerMessage(SERVER_MSG_STRING, msg.str().c_str());
    };
};

#define sBRChatMgr BRChatMgr::instance()

#endif