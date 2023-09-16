#ifndef SC_BR_CHAT_MGR_H
#define SC_BR_CHAT_MGR_H

#include "Chat.h"
#include "Player.h"

enum BR_TipoMensaje
{
    MENSAJE_ERROR_MAZMORRA = 0,
    MENSAJE_ERROR_BG,
    MENSAJE_ERROR_EN_COLA,
    MENSAJE_ERROR_EN_EVENTO,
    MENSAJE_ERROR_EN_VUELO,
    MENSAJE_ERROR_EN_COMBATE,
};

enum BR_TipoMensajeEstado
{
    MENSAJE_ESTADO_EVENTO_OK = 0,
    MENSAJE_ESTADO_EVENTO_EN_CURSO,
    MENSAJE_ESTADO_EVENTO_LLENO,
};

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

    void NotificarJugadoresEnCola(Player* player, uint32 minimo, BR_ListaChat lista, BR_TipoMensajeEstado estado = MENSAJE_ESTADO_EVENTO_OK)
    {
        if (lista.size())
        {
            std::string mensajeEstado;
            switch (estado)
            {
                case MENSAJE_ESTADO_EVENTO_LLENO:
                {
                    mensajeEstado = " Evento lleno, espera a que termine la ronda.";
                    break;
                }
                case MENSAJE_ESTADO_EVENTO_EN_CURSO:
                {
                    mensajeEstado = " Evento en curso, espera a que termine la ronda.";
                    break;
                }
                default:
                {
                    mensajeEstado = "";
                    break;
                }
            }
            Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r Te has unido a la cola del evento. Jugadores en cola: |cff4CFF00%u|r/|cff4CFF00%u|r.%s", lista.size(), minimo, mensajeEstado.c_str());
            for (BR_ListaChat::iterator it = lista.begin(); it != lista.end(); ++it)
            {
                if (it->second != player)
                {
                    ChatHandler h = Chat(it->second);
                    h.PSendSysMessage("|cff4CFF00BattleRoyale::|r %s se ha unido a la cola. Jugadores en cola: |cff4CFF00%u|r/|cff4CFF00%u|r.%s", h.GetNameLink(player), lista.size(), minimo, mensajeEstado.c_str());
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
    void AnunciarEfectoZona(BR_ListaChat lista, int vivos)
    {
        if (lista.size())
        {
            for (BR_ListaChat::iterator it = lista.begin(); it != lista.end(); ++it)
            {
                Chat(it->second).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡Efectos de Zona aplicados! Jugadores vivos: |cff4CFF00%u|r, y espectadores: |cff4CFF00%u|r.", vivos, lista.size() - vivos);
            }
        }
    };
    void AnunciarConteoCofres(int cofres, BR_ListaChat lista)
    {
        if (lista.size())
        {
            for (BR_ListaChat::iterator it = lista.begin(); it != lista.end(); ++it)
            {
                Chat(it->second).PSendSysMessage("|cff4CFF00BattleRoyale::|r |cff1c7500Ha%s aparecido %i cofre%s con recompensas aleatorias.|r", (cofres > 1 ? "n" : ""), cofres, (cofres > 1 ? "s" : ""));
            }
        }
    };
    void AnunciarMensajeBienvenida(Player* player)
    {
        ChatHandler h = Chat(player);
        h.SendSysMessage("|-----> |cff4CFF00BattleRoyale|r ----->");
        h.SendSysMessage("|-> Bienvenido a este nuevo modo de juego |cffff5733todos contra todos|r.");
        h.SendSysMessage("|-> Se te han otorgado tus alas, están en tu mochila. |cffff0000¡EQUÍPALAS!|r");
        h.SendSysMessage("|-> Puedes arrastrarlas a la barra de acción para facilitar su uso.");
        h.SendSysMessage("|-> Recuerda |cffff5733permanecer en la nave|r hasta que se anuncie que puedes saltar.");
        h.SendSysMessage("|-> |cffff0000La batalla es individual, se castigará a quien incumpla esta regla.|r");
        h.SendSysMessage("|-----> |cff4CFF00¡Que te diviertas!|r ----->");
    };
    void AnunciarErrorAlas(Player* player, int noSpace = false)
    {
        if (noSpace)
        {
            Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡No has obtenido las alas porque no tienes espacio disponible! |cffff0000¡Descansa en paz! :(|r");
        }
        else
        {
            Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡No has obtenido las alas porque no se ha podido crear el objeto! |cffff0000¡Descansa en paz! :(|r");
        }
    };
    void AnunciarMensajeEntrada(Player* player, BR_TipoMensaje tipo)
    {
        switch (tipo)
        {
            case MENSAJE_ERROR_MAZMORRA:
            {
                Chat(player).SendSysMessage("|cff4CFF00BattleRoyale::|r ¡No puedes participar mientras utilizas el buscador de mazmorras!");
                break;
            }
            case MENSAJE_ERROR_BG:
            {
                Chat(player).SendSysMessage("|cff4CFF00BattleRoyale::|r ¡No puedes participar mientras estás en cola para Campos de Batalla o Arenas!");
                break;
            }
            case MENSAJE_ERROR_EN_COLA:
            {
                Chat(player).SendSysMessage("|cff4CFF00BattleRoyale::|r ¡Ya estas en cola para el evento!");
                break;
            }
            case MENSAJE_ERROR_EN_EVENTO:
            {
                Chat(player).SendSysMessage("|cff4CFF00BattleRoyale::|r ¡Ya estas dentro del evento!");
                break;
            }
            case MENSAJE_ERROR_EN_VUELO:
            {
                Chat(player).SendSysMessage("|cff4CFF00BattleRoyale::|r ¡No has podido entrar al evento porque vas en ruta de vuelo! ¡Se te ha quitado de la cola!");
                break;
            }
            case MENSAJE_ERROR_EN_COMBATE:
            {
                Chat(player).SendSysMessage("|cff4CFF00BattleRoyale::|r ¡No has podido entrar al evento porque estás en combate! ¡Se te ha quitado de la cola!");
                break;
            }
            default:
            {
                break;
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