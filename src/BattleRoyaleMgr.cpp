#include "BattleRoyaleMgr.h"
#include "Config.h"
#include "Chat.h"
#include "Player.h"

// -- CONSTANTES -- //
const int TEMP_CURRENT_MAP_INDEX = 1;
const int BRMapCount = 2;

const int BRMapID[BRMapCount] = { 1, 1 };

const Position BRZonesCenter[BRMapCount] =
{
    { 5411.041504f, -2701.873779f, 1451.660522f },      // 1: Kalimdor: Hyjal
    { 5261.581055f, -2164.183105f, 1259.483765f }       // 1: Kalimdor: Hyjal
};

const std::string BRZonesNames[BRMapCount] =
{
    "Kalimdor: Hyjal",
    "Kalimdor: Hyjal"
};

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
    ST_NO_PLAYERS                           = 0,
    ST_SUMMON_PLAYERS                       = 1,
    ST_IN_PROGRESS                          = 2,
    ST_ENDING                               = 3,
};

// -- FUNCIONES -- //
BattleRoyaleMgr::BattleRoyaleMgr()
{
    rotationMapIndex = 0;
    eventCurrentStatus = ST_NO_PLAYERS;
    eventMinPlayers = sConfigMgr->GetOption<int32>("BattleRoyale.MinPlayers", 25);
    eventMaxPlayers = sConfigMgr->GetOption<int32>("BattleRoyale.MaxPlayers", 50);

    secondsTicksHelper = 1000;
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
            // TODO: Iniciar sobrevuelo de la zona.
        }
	}
	else
    {
        ep_Players[guid]->SetByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP);
    }    
    eventCurrentStatus = ST_IN_PROGRESS;
    secureZoneIndex = 0;
    secureZoneDelay = 0;
    secureZoneAnnounced = false;
    secureZone = nullptr;
    secureZoneCenter = nullptr;
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
            ep_PlayersData[guid].SetPosition(ep_Players[guid]->GetMapId(), ep_Players[guid]->GetPositionX(), ep_Players[guid]->GetPositionY(), ep_Players[guid]->GetPositionZ(), ep_Players[guid]->GetOrientation());

			ep_Players[guid]->TeleportTo(BRMapID[TEMP_CURRENT_MAP_INDEX], BRZonesCenter[TEMP_CURRENT_MAP_INDEX].GetPositionX(), BRZonesCenter[TEMP_CURRENT_MAP_INDEX].GetPositionY(), BRZonesCenter[TEMP_CURRENT_MAP_INDEX].GetPositionZ(), 0.0f);
            EnterToPhaseEvent(guid);
            ep_Players[guid]->SaveToDB(false, false);
		}
        for (BattleRoyalePlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it) ep_PlayersQueue.erase((*it).first);
    }
	else
	{
        if (eventCurrentStatus != ST_SUMMON_PLAYERS) return;
        ep_Players[guid] = ep_PlayersQueue[guid];
        ep_PlayersData[guid].SetPosition(ep_Players[guid]->GetMapId(), ep_Players[guid]->GetPositionX(), ep_Players[guid]->GetPositionY(), ep_Players[guid]->GetPositionZ(), ep_Players[guid]->GetOrientation());
        ep_Players[guid]->TeleportTo(BRMapID[TEMP_CURRENT_MAP_INDEX], BRZonesCenter[TEMP_CURRENT_MAP_INDEX].GetPositionX(), BRZonesCenter[TEMP_CURRENT_MAP_INDEX].GetPositionY(), BRZonesCenter[TEMP_CURRENT_MAP_INDEX].GetPositionZ(), 0.0f);
        EnterToPhaseEvent(guid);
        ep_Players[guid]->SaveToDB(false, false);
        ep_PlayersQueue.erase(guid);
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
        secureZoneCenter->GetMap()->SetVisibilityRange(World::GetMaxVisibleDistanceOnContinents());
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
    // time_t t = time(NULL);
    // tm *now = localtime(&t);

	// if (now->tm_wday == 6 /*Saturday*/ && now->tm_hour >= 20/*8:00pm*/ && now->tm_hour <= 21/*9:00pm*/)
	// {
	// 	if (!hasAnnouncedEvent && hasEventEnded)
	// 	{
	// 		std::ostringstream msg2;
	// 		msg2 << "|cff4CFF00BattleRoyale::|r El Evento Parkour de la Muerte va a comenzar en la Arena Gurubachi en Vega de Tuercespina.";
	// 		sWorld->SendServerMessage(SERVER_MSG_STRING, msg2.str().c_str());
	// 		hasAnnouncedEvent = true;
	// 		inTimeToEvent = true;
    //         hasEventEnded = false;
	// 	}
	// }
	// else
	// {
	// 	if (!hasEventEnded)
	// 	{
	// 		std::ostringstream msg2;
	// 		msg2 << "|cff4CFF00BattleRoyale::|r El evento ha finalizado.";
	// 		sWorld->SendServerMessage(SERVER_MSG_STRING, msg2.str().c_str());
	// 		ExitFromEvent(0);
	// 		hasEventEnded = true;
	// 	}
    //     if (hasAnnouncedEvent)
    //     {
    //         hasTeleported = false;
    //         hasEventStarted = false;
    //         hasEventClose = false;
    //         nextReward = 1;
    //         inTimeToEvent = false;
    //         hasAnnouncedEvent = false;
    //     }
	// }

	// if (hasEventStarted)
    // {
    //     if (hackCheckDelay <= diff)
    //     {
    //         CheckForHacks(0);
    //         hackCheckDelay = 5000;
    //     }
    //     else hackCheckDelay -= diff;
    // }
    // if (!hasTeleported || hasEventStarted) return;
    // if (secondsDelay * 1000 > startDelay)
    // {
    //     SendNotification(0, secondsDelay);
    //     if (secondsDelay > 5) secondsDelay -= 5;
    //     else if (secondsDelay > 0) secondsDelay--;
    // }
    // if (startDelay <= diff) StartEvent(0);
    // else startDelay -= diff;
    switch(eventCurrentStatus)
    {
        case ST_SUMMON_PLAYERS:
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
                if (secureZone) {
                    secureZone->DespawnOrUnsummon();
                    secureZone->Delete();
                    secureZone = nullptr;
                }
                if (secureZoneIndex < 10) {
                    for (BattleRoyalePlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it)
                    {
                        if (secureZoneIndex == 0) {
                            if (secureZoneCenter) {
                                secureZoneCenter->DespawnOrUnsummon();
                                secureZoneCenter->Delete();
                                secureZoneCenter = nullptr;
                            }
                            secureZoneCenter = (*it).second->SummonGameObject(500010, BRZonesCenter[TEMP_CURRENT_MAP_INDEX].GetPositionX(), BRZonesCenter[TEMP_CURRENT_MAP_INDEX].GetPositionY(), BRZonesCenter[TEMP_CURRENT_MAP_INDEX].GetPositionZ(), 0, 0, 0, 0, 0, 15 * 60);
                        }
                        break;
                    }
                    secureZone = secureZoneCenter->SummonGameObject(500000 + secureZoneIndex, BRZonesCenter[TEMP_CURRENT_MAP_INDEX].GetPositionX(), BRZonesCenter[TEMP_CURRENT_MAP_INDEX].GetPositionY(), BRZonesCenter[TEMP_CURRENT_MAP_INDEX].GetPositionZ() + BRSecureZoneZPlus[secureZoneIndex], 0, 0, 0, 0, 0, 2 * 60);
                    secureZone->SetPhaseMask(2, true);
                    secureZone->GetMap()->SetVisibilityRange(500.0f);
                    secureZone->SetVisibilityDistanceOverride(VisibilityDistanceType::Infinite);
                }
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
                (*it).second->GetSession()->SendNotification("|cff00ff00¡Que comience la batalla de |cffDA70D6%s|cff00ff00!", BRZonesNames[TEMP_CURRENT_MAP_INDEX]);
            } else {
                (*it).second->GetSession()->SendNotification("|cff00ff00¡La batalla iniciará en |cffDA70D6%u|cff00ff00 segundos!", delay);
            }
        }
    } else ep_Players[guid]->GetSession()->SendNotification("|cff00ff00¡La batalla iniciará en |cffDA70D6%u|cff00ff00 segundos!", delay);
}

void BattleRoyaleMgr::OutOfZoneDamage()
{
    for (BattleRoyalePlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it)
    {
        float distance = (*it).second->GetExactDist(secureZoneCenter);
        ChatHandler((*it).second->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r Distancia del centro de la zona segura: %f.", distance); // temp
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
