#include "BattleRoyaleMgr.h"
#include "Config.h"
#include "Chat.h"
#include "Player.h"

// -- CONSTANTES -- //
const int secureZoneUpdateInterval = 60000; // TODO: Configuracion (defaul: 60000).

const int BRMapCount = 1;
const int BRMapID[BRMapCount] = { 1 };

const Position BRZonesCenter[BRMapCount] =
{
    { 5261.581055f, -2164.183105f, 1259.483765f }       // 1: Kalimdor: Hyjal
};

const std::string BRZonesNames[BRMapCount] =
{
    "Kalimdor: Hyjal"
};

// DIST: 2294, ALT: 297.
const float BRZonesShipStart[BRMapCount][4] =
{
    { 2967.581055f, -2164.183105f, 1556.483765f, 0.0f - M_PI / 2.0f }
};

// -- FUNCIONES -- //
BattleRoyaleMgr::BattleRoyaleMgr()
{
    eventMinPlayers = sConfigMgr->GetOption<int32>("BattleRoyale.MinPlayers", 25);
    eventMaxPlayers = sConfigMgr->GetOption<int32>("BattleRoyale.MaxPlayers", 50);
    go_SecureZone = nullptr;
    go_CenterOfBattle = nullptr;
    go_TransportShip = nullptr;
    ResetFullEvent();
}

BattleRoyaleMgr::~BattleRoyaleMgr()
{
    ep_PlayersQueue.clear();
	ep_Players.clear();
    ep_PlayersData.clear();
}

void BattleRoyaleMgr::HandlePlayerJoin(Player *player)
{
    uint32 guid = player->GetGUID().GetCounter();
    if (ep_PlayersQueue.find(guid) != ep_PlayersQueue.end())
    {
        ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r Ya estas en cola para el evento.");
        return;
    }
    if (ep_Players.find(guid) != ep_Players.end())
    {
        ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r Ya estas dentro del evento.");
        return;
    }
    ep_PlayersQueue[guid] = player;
    ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r Te has unido a la cola del evento. Jugadores en cola: %u/%u.", ep_PlayersQueue.size(), eventMinPlayers);
    switch (eventCurrentStatus)
    {
        case STATUS_NO_ENOUGH_PLAYERS:
        {
            if (ep_PlayersQueue.size() >= eventMinPlayers) {
                TeleportToEvent(0);
            }
            break;
        }
        case STATUS_SUMMONING_PLAYERS:
        case STATUS_SHIP_WAITING:
        {
            if (ep_Players.size() < eventMaxPlayers) {
                TeleportToEvent(guid);
            }
            break;
        }
    }
}

void BattleRoyaleMgr::HandlePlayerLogout(Player *player)
{
    uint32 guid = player->GetGUID().GetCounter();
    if (ep_PlayersQueue.find(guid) != ep_PlayersQueue.end()) {
        ep_PlayersQueue.erase(guid);
    };
    if (ep_Players.find(guid) != ep_Players.end()) {
        ExitFromPhaseEvent(guid);
        ep_Players.erase(guid);
        ep_PlayersData.erase(guid);
    }

    // TEMP: Finalizar evento al no quedar nadie en el.
    if (!ep_Players.size())
    {
        eventCurrentStatus = STATUS_NO_ENOUGH_PLAYERS;
    }
}

void BattleRoyaleMgr::HandleOnPVPLill(Player *killer, Player *killed)
{
    if (!killer || !killed) return;
    if (!ep_Players.size() || eventCurrentStatus != STATUS_BATTLE_STARTED) return;
    uint32 guid_r = killer->GetGUID().GetCounter();
    uint32 guid_d = killed->GetGUID().GetCounter();
    if (ep_Players.find(guid_r) == ep_Players.end() || ep_Players.find(guid_d) == ep_Players.end()) return;
    killed->CastSpell(killer, 6277, true);
    ChatHandler handler = ChatHandler(killer->GetSession());
    NotifyPvPKill(handler.GetNameLink(killer), handler.GetNameLink(killed));
}

/**
 * @brief Decide si se debe retornar al jugador o dejar que valla al cementerio.
 * 
 * @param player 
 * @return true: Retornado!
 * @return false: Dejar por defecto!
 */
bool BattleRoyaleMgr::HandleReleaseGhost(Player *player)
{
    if (!ep_Players.size()) return false;
    uint32 guid = player->GetGUID().GetCounter();
    if (ep_Players.find(guid) == ep_Players.end()) return false;
    if (!player->isPossessing())
    {
        player->StopCastingBindSight();
    }
    if (!player->IsAlive()) ResurrectPlayer(player);
    ExitFromPhaseEvent(guid);
    player->TeleportTo(ep_PlayersData[guid].GetMap(), ep_PlayersData[guid].GetX(), ep_PlayersData[guid].GetY(), ep_PlayersData[guid].GetZ(), ep_PlayersData[guid].GetO());
    player->SaveToDB(false, false);
    ep_Players.erase(guid);
	ep_PlayersData.erase(guid);
    
    // TEMP: Finalizar evento al no quedar nadie en el.
    if (!ep_Players.size())
    {
        eventCurrentStatus = STATUS_NO_ENOUGH_PLAYERS;
    }

    return true;
}

/**
 * @brief Teletransporta todos los personajes al aire sobre el spawn de la nave. / De manera individual elije si mandar a un personaje a la nave o encima en el aire.
 * 
 * @param guid 
 */
void BattleRoyaleMgr::TeleportToEvent(uint32 guid)
{
	if (!guid)
	{
        if (eventCurrentStatus != STATUS_NO_ENOUGH_PLAYERS) return;
        startRemainingTime = 65;
        eventCurrentStatus = STATUS_SUMMONING_PLAYERS;
        for (BattleRoyalePlayerQueue::iterator it = ep_PlayersQueue.begin(); it != ep_PlayersQueue.end(); ++it)
		{
            uint32 guid = (*it).first;
            ep_Players[guid] = (*it).second;
            StorePlayerStartPosition(guid);
            TeleportPlayerBeforeShip(guid);            
            EnterToPhaseEvent(guid);
            ep_Players[guid]->SaveToDB(false, false);
		}
        for (BattleRoyalePlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it) ep_PlayersQueue.erase((*it).first);
    }
	else
	{
        if (eventCurrentStatus == STATUS_SUMMONING_PLAYERS)
        {
            ep_Players[guid] = ep_PlayersQueue[guid];
            StorePlayerStartPosition(guid);
            TeleportPlayerBeforeShip(guid);            
            EnterToPhaseEvent(guid);
            ep_Players[guid]->SaveToDB(false, false);
            ep_PlayersQueue.erase(guid);
        } 
        else if (eventCurrentStatus == STATUS_SHIP_WAITING)
        {
            ep_Players[guid] = ep_PlayersQueue[guid];
            StorePlayerStartPosition(guid);
            TeleportPlayerToShip(guid);
            EnterToPhaseEvent(guid);
            ep_Players[guid]->SaveToDB(false, false);
            ep_PlayersQueue.erase(guid);
        }
	}
}

void BattleRoyaleMgr::HandleOnWoldUpdate(uint32 diff)
{
    switch(eventCurrentStatus)
    {
        case STATUS_SUMMONING_PLAYERS:
        case STATUS_SHIP_WAITING:
        case STATUS_SHIP_MOVING:
        case STATUS_SHIP_NEAR_CENTER:
        {
            if (secondsTicksHelper <= 0) {
                secondsTicksHelper = 1000;
                if (startRemainingTime <= 0) {
                    eventCurrentStatus = STATUS_BATTLE_STARTED;
                    NotifyTimeRemainingToStart(0);
                    secureZoneDelay = secureZoneUpdateInterval;
                } else {
                    if (startRemainingTime % 5 == 0) {
                        NotifyTimeRemainingToStart(startRemainingTime);
                    }
                    if (eventCurrentStatus == STATUS_SUMMONING_PLAYERS && startRemainingTime <= 60)
                    {
                        eventCurrentStatus = STATUS_SHIP_WAITING;
                        if (!SpawnTransportShip()) {
                            ResetFullEvent();
                            return;
                        }
                        TeleportPlayersToShip();
                    }
                    if (eventCurrentStatus == STATUS_SHIP_WAITING && startRemainingTime <= 30 && go_TransportShip)
                    {
                        eventCurrentStatus = STATUS_SHIP_MOVING;
                        uint32_t const autoCloseTime = go_TransportShip->GetGOInfo()->GetAutoCloseTime() ? 10000u : 0u;
                        go_TransportShip->SetLootState(GO_READY);
                        go_TransportShip->UseDoorOrButton(autoCloseTime, false, nullptr);
                    }
                    if (eventCurrentStatus == STATUS_SHIP_MOVING && startRemainingTime <= 5)
                    {
                        eventCurrentStatus = STATUS_SHIP_NEAR_CENTER;
                        secureZoneIndex = 0;
                        secureZoneDelay = 0;
                        secureZoneAnnounced = false;
                        go_CenterOfBattle = nullptr;
                        go_SecureZone = nullptr;
                        if (!SpawnTheCenterOfBattle()) {
                            ResetFullEvent();
                            return;
                        }
                        if (!SpawnSecureZone()) {
                            ResetFullEvent();
                            return;
                        }
                        AddParachuteToAllPlayers();
                    }
                    startRemainingTime--;
                }
            } else {
                secondsTicksHelper -= diff;
            }
            break;
        }
        case STATUS_BATTLE_STARTED:
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
                secureZoneDelay = secureZoneUpdateInterval;
                secureZoneAnnounced = false;
            } else {
                if (secureZoneDelay <= 5000 && !secureZoneAnnounced) {
                    NotifySecureZoneReduceWarn(5);
                    secureZoneAnnounced = true;
                }
                if (secureZoneIndex <= SECURE_ZONE_COUNT) {
                    secureZoneDelay -= diff;
                }
            }
            break;
        }
    }
}

bool BattleRoyaleMgr::ForceFFAPvPFlag(Player* player)
{
    if (eventCurrentStatus != STATUS_BATTLE_STARTED || !ep_Players.size() || ep_Players.find(player->GetGUID().GetCounter()) == ep_Players.end()) return false;
    return !(go_TransportShip && player->GetTransport() && player->GetExactDist(go_TransportShip) < 25.0f);
}

void BattleRoyaleMgr::PreventPvPBeforeBattle(Player* player, bool state)
{
    if (!state || !ep_Players.size()) return;
    if (ep_Players.find(player->GetGUID().GetCounter()) != ep_Players.end() && !ForceFFAPvPFlag(player)) player->SetPvP(false);
}

bool BattleRoyaleMgr::RestrictPlayerFunctions(Player* player)
{
    if (eventCurrentStatus > STATUS_NO_ENOUGH_PLAYERS && ep_Players.find(player->GetGUID().GetCounter()) != ep_Players.end()) return true;
    return false;
}

// -- Private functions -- //
void BattleRoyaleMgr::EnterToPhaseEvent(uint32 guid)
{
	ep_Players[guid]->SetPhaseMask(2, false);
    ep_Players[guid]->UpdateObjectVisibility();
}

void BattleRoyaleMgr::ExitFromPhaseEvent(uint32 guid)
{
	ep_Players[guid]->SetPhaseMask(1, false);
    ep_Players[guid]->UpdateObjectVisibility();
}

void BattleRoyaleMgr::ResurrectPlayer(Player* player)
{
	player->ResurrectPlayer(1.0f);
    player->SpawnCorpseBones();
    player->SaveToDB(false, false);
}

void BattleRoyaleMgr::NotifySecureZoneReduceWarn(uint32 delay)
{
    if (ep_Players.size())
    {
        for (BattleRoyalePlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it)
        {
            (*it).second->GetSession()->SendNotification("|cff00ff00¡La zona segura se reducirá en |cffDA70D6%u|cff00ff00 segundos!", delay);
        }
    }
}

void BattleRoyaleMgr::NotifySecureZoneReduced()
{
    if (ep_Players.size())
    {
        for (BattleRoyalePlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it)
        {
            (*it).second->GetSession()->SendNotification("|cffff0000¡ALERTA: La zona segura se ha actualizado!");
        }
    }
}

/**
 * @brief Sistema de notificaciones previas a la batalla.
 * 
 * @param delay 
 */
void BattleRoyaleMgr::NotifyTimeRemainingToStart(uint32 delay)
{
    if (ep_Players.size())
    {
        for (BattleRoyalePlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it) {
            switch (delay)
            {
                case 0:
                {
                    (*it).second->GetSession()->SendNotification("|cff00ff00¡Que comience la batalla de |cffDA70D6%s|cff00ff00!", BRZonesNames[rotationMapIndex].c_str());
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

/**
 * @brief Notifica los KILL JcJ.
 * 
 * @param handler 
 * @param killer 
 * @param killed 
 */
void BattleRoyaleMgr::NotifyPvPKill(std::string killer, std::string killed)
{
    if (ep_Players.size())
    {
        for (BattleRoyalePlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it)
        {
            ChatHandler((*it).second->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡%s ha eliminado a %s!.", killer, killed);
        }
    }
}

/**
 * @brief Hace que el primer jugador de la lista invoque la nave.
 * 
 * @return true 
 * @return false 
 */
bool BattleRoyaleMgr::SpawnTransportShip()
{
    bool success = false;
    if (ep_Players.size())
    {
        for (BattleRoyalePlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it)
        {
            if ((*it).second)
            {
                if (go_TransportShip) {
                    go_TransportShip->DespawnOrUnsummon();
                    go_TransportShip->Delete();
                    go_TransportShip = nullptr;
                }
                float x = BRZonesShipStart[rotationMapIndex][0];
                float y = BRZonesShipStart[rotationMapIndex][1];
                float z = BRZonesShipStart[rotationMapIndex][2];
                float o = BRZonesShipStart[rotationMapIndex][3];
                float rot2 = std::sin(o / 2);
                float rot3 = cos(o / 2);
                go_TransportShip = (*it).second->SummonGameObject(GAMEOBJECT_SHIP, x, y, z, o, 0, 0, rot2, rot3, 2 * 60);
                success = true;
                break;
            }
        }
    }
    return success;
}

/**
 * @brief Hace que la propia nave invoque el centro de la batalla.
 * 
 * @return true 
 * @return false 
 */
bool BattleRoyaleMgr::SpawnTheCenterOfBattle()
{
    if (go_TransportShip)
    {
        if (go_CenterOfBattle) {
            go_CenterOfBattle->DespawnOrUnsummon();
            go_CenterOfBattle->Delete();
            go_CenterOfBattle = nullptr;
        }
        go_CenterOfBattle = go_TransportShip->SummonGameObject(GAMEOBJECT_MAP_CENTER, BRZonesCenter[rotationMapIndex].GetPositionX(), BRZonesCenter[rotationMapIndex].GetPositionY(), BRZonesCenter[rotationMapIndex].GetPositionZ(), 0, 0, 0, 0, 0, 15 * 60);
        return true;
    }
    return false;
}

/**
 * @brief Hace que el centro de la batalla invoque a la zona segura.
 * 
 * @return true 
 * @return false 
 */
bool BattleRoyaleMgr::SpawnSecureZone()
{
    if (go_CenterOfBattle)
    {
        if (go_SecureZone) {
            go_SecureZone->DespawnOrUnsummon();
            go_SecureZone->Delete();
            go_SecureZone = nullptr;
        }
        if (secureZoneIndex < SECURE_ZONE_COUNT) {
            go_SecureZone = go_CenterOfBattle->SummonGameObject(GAMEOBJECT_SECURE_ZONE_BASE + secureZoneIndex, BRZonesCenter[rotationMapIndex].GetPositionX(), BRZonesCenter[rotationMapIndex].GetPositionY(), BRZonesCenter[rotationMapIndex].GetPositionZ() + BRSecureZoneScales[secureZoneIndex] * 66.0f, 0, 0, 0, 0, 0, 2 * 60);
            go_SecureZone->SetPhaseMask(2, true);
            go_SecureZone->SetVisibilityDistanceOverride(VisibilityDistanceType::Infinite);
        }
        secureZoneIndex++;
        return true;
    }
    return false;
}

/**
 * @brief Almacena la posicion a la que se debe enviar al personaje tras salir del evento.
 * 
 * @param guid 
 */
void BattleRoyaleMgr::StorePlayerStartPosition(uint32 guid)
{
    if (ep_Players.find(guid) != ep_Players.end())
    {
        if (ep_Players[guid]->GetMap()->Instanceable())
        {
            ep_PlayersData[guid].SetPosition(ep_Players[guid]->m_homebindMapId, ep_Players[guid]->m_homebindX, ep_Players[guid]->m_homebindY, ep_Players[guid]->m_homebindZ, ep_Players[guid]->GetOrientation());
        }            
        else
        {
            ep_PlayersData[guid].SetPosition(ep_Players[guid]->GetMapId(), ep_Players[guid]->GetPositionX(), ep_Players[guid]->GetPositionY(), ep_Players[guid]->GetPositionZ(), ep_Players[guid]->GetOrientation());
        }
    }
}

/**
 * @brief Teletransporta a un personaje unas yardas encima de donde aparecera la nave y le pone paracaidas.
 * 
 * @param guid 
 */
void BattleRoyaleMgr::TeleportPlayerBeforeShip(uint32 guid)
{
    
    if (ep_Players.find(guid) != ep_Players.end())
    {
        float ox = ShipOffsets[summonOffsetIndex][0];
        float oy = ShipOffsets[summonOffsetIndex][1];
        summonOffsetIndex++;
        if (summonOffsetIndex >= BROffsetsCount) summonOffsetIndex = 0;
        Dismount(ep_Players[guid]);
        ep_Players[guid]->SetPvP(false);
        ep_Players[guid]->TeleportTo(BRMapID[rotationMapIndex], BRZonesShipStart[rotationMapIndex][0] + ox, BRZonesShipStart[rotationMapIndex][1] + oy, BRZonesShipStart[rotationMapIndex][2] + 15.0f, 0.0f);
        ep_Players[guid]->AddAura(SPELL_PARACHUTE_DALARAN, ep_Players[guid]);
    }
}

/**
 * @brief Teletransporta a un personaje a la nave.
 * 
 * @param guid 
 */
void BattleRoyaleMgr::TeleportPlayerToShip(uint32 guid)
{
    if (ep_Players.find(guid) != ep_Players.end())
    {
        float ox = ShipOffsets[summonOffsetIndex][0];
        float oy = ShipOffsets[summonOffsetIndex][1];
        summonOffsetIndex++;
        if (summonOffsetIndex >= BROffsetsCount) summonOffsetIndex = 0;
        Dismount(ep_Players[guid]);
        ep_Players[guid]->SetPvP(false);
        ep_Players[guid]->TeleportTo(BRMapID[rotationMapIndex], BRZonesShipStart[rotationMapIndex][0] + ox, BRZonesShipStart[rotationMapIndex][1] + oy, BRZonesShipStart[rotationMapIndex][2] + 1.5f, 0.0f);
    }
}

/**
 * @brief Teletransporta a todos los personajes del evento hacia el interior de la nave.
 * 
 */
void BattleRoyaleMgr::TeleportPlayersToShip()
{
    if (ep_Players.size() == 0) return;
    for (BattleRoyalePlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it)
    {
        TeleportPlayerToShip((*it).first);
    }
}

/**
 * @brief Desmonta a un personaje si se encuentra montado y no es en ruta de vuelo.
 * 
 * @param player 
 */
void BattleRoyaleMgr::Dismount(Player* player)
{
    if (player && player->IsMounted())
    {
        if (!player->IsInFlight())
        {
            player->Dismount();
            player->RemoveAurasByType(SPELL_AURA_MOUNTED);
            player->SetSpeed(MOVE_RUN, 1, true);
            player->SetSpeed(MOVE_FLIGHT, 1, true);
        }
    }
}

/**
 * @brief Coloca un paracaidas a todos los participantes del evento (en la nave) que se activa al estar en el aire.
 * 
 */
void BattleRoyaleMgr::AddParachuteToAllPlayers()
{
    if (ep_Players.size())
    {
        for (BattleRoyalePlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it)
        {
            // TODO: Condiciones para poner el paracaidas (Que exista, que se encuentre en la nave).
            (*it).second->AddAura(SPELL_PARACHUTE_DALARAN, (*it).second);
        }
    }
}

/**
 * @brief Causa daño a todos los que esten fuera de la zona segura.
 * 
 */
void BattleRoyaleMgr::OutOfZoneDamage()
{
    if (ep_Players.size())
    {
        for (BattleRoyalePlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it)
        {
            float distance = (*it).second->GetExactDist(go_CenterOfBattle);
            if (secureZoneIndex > 0 && distance > BRSecureZoneScales[secureZoneIndex - 1] * 66.0f) {
                ep_PlayersData[(*it).first].SetDTick(ep_PlayersData[(*it).first].GetDTick() + 1);
                uint32 damage = (*it).second->GetMaxHealth() * (2 * sqrt(ep_PlayersData[(*it).first].GetDTick()) + secureZoneIndex) / 100;
                (*it).second->GetSession()->SendNotification("|cffff0000¡Has recibido |cffDA70D6%u|cffff0000 de daño, adéntrate en la zona segura!", damage);
                Unit::DealDamage(nullptr, (*it).second, damage, nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false, true);
            } else {
                ep_PlayersData[(*it).first].SetDTick(0);
            }
        }
    }
}

/**
 * @brief Controla cuando poner al jugador en modo FFA PvP.
 * 
 */
void BattleRoyaleMgr::AddFFAPvPFlag()
{
    if (ep_Players.size())
    {
        for (BattleRoyalePlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it)
        {
            if ((*it).second && (*it).second->IsAlive() && ForceFFAPvPFlag((*it).second) && !((*it).second->HasByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP)))
            {
                (*it).second->SetByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP);
            }
        }
    }
}

/**
 * @brief Reinicia el valor de todas las variables del evento (El boton de pánico).
 * 
 */
void BattleRoyaleMgr::ResetFullEvent()
{
    ep_PlayersQueue.clear();
	ep_Players.clear();
    ep_PlayersData.clear();
    rotationMapIndex = 0;
    eventCurrentStatus = STATUS_NO_ENOUGH_PLAYERS;
    secondsTicksHelper = 1000;
    summonOffsetIndex = 0;
    if (go_SecureZone) {
        go_SecureZone->DespawnOrUnsummon();
        go_SecureZone->Delete();
        go_SecureZone = nullptr;
    }
    if (go_CenterOfBattle) {
        go_CenterOfBattle->DespawnOrUnsummon();
        go_CenterOfBattle->Delete();
        go_CenterOfBattle = nullptr;
    }
    if (go_TransportShip) {
        go_TransportShip->DespawnOrUnsummon();
        go_TransportShip->Delete();
        go_TransportShip = nullptr;
    }
}
