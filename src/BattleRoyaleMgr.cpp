#include "BattleRoyaleMgr.h"
#include "Config.h"
#include "Chat.h"
#include "Player.h"

// -- CONSTANTES -- //
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

const float BRZonesShipStart[BRMapCount][4] =
{
    { 3067.581055f, -2164.183105f, 1609.483765f, 0.0f - M_PI / 2.0f }
};

const float ShipOffsets[25][2] = 
{
    { 0.0f, 0.0f },
    { 0.0f, 0.5f },
    { 0.0f, 1.0f },
    { 0.0f, 1.5f },
    { 0.0f, -0.5f },
    { 0.0f, -1.0f },
    { 0.0f, -1.5f },
    { 0.5f, 0.0f },
    { 0.5f, 0.5f },
    { 0.5f, 1.0f },
    { 0.5f, 1.5f },
    { 0.5f, -0.5f },
    { 0.5f, -1.0f },
    { 0.5f, -1.5f },
    { 1.0f, 0.0f },
    { 1.0f, 0.5f },
    { 1.0f, 1.0f },
    { 1.0f, 1.5f },
    { 1.0f, -0.5f },
    { 1.0f, -1.0f },
    { 1.0f, -1.5f },
    { 1.5f, 0.0f },
    { 1.5f, 0.5f },
    { 1.5f, 1.0f },
    { 1.5f, 1.5f },
    { 1.5f, -0.5f },
    { 1.5f, -1.0f },
    { 1.5f, -1.5f },
    { -0.5f, 0.0f },
    { -0.5f, 0.5f },
    { -0.5f, 1.0f },
    { -0.5f, 1.5f },
    { -0.5f, -0.5f },
    { -0.5f, -1.0f },
    { -0.5f, -1.5f },
    { -1.0f, 0.0f },
    { -1.0f, 0.5f },
    { -1.0f, 1.0f },
    { -1.0f, 1.5f },
    { -1.0f, -0.5f },
    { -1.0f, -1.0f },
    { -1.0f, -1.5f },
    { -1.5f, 0.0f },
    { -1.5f, 0.5f },
    { -1.5f, 1.0f },
    { -1.5f, 1.5f },
    { -1.5f, -0.5f },
    { -1.5f, -1.0f },
    { -1.5f, -1.5f }
}

const float BRSecureZoneZPlus[10] = { 
    297.0f,
    264.0f,
    231.0f,
    198.0f,
    165.0f,
    132.0f,
    99.0f,
    66.0f,
    33.0f,
    16.5f
};
const float BRSecureZoneDists[10] = {
    297.0f,
    264.0f,
    231.0f,
    198.0f,
    165.0f,
    132.0f,
    99.0f,
    66.0f,
    33.0f,
    16.5f
};

enum BREventStatus : int
{
    ST_NO_PLAYERS                           = 0, // No hay suficientes jugadores.
    ST_SUMMON_PLAYERS                       = 1, // Se ha comenzado a teletransportar jugadores a la zona central.
    ST_SHIP_WAITING                         = 2, // Ahora se estan moviendo a los jugadores a la nave en espera.
    ST_SHIP_IN_WAY                          = 3, // La nave esta en camino a su destino.
    ST_IN_PROGRESS                          = 4, // La batalla ha iniciado.
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
        case ST_NO_PLAYERS:
        {
            if (ep_PlayersQueue.size() >= eventMinPlayers) {
                TeleportToEvent(0);
            }
            break;
        }
        case ST_SUMMON_PLAYERS:
        case ST_SHIP_WAITING:
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
}

void BattleRoyaleMgr::HandleGiveReward(Player *player)
{
    // if (ep_Players.find(player->GetGUID().GetCounter()) == ep_Players.end() || !inTimeToEvent) return;

    // uint32 money;
    // std::string place;
    // if (nextReward == 1)
    // {
    //     money = 50000000;
    //     place = "PRIMER";
    // }
    // else if (nextReward == 2)
    // {
    //     money = 30000000;
    //     place = "SEGUNDO";
    // }
    // else if (nextReward == 3)
    // {
    //     money = 10000000;
    //     place = "TERCER";
    //     hasEventClose = true;
    // }
    // else return;

    // std::ostringstream msg;
    // msg << "|cff4CFF00BattleRoyale::|r Felicitaciones " << player->GetName().c_str() << " has quedado en el |cff4CFF00" << place << " LUGAR|r.";
    // sWorld->SendServerMessage(SERVER_MSG_STRING, msg.str().c_str());

    // std::string subject = "Parkour de la Muerte";
    // std::string text    = "Felicitaciones, has ganado en el evento, aqui tienes tu recompensa!";

    // // Enviado por Atencion al cliente
    // MailSender sender(MAIL_NORMAL, 0, MAIL_STATIONERY_GM);

    // // Llenar el correo
    // MailDraft draft(subject, text);

    // CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

    // // Agrega el objeto al correo si es el primer lugar (TODO: Y no tiene ya el objeto).
    // if (nextReward == 1)
    // {
    //     Item* item = Item::CreateItem(21176, 1, 0);
    //     item->SaveToDB(trans);
    //     draft.AddItem(item);
    // }

    // // Agrega la recompensa monetaria al correo.
    // draft.AddMoney(money);

    // // Envia el correo.
    // draft.SendMailTo(trans, MailReceiver(player, player->GetGUID().GetCounter()), sender);
    // CharacterDatabase.CommitTransaction(trans);

    // // Si ya ganaron los 3 lugares termina el evento.
    // nextReward++;
    // if (nextReward == 4)
    // {
    //     std::ostringstream msg2;
    //     msg2 << "|cff4CFF00BattleRoyale::|r El evento ha finalizado.";
    //     sWorld->SendServerMessage(SERVER_MSG_STRING, msg2.str().c_str());
    //     ExitFromEvent(0);
	// 	hasEventEnded = true;
    // }
    // else
    // {
    //     ExitFromEvent(player->GetGUID().GetCounter());
    // }
}

void BattleRoyaleMgr::StartEvent(uint32 guid)
{
	if (!guid)
	{
	 	for (BattleRoyalePlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it)
        {
            (*it).second->SetByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP);
        }
	}
	else
    {
        ep_Players[guid]->SetByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP);
    } 

    // TEST de Mover Nave
    uint32_t const autoCloseTime = go_TransportShip->GetGOInfo()->GetAutoCloseTime() ? 10000u : 0u;
    go_TransportShip->SetLootState(GO_READY);
    go_TransportShip->UseDoorOrButton(autoCloseTime, false, nullptr);

    eventCurrentStatus = ST_IN_PROGRESS;
    secureZoneIndex = 0;
    secureZoneDelay = 0;
    secureZoneAnnounced = false;
    go_SecureZone = nullptr;
    go_CenterOfBattle = nullptr;
}

void BattleRoyaleMgr::TeleportToEvent(uint32 guid)
{
	if (!guid)
	{
        if (eventCurrentStatus != ST_NO_PLAYERS) return;
        summonRemainingTime = 60;
        eventCurrentStatus = ST_SUMMON_PLAYERS;
        for (BattleRoyalePlayerQueue::iterator it = ep_PlayersQueue.begin(); it != ep_PlayersQueue.end(); ++it)
		{
            uint32 guid = (*it).first;
            ep_Players[guid] = (*it).second;

            // TODO: Si esta en intancia o transporte entonces almacenar coordenadas de hogar.
            ep_PlayersData[guid].SetPosition(ep_Players[guid]->GetMapId(), ep_Players[guid]->GetPositionX(), ep_Players[guid]->GetPositionY(), ep_Players[guid]->GetPositionZ(), ep_Players[guid]->GetOrientation());

			ep_Players[guid]->TeleportTo(BRMapID[rotationMapIndex], BRZonesCenter[rotationMapIndex].GetPositionX(), BRZonesCenter[rotationMapIndex].GetPositionY(), BRZonesCenter[rotationMapIndex].GetPositionZ(), 0.0f);
            EnterToPhaseEvent(guid);
            ep_Players[guid]->SaveToDB(false, false);
		}
        for (BattleRoyalePlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it) ep_PlayersQueue.erase((*it).first);
    }
	else
	{
        if (eventCurrentStatus == ST_SUMMON_PLAYERS)
        {
            ep_Players[guid] = ep_PlayersQueue[guid];
            ep_PlayersData[guid].SetPosition(ep_Players[guid]->GetMapId(), ep_Players[guid]->GetPositionX(), ep_Players[guid]->GetPositionY(), ep_Players[guid]->GetPositionZ(), ep_Players[guid]->GetOrientation());
            ep_Players[guid]->TeleportTo(BRMapID[rotationMapIndex], BRZonesCenter[rotationMapIndex].GetPositionX(), BRZonesCenter[rotationMapIndex].GetPositionY(), BRZonesCenter[rotationMapIndex].GetPositionZ(), 0.0f);
            EnterToPhaseEvent(guid);
            ep_Players[guid]->SaveToDB(false, false);
            ep_PlayersQueue.erase(guid);
        } 
        else if (eventCurrentStatus == ST_SHIP_WAITING)
        {
            ep_Players[guid] = ep_PlayersQueue[guid];
            ep_PlayersData[guid].SetPosition(ep_Players[guid]->GetMapId(), ep_Players[guid]->GetPositionX(), ep_Players[guid]->GetPositionY(), ep_Players[guid]->GetPositionZ(), ep_Players[guid]->GetOrientation());
            ep_Players[guid]->TeleportTo(BRMapID[rotationMapIndex], BRZonesShipStart[rotationMapIndex][0], BRZonesShipStart[rotationMapIndex][1], BRZonesShipStart[rotationMapIndex][2], 0.0f);
            EnterToPhaseEvent(guid);
            ep_Players[guid]->SaveToDB(false, false);
            ep_PlayersQueue.erase(guid);
        }
	}
}

void BattleRoyaleMgr::ExitFromEvent(uint32 guid)
{
	if (!guid)
	{
		for (BattleRoyalePlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it)
		{
            if (!(*it).second->IsAlive()) ResurrectPlayer((*it).second);
            ExitFromPhaseEvent((*it).first);
            (*it).second->TeleportTo(ep_PlayersData[(*it).first].GetMap(), ep_PlayersData[(*it).first].GetX(), ep_PlayersData[(*it).first].GetY(), ep_PlayersData[(*it).first].GetZ(), ep_PlayersData[(*it).first].GetO());
            (*it).second->SaveToDB(false, false);
            ep_Players.erase((*it).first);
		    ep_PlayersData.erase((*it).first);
		}

        // TODO: Esto no debe ir aqui.
        go_CenterOfBattle->GetMap()->SetVisibilityRange(World::GetMaxVisibleDistanceOnContinents());
	}
	else
	{
        if (!ep_Players[guid]->IsAlive()) ResurrectPlayer(ep_Players[guid]);
        ExitFromPhaseEvent(guid);
        ep_Players[guid]->TeleportTo(ep_PlayersData[guid].GetMap(), ep_PlayersData[guid].GetX(), ep_PlayersData[guid].GetY(), ep_PlayersData[guid].GetZ(), ep_PlayersData[guid].GetO());
        ep_Players[guid]->SaveToDB(false, false);
        ep_Players.erase(guid);
	    ep_PlayersData.erase(guid);
	}
}

void BattleRoyaleMgr::HandleReleaseGhost(Player *player, uint32 oldArea, uint32 newArea)
{
    // if (ep_Players.find(player->GetGUID().GetCounter()) == ep_Players.end()) return;
	// if ((oldArea == 1741 || oldArea == 2177) && newArea == 1741 && !player->IsAlive())
    // {
    //     player->TeleportTo(0, -13246.281f, 193.465f, 31.019f, 1.130f);
    //     ResurrectPlayer(player);
    // }
    // else if ((oldArea == 1741 || oldArea == 2177) && newArea != 1741 && newArea != 2177)
    // {
    //     if (!player->IsAlive()) ResurrectPlayer(player);
    //     ExitFromPhaseEvent(player->GetGUID().GetCounter());
    //     ep_Players.erase(player->GetGUID().GetCounter());
    //     ep_PlayersData.erase(player->GetGUID().GetCounter());
    //     ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r Has abandonado la zona del evento.");
    // }
}

void BattleRoyaleMgr::HandleOnWoldUpdate(uint32 diff)
{
    switch(eventCurrentStatus)
    {
        case ST_SUMMON_PLAYERS:
        case ST_SHIP_WAITING:
        {
            if (secondsTicksHelper <= 0) {
                secondsTicksHelper = 1000;
                if (summonRemainingTime <= 0) {
                    SendNotificationStart(0, 0);
                    StartEvent(0);
                } else {
                    int srt = summonRemainingTime;
                    if (srt == 5 || srt == 10 || srt == 15 || srt == 20 || srt == 25 || srt == 30 || srt == 35 || srt == 40 || srt == 45 || srt == 50 || srt == 55 || srt == 60) {
                        SendNotificationStart(0, srt);
                    }
                    if (eventCurrentStatus == ST_SUMMON_PLAYERS && summonRemainingTime <= 30) {
                        eventCurrentStatus = ST_SHIP_WAITING;
                        if (!SpawnTheCenterOfBattle()) {
                            ResetFullEvent();
                            return;
                        }
                        SpawnTransportShip();
                        TeleportPlayersToShip();
                    }
                    summonRemainingTime--;
                }
            } else {
                secondsTicksHelper -= diff;
            }
            break;
        }
        case ST_IN_PROGRESS:
        {
            if (secondsTicksHelper <= 0) {
                secondsTicksHelper = 1000;
                OutOfZoneDamage();
            } else {
                secondsTicksHelper -= diff;
            }
            if (secureZoneDelay <= 0) {
                SpawnSecureZone();
                secureZoneIndex++;
                secureZoneDelay = 60000;
                secureZoneAnnounced = false;
            } else {
                if (secureZoneDelay <= 5000 && !secureZoneAnnounced) {
                    SendNotification(0, 5);
                    secureZoneAnnounced = true;
                }
                if (secureZoneIndex <= 10) {
                    secureZoneDelay -= diff;
                } else {
                    ExitFromEvent(0); // TODO: Esto no va aqui.
                    eventCurrentStatus = ST_NO_PLAYERS;  // TODO: Esto no va aqui.
                }
            }
            break;
        }
    }
}

bool BattleRoyaleMgr::ForceFFAPvPFlag(Player* player)
{
    if (eventCurrentStatus != ST_IN_PROGRESS || ep_Players.find(player->GetGUID().GetCounter()) == ep_Players.end()) return false;
    return true;
}

bool BattleRoyaleMgr::RestrictPlayerFunctions(Player* player)
{
    if (eventCurrentStatus > ST_NO_PLAYERS && ep_Players.find(player->GetGUID().GetCounter()) != ep_Players.end()) return true;
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

void BattleRoyaleMgr::ResurrectPlayer(Player *player)
{
	player->ResurrectPlayer(1.0f);
    player->SpawnCorpseBones();
    player->SaveToDB(false, false);
}

void BattleRoyaleMgr::SendNotification(uint32 guid, uint32 delay)
{
    if (!guid)
        for (BattleRoyalePlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it)
			(*it).second->GetSession()->SendNotification("|cff00ff00¡La zona segura se reducirá en |cffDA70D6%u|cff00ff00 segundos!", delay);
    else ep_Players[guid]->GetSession()->SendNotification("|cff00ff00¡La zona segura se reducirá en |cffDA70D6%u|cff00ff00 segundos!", delay);
}

void BattleRoyaleMgr::SendNotificationStart(uint32 guid, uint32 delay)
{
    if (!guid){
        for (BattleRoyalePlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it) {
            if (delay == 0){
                (*it).second->GetSession()->SendNotification("|cff00ff00¡Que comience la batalla de |cffDA70D6%s|cff00ff00!", BRZonesNames[rotationMapIndex]);
            } else {
                (*it).second->GetSession()->SendNotification("|cff00ff00¡La batalla iniciará en |cffDA70D6%u|cff00ff00 segundos!", delay);
            }
        }
    } else ep_Players[guid]->GetSession()->SendNotification("|cff00ff00¡La batalla iniciará en |cffDA70D6%u|cff00ff00 segundos!", delay);
}

bool BattleRoyaleMgr::SpawnTheCenterOfBattle()
{
    if (ep_Players.size() == 0) return false;
    bool success = false;
    for (BattleRoyalePlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it)
    {
        if ((*it).second) {
            if (go_CenterOfBattle) {
                go_CenterOfBattle->DespawnOrUnsummon();
                go_CenterOfBattle->Delete();
                go_CenterOfBattle = nullptr;
            }
            go_CenterOfBattle = (*it).second->SummonGameObject(500010, BRZonesCenter[rotationMapIndex].GetPositionX(), BRZonesCenter[rotationMapIndex].GetPositionY(), BRZonesCenter[rotationMapIndex].GetPositionZ(), 0, 0, 0, 0, 0, 15 * 60);
            success = true;
            break;
        }
    }
    return success;
}

void BattleRoyaleMgr::SpawnTransportShip()
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
    go_TransportShip = go_CenterOfBattle->SummonGameObject(194675, x, y, z, o, 0, 0, rot2, rot3, 2 * 60);
}

void BattleRoyaleMgr::SpawnSecureZone()
{
    // return; // TODO
    if (go_SecureZone) {
        go_SecureZone->DespawnOrUnsummon();
        go_SecureZone->Delete();
        go_SecureZone = nullptr;
    }
    if (secureZoneIndex < 10) {
        go_SecureZone = go_CenterOfBattle->SummonGameObject(500000 + secureZoneIndex, BRZonesCenter[rotationMapIndex].GetPositionX(), BRZonesCenter[rotationMapIndex].GetPositionY(), BRZonesCenter[rotationMapIndex].GetPositionZ() + BRSecureZoneZPlus[secureZoneIndex], 0, 0, 0, 0, 0, 2 * 60);
        go_SecureZone->SetPhaseMask(2, true);
        go_SecureZone->GetMap()->SetVisibilityRange(500.0f);
        go_SecureZone->SetVisibilityDistanceOverride(VisibilityDistanceType::Infinite);
    }
}

void BattleRoyaleMgr::TeleportPlayersToShip()
{
    if (ep_Players.size() == 0) return;
    for (BattleRoyalePlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it)
    {
        (*it).second->TeleportTo(BRMapID[rotationMapIndex], BRZonesShipStart[rotationMapIndex][0], BRZonesShipStart[rotationMapIndex][1], BRZonesShipStart[rotationMapIndex][2] + 2.5f, 0.0f);
    }
}

void BattleRoyaleMgr::OutOfZoneDamage()
{
    return; // TODO
    for (BattleRoyalePlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it)
    {
        float distance = (*it).second->GetExactDist(go_CenterOfBattle);
        if (secureZoneIndex > 0 && distance > BRSecureZoneDists[secureZoneIndex - 1]) {
            ep_PlayersData[(*it).first].SetDTick(ep_PlayersData[(*it).first].GetDTick() + 1);
            uint32 damage = (*it).second->GetMaxHealth() * (2 * sqrt(ep_PlayersData[(*it).first].GetDTick()) + secureZoneIndex) / 100;
            (*it).second->GetSession()->SendNotification("|cffff0000¡Has recibido |cffDA70D6%u|cffff0000 de daño, adéntrate en la zona segura!", damage);
            Unit::DealDamage(nullptr, (*it).second, damage, nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false, true);
        } else {
            ep_PlayersData[(*it).first].SetDTick(0);
        }
    }
}

void BattleRoyaleMgr::ResetFullEvent()
{
    ep_PlayersQueue.clear();
	ep_Players.clear();
    ep_PlayersData.clear();
    rotationMapIndex = 0;
    eventCurrentStatus = ST_NO_PLAYERS;
    secondsTicksHelper = 1000;
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
