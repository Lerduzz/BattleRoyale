#include "EventoMgr.h"

EventoMgr::EventoMgr()
{
    LimpiarEvento();
}

EventoMgr::~EventoMgr()
{
    LimpiarEvento();
}

bool EventoMgr::PuedeEntrarCola(Player *player)
{
    if (!player)
    {
        LOG_ERROR("event.br", "[BattleRoyale] EventoMgr::PuedeEntrarCola(Player *player): El jugador no exixte.");
        return false;
    }
    if (player->getLevel() != 80)
    {
        sChatMgr->MensajeEntrada(player, BR_MENSAJE_ERROR_NIVEL);
        return false;
    }
    if (player->getClass() == CLASS_DEATH_KNIGHT && player->GetMapId() == 609)
    {
        sChatMgr->MensajeEntrada(player, BR_MENSAJE_ERROR_DK_INICIO);
        return false;
    }
    if (EstaEnCola(player))
    {
        sChatMgr->MensajeEntrada(player, BR_MENSAJE_ERROR_EN_COLA);
        return false;
    }
    if (EstaInvitado(player))
    {
        sChatMgr->MensajeEntrada(player, BR_MENSAJE_ERROR_INVITADO);
        return false;
    }
    if (EstaEnEvento(player))
    {
        sChatMgr->MensajeEntrada(player, BR_MENSAJE_ERROR_EN_EVENTO);
        return false;
    }
    return true;
}

bool EventoMgr::EstaEnCola(Player *player)
{
    return cola.find(player->GetGUID().GetCounter()) != cola.end();
}

bool EventoMgr::EstaInvitado(Player *player)
{
    return false;
}

bool EventoMgr::EstaEnEvento(Player *player)
{
    return false;
}

void EventoMgr::JugadorEntrando(Player *player)
{
    if (!PuedeEntrarCola(player))
        return;
    // TODO: Agregar a la cola, llamar al evento o iniciar ronda nueva segun estado actual.
    LOG_ERROR("event.br", "[BattleRoyale] TODO: EventoMgr::JugadorEntrando(Player *player): El jugador cumple los requisitos para entrar en cola.");
}

void EventoMgr::LimpiarEvento()
{
    estado = BR_ESTADO_SIN_SUFICIENTES_JUGADORES;
    cola.clear();
}
