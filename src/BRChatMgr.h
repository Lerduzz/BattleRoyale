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

    void NotificarJugadoresEnCola(Player* player, uint32 minimo, BR_ListaChat lista)
    {
        if (lista.size())
        {
            for (BR_ListaChat::iterator it = lista.begin(); it != lista.end(); ++it)
            {
                if (it->second != player)
                {
                    ChatHandler h = Chat(it->second);
                    h.PSendSysMessage("|cff4CFF00BattleRoyale::|r %s se ha unido a la cola. Jugadores en cola: |cff4CFF00%u|r/|cff4CFF00%u|r.", h.GetNameLink(player), lista.size(), minimo);
                }
            }
        }
    };
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
    void NotificarAdvertenciaDeZona(uint32 tiempo, BR_ListaChat lista)
    {
        if (lista.size())
        {
            for (BR_ListaChat::iterator it = lista.begin(); it != lista.end(); ++it)
            {
                it->second->GetSession()->SendNotification("|cff00ff00¡La zona segura se reducirá en |cffDA70D6%u|cff00ff00 segundos!", tiempo);
            }
        }
    };
    void NotificarZonaReducida(BR_ListaChat lista)
    {
        if (lista.size())
        {
            for (BR_ListaChat::iterator it = lista.begin(); it != lista.end(); ++it)
            {
                it->second->GetSession()->SendNotification("|cffff0000¡La zona segura se ha actualizado!");
            }
        }
    };
    void NotificarMuerteJcJ(Player* killer, Player* killed, int kills, BR_ListaChat lista)
    {
        if (lista.size())
        {
            for (BR_ListaChat::iterator it = lista.begin(); it != lista.end(); ++it)
            {
                Chat(it->second).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡%s ha eliminado a %s!, racha: |cff4CFF00%i|r.", Chat(killer).GetNameLink(killer), Chat(killed).GetNameLink(killed), kills);
            }
        }
    };
    void NotificarNaveRetirada(BR_ListaChat lista)
    {
        if (lista.size())
        {
            for (BR_ListaChat::iterator it = lista.begin(); it != lista.end(); ++it)
            {
                it->second->GetSession()->SendNotification("|cff0000ff¡La nave se ha retirado!");
            }
        }
    };
    void AnunciarGanador(Player* winner, int kills)
    {
        std::ostringstream msg;
        msg << "|cff4CFF00BattleRoyale::|r Ronda finalizada, ganador: " << Chat(winner).GetNameLink(winner) << ", víctimas: |cff4CFF00" << kills << "|r.";
        sWorld->SendServerMessage(SERVER_MSG_STRING, msg.str().c_str());
    };
    void AnunciarEmpate()
    {
        std::ostringstream msg;
        msg << "|cff4CFF00BattleRoyale::|r Ronda finalizada, no hubo ganador|r.";
        sWorld->SendServerMessage(SERVER_MSG_STRING, msg.str().c_str());
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