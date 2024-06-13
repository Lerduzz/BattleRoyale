#include "MensajeMgr.h"

void MensajeMgr::NotificarTiempoInicial(uint32 tiempo, BRListaPersonajes lista, std::string mapa)
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
        for (BRListaPersonajes::iterator it = lista.begin(); it != lista.end(); ++it)
        {
            it->second->GetSession()->SendAreaTriggerMessage("%s", notificacion.str().c_str());
        }
    }
}

void MensajeMgr::NotificarAdvertenciaDeZona(uint32 tiempo, BRListaPersonajes lista)
{
    if (lista.size())
    {
        for (BRListaPersonajes::iterator it = lista.begin(); it != lista.end(); ++it)
        {
            it->second->GetSession()->SendNotification("|cff00ff00¡La zona segura se reducirá en |cffDA70D6%u|cff00ff00 segundos!", tiempo);
        }
    }
}

void MensajeMgr::NotificarZonaEnReduccion(BRListaPersonajes lista)
{
    if (lista.size())
    {
        for (BRListaPersonajes::iterator it = lista.begin(); it != lista.end(); ++it)
        {
            it->second->GetSession()->SendNotification("|cffff0000¡La zona segura se está reduciendo!");
        }
    }
}

void MensajeMgr::NotificarNaveRetirada(BRListaPersonajes lista)
{
    if (lista.size())
    {
        for (BRListaPersonajes::iterator it = lista.begin(); it != lista.end(); ++it)
        {
            it->second->GetSession()->SendNotification("|cff0000ff¡La nave se ha retirado!");
        }
    }
}

void MensajeMgr::AnunciarJugadoresEnCola(Player *player, uint32 minimo, BRListaPersonajes lista, BRMensaje estado)
{
    if (lista.size())
    {
        std::string mensajeEstado;
        switch (estado)
        {
        case BR_MENSAJE_ESTADO_EVENTO_LLENO:
        {
            mensajeEstado = " Evento lleno, espera a que termine la ronda.";
            break;
        }
        case BR_MENSAJE_ESTADO_EVENTO_EN_CURSO:
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
        Chat(player).PSendSysMessage("%s Te has unido a la cola del evento. Jugadores en cola: |cff4CFF00%u|r/|cff4CFF00%u|r.%s", BR_NOMBRE_CHAT.c_str(), lista.size(), minimo, mensajeEstado.c_str());
        for (BRListaPersonajes::iterator it = lista.begin(); it != lista.end(); ++it)
        {
            if (it->second != player)
            {
                ChatHandler h = Chat(it->second);
                h.PSendSysMessage("%s El jugador %s se ha unido a la cola. Jugadores en cola: |cff4CFF00%u|r/|cff4CFF00%u|r.%s", BR_NOMBRE_CHAT.c_str(), h.GetNameLink(player), lista.size(), minimo, mensajeEstado.c_str());
            }
        }
    }
}

void MensajeMgr::AnunciarMuerteJcJ(Player *killer, Player *killed, int kills, BRListaPersonajes lista)
{
    if (lista.size())
    {
        for (BRListaPersonajes::iterator it = lista.begin(); it != lista.end(); ++it)
        {
            if (killer == killed)
            {
                Chat(it->second).PSendSysMessage("%s ¡%s ha muerto!.", BR_NOMBRE_CHAT.c_str(), Chat(killed).GetNameLink(killed));
            }
            else
            {
                Chat(it->second).PSendSysMessage("%s ¡%s ha sido eliminado por %s!, racha: |cff4CFF00%i|r.", BR_NOMBRE_CHAT.c_str(), Chat(killed).GetNameLink(killed), Chat(killer).GetNameLink(killer), kills);
            }
        }
    }
}

void MensajeMgr::AnunciarGanador(Player *winner, int kills)
{
    std::ostringstream msg;
    msg << BR_NOMBRE_CHAT << " Ronda finalizada, ganador: " << Chat(winner).GetNameLink(winner) << ", víctimas: |cff4CFF00" << kills << "|r.";
    sWorld->SendServerMessage(SERVER_MSG_STRING, msg.str().c_str());
}

void MensajeMgr::AnunciarEmpate()
{
    std::ostringstream msg;
    msg << BR_NOMBRE_CHAT << " Ronda finalizada, no hubo ganador|r.";
    sWorld->SendServerMessage(SERVER_MSG_STRING, msg.str().c_str());
}

void MensajeMgr::AnunciarEfectoZona(BRListaPersonajes lista, int vivos)
{
    if (lista.size())
    {
        for (BRListaPersonajes::iterator it = lista.begin(); it != lista.end(); ++it)
        {
            Chat(it->second).PSendSysMessage("%s ¡Efectos de Zona aplicados! Jugadores vivos: |cff4CFF00%u|r, y espectadores: |cff4CFF00%u|r.", BR_NOMBRE_CHAT.c_str(), vivos, lista.size() - vivos);
        }
    }
}

void MensajeMgr::AnunciarConteoCofres(int cofres, BRListaPersonajes lista)
{
    if (lista.size())
    {
        for (BRListaPersonajes::iterator it = lista.begin(); it != lista.end(); ++it)
        {
            Chat(it->second).PSendSysMessage("%s Ha%s aparecido %i cofre%s con recompensas aleatorias.", BR_NOMBRE_CHAT.c_str(), (cofres > 1 ? "n" : ""), cofres, (cofres > 1 ? "s" : ""));
        }
    }
}

void MensajeMgr::AnunciarMensajeBienvenida(Player *player)
{
    ChatHandler h = Chat(player);
    h.PSendSysMessage("|-----> %s ----->", BR_NOMBRE_CHAT.c_str());
    h.SendSysMessage("|-> Bienvenido a este nuevo modo de juego |cffff5733todos contra todos|r.");
    h.SendSysMessage("|-> Se te han otorgado tus alas. |cffff0000¡SON TU CAMISA!|r");
    h.SendSysMessage("|-> Puedes arrastrarlas a la barra de acción para facilitar su uso.");
    h.SendSysMessage("|-> Recuerda |cffff5733permanecer en la nave|r hasta que se anuncie que puedes saltar.");
    h.SendSysMessage("|-> |cffff0000La batalla es individual, se castigará a quien incumpla esta regla.|r");
    h.SendSysMessage("|-----> |cff4CFF00¡Que te diviertas!|r ----->");
}

void MensajeMgr::AnunciarErrorAlas(Player *player)
{
    Chat(player).PSendSysMessage("%s ¡No has obtenido las alas porque no tienes espacio disponible! |cffff0000¡Descansa en paz! :(|r", BR_NOMBRE_CHAT.c_str());
}

void MensajeMgr::AnunciarMensajeEntrada(Player *player, BRMensaje tipo)
{
    switch (tipo)
    {
    case BR_MENSAJE_ERROR_EN_COLA:
        Chat(player).PSendSysMessage("%s ¡Ya estas en cola para el evento!", BR_NOMBRE_CHAT.c_str());
        break;
    case BR_MENSAJE_ERROR_EN_EVENTO:
        Chat(player).PSendSysMessage("%s ¡Ya estas dentro del evento!", BR_NOMBRE_CHAT.c_str());
        break;
    case BR_MENSAJE_ERROR_DK_INICIO:
        Chat(player).PSendSysMessage("%s Primero debes terminar la cadena de misiones inicial.", BR_NOMBRE_CHAT.c_str());
        break;
    case BR_MENSAJE_ERROR_NIVEL:
        Chat(player).PSendSysMessage("%s ¡Este modo de juego es solo para personajes de nivel 80!", BR_NOMBRE_CHAT.c_str());
        break;
    }
}

void MensajeMgr::AnunciarAvisoInicioForzado()
{
    std::ostringstream msg;
    msg << BR_NOMBRE_CHAT << " Dentro de 5 minutos se iniciará la ronda si hay al menos un jugador en cola.";
    sWorld->SendServerMessage(SERVER_MSG_STRING, msg.str().c_str());
}

void MensajeMgr::AnunciarErrorInicioForzado()
{
    std::ostringstream msg;
    msg << BR_NOMBRE_CHAT << " El inicio automático ha sido pospuesto porque no hay ningún jugador en cola.";
    sWorld->SendServerMessage(SERVER_MSG_STRING, msg.str().c_str());
}

void MensajeMgr::AnunciarInicioForzado(uint32 count)
{
    std::ostringstream msg;
    msg << BR_NOMBRE_CHAT << " Invocando a " << count << " jugador" << (count == 1 ? "" : "es") << " a la nave.";
    sWorld->SendServerMessage(SERVER_MSG_STRING, msg.str().c_str());
}

void MensajeMgr::AnunciarRondaIniciada(std::string mapa, uint32 cantidad)
{
    std::ostringstream msg;
    msg << BR_NOMBRE_CHAT << " Ronda iniciada en |cffDA70D6" << mapa.c_str() << "|r con |cff4CFF00" << cantidad << "|r jugadores.";
    sWorld->SendServerMessage(SERVER_MSG_STRING, msg.str().c_str());
}
