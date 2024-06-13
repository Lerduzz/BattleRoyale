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
    return std::find(cola.begin(), cola.end(), player) != cola.end();
}

bool EventoMgr::EstaInvitado(Player *player)
{
    return invitados.find(player->GetGUID().GetCounter()) != invitados.end();
}

bool EventoMgr::EstaEnEvento(Player *player)
{
    return jugadores.find(player->GetGUID().GetCounter()) != jugadores.end();
}

void EventoMgr::JugadorEntrando(Player *player)
{
    if (!PuedeEntrarCola(player))
        return;
    cola.push_back(player);
    switch (estado)
    {
    case BR_ESTADO_SIN_SUFICIENTES_JUGADORES:
        // TODO: Si se lleno la cola iniciar nueva ronda.
        if (cola.size() >= minJugadores)
            IniciarRonda();
        break;
    case BR_ESTADO_INVITANDO_JUGADORES:
        // TODO: Invitar directamente a unirse al evento.
        break;
    default:
        // TODO: Anunciar que se ha unido a la cola y el estado actual del evento.
        // TODO: Anunciar a los miembros de la cola que se ha unido otro si procede.
        break;
    }
}

void EventoMgr::LimpiarEvento()
{
    estado = BR_ESTADO_SIN_SUFICIENTES_JUGADORES;
    cola.clear();
    invitados.clear();
    jugadores.clear();
    intervalo = BRConfigIntervalos(); // TODO: Conf (seg).
    intervalo.sinJugadores = 1800;
    intervalo.invitandoJugadores = 30;
    intervalo.esperandoJugadores = 15;
    intervalo.naveEnEspera = 30;
    intervalo.zonaEnEspera = 60;
    minJugadores = 1; // TODO: Conf.
    maxJugadores = 5; // TODO: Conf.
    tiempoRestanteInicio = 45 + intervalo.naveEnEspera + intervalo.esperandoJugadores + intervalo.invitandoJugadores;
    indiceDePosicionNave = 0;
}

void EventoMgr::IniciarRonda()
{
    if (estado != BR_ESTADO_SIN_SUFICIENTES_JUGADORES)
    {
        LOG_ERROR("event.br", "[BattleRoyale] EventoMgr::IniciarRonda(): El evento no se encuentra en el estado correcto.");
        return;
    }
    if (cola.empty())
    {
        LOG_ERROR("event.br", "[BattleRoyale] EventoMgr::IniciarRonda(): No hay jugadores en cola.");
        return;
    }
    // TODO: Establecer mapa mas votado.
    // TODO: Invocar nave.
    estado = BR_ESTADO_INVITANDO_JUGADORES;
    tiempoRestanteInicio = 45 + intervalo.naveEnEspera + intervalo.esperandoJugadores + intervalo.invitandoJugadores;
    InvitarJugadores();
}

void EventoMgr::InvitarJugadores()
{
    while (!cola.empty() && invitados.size() + jugadores.size() < maxJugadores && tiempoRestanteInicio > 45 + intervalo.naveEnEspera + intervalo.esperandoJugadores)
    {
        Player *player = cola.front();
        cola.pop_front();
        if (!player)
            continue;
        invitados[player->GetGUID().GetCounter()] = player;
        if (!player->IsInWorld() || !player->IsAlive() || player->IsBeingTeleported())
            continue;
        InvitarJugador(player);
    }
}

void EventoMgr::InvitarJugador(Player *player)
{
    if (!EstaInvitado(player))
        return;
    float ox = BR_VARIACIONES_POSICION[indiceDePosicionNave][0];
    float oy = BR_VARIACIONES_POSICION[indiceDePosicionNave][1];
    BRMapa *brM = sMapaMgr->MapaActual();
    Position iN = brM->inicioNave;
    uint32 tiempo = (tiempoRestanteInicio - (45 + intervalo.naveEnEspera)) * IN_MILLISECONDS;
    player->SetSummonPoint(brM->idMapa, iN.GetPositionX() + ox, iN.GetPositionY() + oy, iN.GetPositionZ() + 2.5f);
    WorldPacket data(SMSG_SUMMON_REQUEST, 8 + 4 + 4);
    data << sCharacterCache->GetCharacterGuidByName("BattleRoyale");
    data << uint32(brM->idZona);
    data << uint32(tiempo);
    player->GetSession()->SendPacket(&data);
    if (++indiceDePosicionNave >= BR_CANTIDAD_VARIACIONES)
    {
        indiceDePosicionNave = 0;
    }
}
