#include "BattleRoyaleMgr.h"
#include "Config.h"
#include "Chat.h"
#include "Player.h"

BattleRoyaleMgr::BattleRoyaleMgr()
{
    conf_JugadoresMinimo = sConfigMgr->GetOption<int32>("BattleRoyale.MinPlayers", 25);
    conf_JugadoresMaximo = sConfigMgr->GetOption<int32>("BattleRoyale.MaxPlayers", 50);
    conf_IntervaloEntreRecuccionDeZona = sConfigMgr->GetOption<int32>("BattleRoyale.SecureZoneInterval", 60000);
    obj_Zona = nullptr;
    obj_Centro = nullptr;
    obj_Nave = nullptr;
    RestablecerTodoElEvento();
}

BattleRoyaleMgr::~BattleRoyaleMgr()
{
    RestablecerTodoElEvento();
}

// --- PUBLICO --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void BattleRoyaleMgr::GestionarJugadorEntrando(Player *player)
{
    if (EstaEnCola(player))
    {
        Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r Ya estas en cola para el evento.");
        return;
    }
    if (EstaEnEvento(player))
    {
        Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r Ya estas dentro del evento.");
        return;
    }
    switch (estadoActual)
    {
        case ESTADO_NO_HAY_SUFICIENTES_JUGADORES:
        {
            list_Cola[player->GetGUID().GetCounter()] = player;
            if (HaySuficientesEnCola()) {
                IniciarNuevaRonda();
            }
            else
            {
                Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r Te has unido a la cola del evento. Jugadores en cola: %u/%u.", list_Cola.size(), conf_JugadoresMinimo);
            }
            break;
        }
        case ESTADO_NAVE_EN_ESPERA:
        {
            if (EstaLlenoElEvento()) {
                list_Cola[player->GetGUID().GetCounter()] = player;
                Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r Te has unido a la cola del evento. Jugadores en cola: %u/%u. El evento esta lleno, espera la otra ronda.", list_Cola.size(), conf_JugadoresMinimo);
            }
            else
            {
                list_Jugadores[player->GetGUID().GetCounter()] = player;
                AlmacenarPosicionInicial(player->GetGUID().GetCounter());
                LlamarDentroDeNave(player->GetGUID().GetCounter());
            }
            break;
        }
        default:
        {
            list_Cola[player->GetGUID().GetCounter()] = player;
            Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r Te has unido a la cola del evento. Jugadores en cola: %u/%u. El evento esta en curso, espera la otra ronda.", list_Cola.size(), conf_JugadoresMinimo);
            break;
        }
    }
}

void BattleRoyaleMgr::GestionarJugadorDesconectar(Player *player)
{
    if (EstaEnEvento(player)) SalirDelEvento(player->GetGUID().GetCounter(), true);
}

void BattleRoyaleMgr::GestionarMuerteJcJ(Player *killer, Player *killed)
{
    if (!killer || !killed) return;
    if (!list_Jugadores.size() || estadoActual != ESTADO_BATALLA_EN_CURSO) return;
    if (!EstaEnEvento(killer) || !EstaEnEvento(killed)) return;
    list_Datos[killer->GetGUID().GetCounter()].kills++;
    killed->CastSpell(killer, 6277, true);
    // TODO: Llevar una lista de los espectadores de un jugador y al morir comprobar y mover los espectadores al nuevo asesino.
    NotifyPvPKill(ChatHandler(killer->GetSession()).GetNameLink(killer), ChatHandler(killed->GetSession()).GetNameLink(killed), list_Datos[killer->GetGUID().GetCounter()].kills);
}

void BattleRoyaleMgr::GestionarActualizacionMundo(uint32 diff)
{
    switch(estadoActual)
    {
        case ESTADO_INVOCANDO_JUGADORES:
        case ESTADO_NAVE_EN_ESPERA:
        case ESTADO_NAVE_EN_MOVIMIENTO:
        case ESTADO_NAVE_CERCA_DEL_CENTRO:
        {
            if (secondsTicksHelper <= 0) {
                secondsTicksHelper = 1000;
                if (tiempoRestanteSeg <= 0) {
                    estadoActual = ESTADO_BATALLA_EN_CURSO;
                    NotifyTimeRemainingToStart(0);
                    secureZoneDelay = conf_IntervaloEntreRecuccionDeZona;
                } else {
                    if (tiempoRestanteSeg % 5 == 0) {
                        NotifyTimeRemainingToStart(tiempoRestanteSeg);
                    }
                    if (estadoActual == ESTADO_INVOCANDO_JUGADORES && tiempoRestanteSeg <= 60)
                    {
                        estadoActual = ESTADO_NAVE_EN_ESPERA;
                        if (!SpawnTransportShip()) {
                            RestablecerTodoElEvento();
                            return;
                        }
                        TeleportPlayersToShip();
                    }
                    if (estadoActual == ESTADO_NAVE_EN_ESPERA && tiempoRestanteSeg <= 30 && obj_Nave)
                    {
                        estadoActual = ESTADO_NAVE_EN_MOVIMIENTO;
                        uint32_t const autoCloseTime = obj_Nave->GetGOInfo()->GetAutoCloseTime() ? 10000u : 0u;
                        obj_Nave->SetLootState(GO_READY);
                        obj_Nave->UseDoorOrButton(autoCloseTime, false, nullptr);
                    }
                    if (estadoActual == ESTADO_NAVE_EN_MOVIMIENTO && tiempoRestanteSeg <= 5)
                    {
                        estadoActual = ESTADO_NAVE_CERCA_DEL_CENTRO;
                        secureZoneIndex = 0;
                        secureZoneDelay = 0;
                        secureZoneAnnounced = false;
                        obj_Centro = nullptr;
                        obj_Zona = nullptr;
                        if (!SpawnTheCenterOfBattle()) {
                            RestablecerTodoElEvento();
                            return;
                        }
                        if (!SpawnSecureZone()) {
                            RestablecerTodoElEvento();
                            return;
                        }
                        AddParachuteToAllPlayers();
                    }
                    tiempoRestanteSeg--;
                }
            } else {
                secondsTicksHelper -= diff;
            }
            break;
        }
        case ESTADO_BATALLA_EN_CURSO:
        {
            if (secondsTicksHelper <= 0) {
                secondsTicksHelper = 1000;
                OutOfZoneDamage();
                AddFFAPvPFlag();
            } else {
                secondsTicksHelper -= diff;
            }
            if (secureZoneDelay <= 0) {
                SpawnSecureZone();
                NotifySecureZoneReduced();
                secureZoneDelay = conf_IntervaloEntreRecuccionDeZona;
                secureZoneAnnounced = false;
            } else {
                if (secureZoneDelay <= 5000 && !secureZoneAnnounced) {
                    NotifySecureZoneReduceWarn(5);
                    secureZoneAnnounced = true;
                }
                if (secureZoneIndex <= CANTIDAD_DE_ZONAS) {
                    secureZoneDelay -= diff;
                }
            }
            break;
        }
    }
}

void BattleRoyaleMgr::PrevenirJcJEnLaNave(Player* player, bool state)
{
    if (!state || !list_Jugadores.size()) return;
    if (list_Jugadores.find(player->GetGUID().GetCounter()) != list_Jugadores.end() && !DebeForzarJcJTcT(player)) player->SetPvP(false);
}

bool BattleRoyaleMgr::PuedeReaparecerEnCementerio(Player *player)
{
    if (HayJugadores() && EstaEnEvento(player)) {
        if (!player->IsAlive()) RevivirJugador(player);
        SalirDelEvento(player->GetGUID().GetCounter());
    }
    return true;
}

// --- PRIVADO --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void BattleRoyaleMgr::RestablecerTodoElEvento()
{
    list_Cola.clear();
	list_Jugadores.clear();
    list_Datos.clear();
    indiceDelMapa = 0;
    estadoActual = ESTADO_NO_HAY_SUFICIENTES_JUGADORES;
    secondsTicksHelper = 1000;
    indiceDeVariacion = 0;
    if (obj_Zona) {
        obj_Zona->DespawnOrUnsummon();
        obj_Zona->Delete();
        obj_Zona = nullptr;
    }
    if (obj_Centro) {
        obj_Centro->DespawnOrUnsummon();
        obj_Centro->Delete();
        obj_Centro = nullptr;
    }
    if (obj_Nave) {
        obj_Nave->DespawnOrUnsummon();
        obj_Nave->Delete();
        obj_Nave = nullptr;
    }
}

void BattleRoyaleMgr::IniciarNuevaRonda()
{
    if (estadoActual == ESTADO_NO_HAY_SUFICIENTES_JUGADORES)
    {
        estadoActual = ESTADO_INVOCANDO_JUGADORES;
        SiguienteMapa();
        tiempoRestanteSeg = 70;
        for (BR_ColaDePersonajes::iterator it = list_Cola.begin(); it != list_Cola.end(); ++it)
	    {
            if (!EstaLlenoElEvento())
            {
                list_Jugadores[(*it).first] = (*it).second;
                AlmacenarPosicionInicial((*it).first);
                LlamarAntesQueNave((*it).first);
            }            
	    }
        if (HayJugadores()) for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it) list_Cola.erase((*it).first);
    }
}

void BattleRoyaleMgr::AlmacenarPosicionInicial(uint32 guid)
{
    if (list_Jugadores[guid]->GetMap()->Instanceable())
    {
        list_Datos[guid].SetPosition(list_Jugadores[guid]->m_homebindMapId, list_Jugadores[guid]->m_homebindX, list_Jugadores[guid]->m_homebindY, list_Jugadores[guid]->m_homebindZ, list_Jugadores[guid]->GetOrientation());
    } 
    else
    {
        list_Datos[guid].SetPosition(list_Jugadores[guid]->GetMapId(), list_Jugadores[guid]->GetPositionX(), list_Jugadores[guid]->GetPositionY(), list_Jugadores[guid]->GetPositionZ(), list_Jugadores[guid]->GetOrientation());
    }
}

void BattleRoyaleMgr::LlamarAntesQueNave(uint32 guid)
{
    CambiarDimension_Entrar(guid);
    float ox = BR_VariacionesDePosicion[indiceDeVariacion][0];
    float oy = BR_VariacionesDePosicion[indiceDeVariacion][1];
    SiguientePosicion();
    Desmontar(list_Jugadores[guid]);
    list_Jugadores[guid]->SetPvP(false);
    list_Jugadores[guid]->TeleportTo(BR_IdentificadorDeMapas[indiceDelMapa], BR_InicioDeLaNave[indiceDelMapa][0] + ox, BR_InicioDeLaNave[indiceDelMapa][1] + oy, BR_InicioDeLaNave[indiceDelMapa][2] + 15.0f, 0.0f);
    list_Jugadores[guid]->SaveToDB(false, false);
    list_Jugadores[guid]->AddAura(HECHIZO_PARACAIDAS, list_Jugadores[guid]);
}

void BattleRoyaleMgr::LlamarDentroDeNave(uint32 guid)
{
    CambiarDimension_Entrar(guid);
    float ox = BR_VariacionesDePosicion[indiceDeVariacion][0];
    float oy = BR_VariacionesDePosicion[indiceDeVariacion][1];
    SiguientePosicion();
    Desmontar(list_Jugadores[guid]);
    list_Jugadores[guid]->SetPvP(false);
    list_Jugadores[guid]->TeleportTo(BR_IdentificadorDeMapas[indiceDelMapa], BR_InicioDeLaNave[indiceDelMapa][0] + ox, BR_InicioDeLaNave[indiceDelMapa][1] + oy, BR_InicioDeLaNave[indiceDelMapa][2] + 1.5f, 0.0f);
    list_Jugadores[guid]->SaveToDB(false, false);
}

void BattleRoyaleMgr::SalirDelEvento(uint32 guid, bool logout)
{
    if (list_Cola.find(guid) != list_Cola.end()) {
        list_Cola.erase(guid);
    };
    if (list_Jugadores.find(guid) != list_Jugadores.end()) {
        CambiarDimension_Salir(guid);
        if(!logout)
        {
            if (!list_Jugadores[guid]->isPossessing()) list_Jugadores[guid]->StopCastingBindSight();
            list_Jugadores[guid]->TeleportTo(list_Datos[guid].GetMap(), list_Datos[guid].GetX(), list_Datos[guid].GetY(), list_Datos[guid].GetZ(), list_Datos[guid].GetO());
            list_Jugadores[guid]->SaveToDB(false, false);
        }
        list_Jugadores.erase(guid);
        list_Datos.erase(guid);
        
    }
}

void BattleRoyaleMgr::RevivirJugador(Player* player)
{
	player->ResurrectPlayer(1.0f);
    player->SpawnCorpseBones();
    player->SaveToDB(false, false);
}

void BattleRoyaleMgr::NotifySecureZoneReduceWarn(uint32 delay)
{
    if (list_Jugadores.size())
    {
        for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
        {
            (*it).second->GetSession()->SendNotification("|cff00ff00¡La zona segura se reducirá en |cffDA70D6%u|cff00ff00 segundos!", delay);
        }
    }
}

void BattleRoyaleMgr::NotifySecureZoneReduced()
{
    if (list_Jugadores.size())
    {
        for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
        {
            (*it).second->GetSession()->SendNotification("|cffff0000¡ALERTA: La zona segura se ha actualizado!");
        }
    }
}

void BattleRoyaleMgr::NotifyTimeRemainingToStart(uint32 delay)
{
    if (list_Jugadores.size())
    {
        for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it) {
            switch (delay)
            {
                case 0:
                {
                    (*it).second->GetSession()->SendNotification("|cff00ff00¡Que comience la batalla de |cffDA70D6%s|cff00ff00!", BR_NombreDeMapas[indiceDelMapa].c_str());
                    break;
                }                
                case 5:
                {
                    (*it).second->GetSession()->SendNotification("|cff00ff00Ya tienes paracaidas. |cff0000ff¡PUEDES SALTAR!");
                    break;
                }                
                case 10:
                case 15:
                case 20:
                case 25:
                {
                    (*it).second->GetSession()->SendNotification("|cff00ff00Faltan |cffDA70D6%u|cff00ff00 segundos para llegar. |cffff0000¡NO TE TIRES!", delay - 5);
                    break;
                }
                case 30:
                {
                    (*it).second->GetSession()->SendNotification("|cff00ff00La nave comienza a moverse. |cffff0000¡NO TE TIRES!");
                    break;
                }
                default:
                {
                    if (delay > 30 && delay <= 60)
                    {
                        (*it).second->GetSession()->SendNotification("|cff00ff00Faltan |cffDA70D6%u|cff00ff00 segundos para encender motores. |cffff0000¡NO TE TIRES!", delay - 30);
                    }
                    else 
                    {
                        (*it).second->GetSession()->SendNotification("|cff00ff00No tengas miedo. |cff0000ff¡LLEGANDO A LA NAVE!");
                    }
                    break;
                }
            }
        }
    }
}

void BattleRoyaleMgr::NotifyPvPKill(std::string killer, std::string killed, int kills)
{
    if (list_Jugadores.size())
    {
        for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
        {
            ChatHandler((*it).second->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡%s ha eliminado a %s!. Racha de %i.", killer, killed, kills);
        }
    }
}

bool BattleRoyaleMgr::SpawnTransportShip()
{
    bool success = false;
    if (list_Jugadores.size())
    {
        for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
        {
            if ((*it).second)
            {
                if (obj_Nave) {
                    obj_Nave->DespawnOrUnsummon();
                    obj_Nave->Delete();
                    obj_Nave = nullptr;
                }
                float x = BR_InicioDeLaNave[indiceDelMapa][0];
                float y = BR_InicioDeLaNave[indiceDelMapa][1];
                float z = BR_InicioDeLaNave[indiceDelMapa][2];
                float o = BR_InicioDeLaNave[indiceDelMapa][3];
                float rot2 = std::sin(o / 2);
                float rot3 = cos(o / 2);
                obj_Nave = (*it).second->SummonGameObject(OBJETO_NAVE, x, y, z, o, 0, 0, rot2, rot3, 2 * 60);
                success = true;
                break;
            }
        }
    }
    return success;
}

bool BattleRoyaleMgr::SpawnTheCenterOfBattle()
{
    if (obj_Nave)
    {
        if (obj_Centro) {
            obj_Centro->DespawnOrUnsummon();
            obj_Centro->Delete();
            obj_Centro = nullptr;
        }
        obj_Centro = obj_Nave->SummonGameObject(OBJETO_CENTRO_DEL_MAPA, BR_CentroDeMapas[indiceDelMapa].GetPositionX(), BR_CentroDeMapas[indiceDelMapa].GetPositionY(), BR_CentroDeMapas[indiceDelMapa].GetPositionZ(), 0, 0, 0, 0, 0, 15 * 60);
        return true;
    }
    return false;
}

bool BattleRoyaleMgr::SpawnSecureZone()
{
    if (obj_Centro)
    {
        if (obj_Zona) {
            obj_Zona->DespawnOrUnsummon();
            obj_Zona->Delete();
            obj_Zona = nullptr;
        }
        if (secureZoneIndex < CANTIDAD_DE_ZONAS) {
            obj_Zona = obj_Centro->SummonGameObject(OBJETO_ZONA_SEGURA_INICIAL + secureZoneIndex, BR_CentroDeMapas[indiceDelMapa].GetPositionX(), BR_CentroDeMapas[indiceDelMapa].GetPositionY(), BR_CentroDeMapas[indiceDelMapa].GetPositionZ() + BR_EscalasDeZonaSegura[secureZoneIndex] * 66.0f, 0, 0, 0, 0, 0, 2 * 60);
            obj_Zona->SetPhaseMask(2, true);
            obj_Zona->SetVisibilityDistanceOverride(VisibilityDistanceType::Infinite);
        }
        secureZoneIndex++;
        return true;
    }
    return false;
}

void BattleRoyaleMgr::TeleportPlayersToShip()
{
    if (!list_Jugadores.size()) return;
    for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
    {
        LlamarDentroDeNave((*it).first);
    }
}

void BattleRoyaleMgr::AddParachuteToAllPlayers()
{
    if (list_Jugadores.size())
    {
        for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
        {
            if (obj_Nave && (*it).second && (*it).second->GetTransport() && (*it).second->GetExactDist(obj_Nave) < 25.0f && (*it).second->IsAlive())
            {
                (*it).second->AddAura(HECHIZO_PARACAIDAS, (*it).second);
            }
        }
    }
}

void BattleRoyaleMgr::OutOfZoneDamage()
{
    if (list_Jugadores.size())
    {
        for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
        {
            if ((*it).second && (*it).second->IsAlive())
            {
                float distance = (*it).second->GetExactDist(obj_Centro);
                if (secureZoneIndex > 0 && distance > BR_EscalasDeZonaSegura[secureZoneIndex - 1] * 66.0f) {
                    list_Datos[(*it).first].dmg_tick++;
                    uint32 damage = (*it).second->GetMaxHealth() * (2 * sqrt(list_Datos[(*it).first].dmg_tick) + secureZoneIndex) / 100;
                    (*it).second->GetSession()->SendNotification("|cffff0000¡Has recibido |cffDA70D6%u|cffff0000 de daño, adéntrate en la zona segura!", damage);
                    Unit::DealDamage(nullptr, (*it).second, damage, nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false, true);
                } else {
                    list_Datos[(*it).first].dmg_tick = 0;
                }
            }
        }
    }
}

void BattleRoyaleMgr::AddFFAPvPFlag()
{
    if (list_Jugadores.size())
    {
        for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
        {
            if ((*it).second && (*it).second->IsAlive() && DebeForzarJcJTcT((*it).second) && !((*it).second->HasByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP)))
            {
                (*it).second->SetByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP);
            }
        }
    }
}
