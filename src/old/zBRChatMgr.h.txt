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
    MENSAJE_ERROR_BLOQUEADO,
    MENSAJE_ERROR_DK_INICIO,
    MENSAJE_ERROR_NIVEL,
};

enum BR_TipoMensajeEstado
{
    MENSAJE_ESTADO_EVENTO_OK = 0,
    MENSAJE_ESTADO_EVENTO_EN_CURSO,
    MENSAJE_ESTADO_EVENTO_LLENO,
};

const std::string TEXTO_NOMBRE = "|cffff6633[|cffff0000B|cffff3300attle |cffff0000R|cffff3300oyale|cffff6633]:|r";

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
                    notificacion << "¡Que comience la batalla de " << mapa.c_str() << "!";
                    AnunciarRondaIniciada(mapa, lista.size());
                    break;
                }
                case 5:
                case 10:
                case 15:
                {
                    notificacion << "En " << tiempo << " segundos... ¡INICIAN LAS HOSTILIDADES!";
                    break;
                }
                case 20:
                {
                    notificacion << "¡YA PUEDES SALTAR CUANDO QUIERAS!";
                    break;
                }
                case 25:
                case 30:
                case 35:
                case 40:
                {
                    notificacion << "Faltan " << (tiempo - 20) << " segundos para llegar.";
                    break;
                }
                case 45:
                {
                    notificacion << "La nave se mueve. ¡PERMANECE EN ELLA HASTA LLEGAR!";
                    break;
                }
                default:
                {
                    notificacion << "Faltan " << (tiempo - 45) << " segundos para encender motores.";
                    break;
                }
            }
            for (BR_ListaChat::iterator it = lista.begin(); it != lista.end(); ++it)
            {
                it->second->GetSession()->SendAreaTriggerMessage("%s", notificacion.str().c_str());
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
    void NotificarZonaEnReduccion(BR_ListaChat lista)
    {
        if (lista.size())
        {
            for (BR_ListaChat::iterator it = lista.begin(); it != lista.end(); ++it)
            {
                it->second->GetSession()->SendNotification("|cffff0000¡La zona segura se está reduciendo!");
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
    void AnunciarJugadoresEnCola(Player* player, uint32 minimo, BR_ListaChat lista, BR_TipoMensajeEstado estado = MENSAJE_ESTADO_EVENTO_OK)
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
            Chat(player).PSendSysMessage("%s Te has unido a la cola del evento. Jugadores en cola: |cff4CFF00%u|r/|cff4CFF00%u|r.%s", TEXTO_NOMBRE.c_str(), lista.size(), minimo, mensajeEstado.c_str());
            for (BR_ListaChat::iterator it = lista.begin(); it != lista.end(); ++it)
            {
                if (it->second != player)
                {
                    ChatHandler h = Chat(it->second);
                    h.PSendSysMessage("%s El jugador %s se ha unido a la cola. Jugadores en cola: |cff4CFF00%u|r/|cff4CFF00%u|r.%s", TEXTO_NOMBRE.c_str(), h.GetNameLink(player), lista.size(), minimo, mensajeEstado.c_str());
                }
            }
        }
    };
    void AnunciarMuerteJcJ(Player* killer, Player* killed, int kills, BR_ListaChat lista)
    {
        if (lista.size())
        {
            for (BR_ListaChat::iterator it = lista.begin(); it != lista.end(); ++it)
            {
                if (killer == killed)
                {
                    Chat(it->second).PSendSysMessage("%s ¡%s ha muerto!.", TEXTO_NOMBRE.c_str(), Chat(killed).GetNameLink(killed));
                }
                else
                {
                    Chat(it->second).PSendSysMessage("%s ¡%s ha sido eliminado por %s!, racha: |cff4CFF00%i|r.", TEXTO_NOMBRE.c_str(), Chat(killed).GetNameLink(killed), Chat(killer).GetNameLink(killer), kills);
                }
            }
        }
    };
    void AnunciarGanador(Player* winner, int kills)
    {
        std::ostringstream msg;
        msg << TEXTO_NOMBRE << " Ronda finalizada, ganador: " << Chat(winner).GetNameLink(winner) << ", víctimas: |cff4CFF00" << kills << "|r.";
        sWorld->SendServerMessage(SERVER_MSG_STRING, msg.str().c_str());
    };
    void AnunciarEmpate()
    {
        std::ostringstream msg;
        msg << TEXTO_NOMBRE << " Ronda finalizada, no hubo ganador|r.";
        sWorld->SendServerMessage(SERVER_MSG_STRING, msg.str().c_str());
    };
    void AnunciarEfectoZona(BR_ListaChat lista, int vivos)
    {
        if (lista.size())
        {
            for (BR_ListaChat::iterator it = lista.begin(); it != lista.end(); ++it)
            {
                Chat(it->second).PSendSysMessage("%s ¡Efectos de Zona aplicados! Jugadores vivos: |cff4CFF00%u|r, y espectadores: |cff4CFF00%u|r.", TEXTO_NOMBRE.c_str(), vivos, lista.size() - vivos);
            }
        }
    };
    void AnunciarConteoCofres(int cofres, BR_ListaChat lista)
    {
        if (lista.size())
        {
            for (BR_ListaChat::iterator it = lista.begin(); it != lista.end(); ++it)
            {
                Chat(it->second).PSendSysMessage("%s Ha%s aparecido %i cofre%s con recompensas aleatorias.", TEXTO_NOMBRE.c_str(), (cofres > 1 ? "n" : ""), cofres, (cofres > 1 ? "s" : ""));
            }
        }
    };
    void AnunciarMensajeBienvenida(Player* player)
    {
        ChatHandler h = Chat(player);
        h.PSendSysMessage("|-----> %s ----->", TEXTO_NOMBRE.c_str());
        h.SendSysMessage("|-> Bienvenido a este nuevo modo de juego |cffff5733todos contra todos|r.");
        h.SendSysMessage("|-> Se te han otorgado tus alas, están en tu mochila. |cffff0000¡EQUÍPALAS!|r");
        h.SendSysMessage("|-> Puedes arrastrarlas a la barra de acción para facilitar su uso.");
        h.SendSysMessage("|-> Recuerda |cffff5733permanecer en la nave|r hasta que se anuncie que puedes saltar.");
        h.SendSysMessage("|-> |cffff0000La batalla es individual, se castigará a quien incumpla esta regla.|r");
        h.SendSysMessage("|-----> |cff4CFF00¡Que te diviertas!|r ----->");
    };
    void AnunciarErrorAlas(Player* player)
    {
        Chat(player).PSendSysMessage("%s ¡No has obtenido las alas porque no tienes espacio disponible! |cffff0000¡Descansa en paz! :(|r", TEXTO_NOMBRE.c_str());
    };
    void AnunciarMensajeEntrada(Player* player, BR_TipoMensaje tipo, std::string extra = "")
    {
        switch (tipo)
        {
            case MENSAJE_ERROR_MAZMORRA:
            {
                Chat(player).PSendSysMessage("%s ¡No puedes participar mientras utilizas el buscador de mazmorras!", TEXTO_NOMBRE.c_str());
                break;
            }
            case MENSAJE_ERROR_BG:
            {
                Chat(player).PSendSysMessage("%s ¡No puedes participar mientras estás en cola para Campos de Batalla o Arenas!", TEXTO_NOMBRE.c_str());
                break;
            }
            case MENSAJE_ERROR_EN_COLA:
            {
                Chat(player).PSendSysMessage("%s ¡Ya estas en cola para el evento!", TEXTO_NOMBRE.c_str());
                break;
            }
            case MENSAJE_ERROR_EN_EVENTO:
            {
                Chat(player).PSendSysMessage("%s ¡Ya estas dentro del evento!", TEXTO_NOMBRE.c_str());
                break;
            }
            case MENSAJE_ERROR_EN_VUELO:
            {
                Chat(player).PSendSysMessage("%s ¡No has podido entrar al evento porque vas en ruta de vuelo! ¡Se te ha quitado de la cola!", TEXTO_NOMBRE.c_str());
                break;
            }
            case MENSAJE_ERROR_BLOQUEADO:
            {
                Chat(player).PSendSysMessage("%s No puedes utilizar este modo de juego. Motivo: |cffff0000%s|r.", TEXTO_NOMBRE.c_str(), extra);
                break;
            }
            case MENSAJE_ERROR_DK_INICIO:
            {
                Chat(player).PSendSysMessage("%s Primero debes terminar la cadena de misiones inicial.", TEXTO_NOMBRE.c_str());
                break;
            }
            case MENSAJE_ERROR_NIVEL:
            {
                Chat(player).PSendSysMessage("%s ¡Este modo de juego es solo para personajes de nivel 80!", TEXTO_NOMBRE.c_str());
                break;
            }
            default:
            {
                break;
            }
        }
    };
    void AnunciarAvisoInicioForzado()
    {
        std::ostringstream msg;
        msg << TEXTO_NOMBRE << " Dentro de 5 minutos se iniciará la ronda si hay al menos un jugador en cola.";
        sWorld->SendServerMessage(SERVER_MSG_STRING, msg.str().c_str());
    };
    void AnunciarErrorInicioForzado()
    {
        std::ostringstream msg;
        msg << TEXTO_NOMBRE << " El inicio automático ha sido pospuesto porque no hay ningún jugador en cola.";
        sWorld->SendServerMessage(SERVER_MSG_STRING, msg.str().c_str());
    };
    void AnunciarInicioForzado(uint32 count)
    {
        std::ostringstream msg;
        msg << TEXTO_NOMBRE << " Invocando a " << count << " jugador" << (count == 1 ? "" : "es") << " a la nave.";
        sWorld->SendServerMessage(SERVER_MSG_STRING, msg.str().c_str());
    };

private:
    ChatHandler Chat(Player* player) { return ChatHandler(player->GetSession()); };
    void AnunciarRondaIniciada(std::string mapa, uint32 cantidad)
    {
        std::ostringstream msg;
        msg << TEXTO_NOMBRE << " Ronda iniciada en |cffDA70D6" << mapa.c_str() << "|r con |cff4CFF00" << cantidad << "|r jugadores.";
        sWorld->SendServerMessage(SERVER_MSG_STRING, msg.str().c_str());
    };
};

#define sBRChatMgr BRChatMgr::instance()

#endif