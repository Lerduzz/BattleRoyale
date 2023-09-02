#include "BattleRoyaleMgr.h"
#include "Config.h"
#include "Chat.h"
#include "Player.h"

// -- CONSTANTES -- //
const int BRMapCount = 1;

const int BRMapID[BRMapCount] = { 1 };

const Position BRZonesCenter[BRMapCount] =
{
    { 5520.340820f, -3700.600342f, 1594.888916f }      // 1: Kalimdor: Hyjal
};

const std:string BRZonesNames[BRMapCount] =
{
    "Kalimdor: Hyjal"
};

const float BRSecureZoneZPlus[10] = { 140.0f, 130.0f, 120.0f, 110.0f, 100.0f, 90.0f, 75.0f, 60.0f, 45.0f, 25.0f };

// -- FUNCIONES -- //
BattleRoyaleMgr::BattleRoyaleMgr()
{
    hasTeleported = false;
    hasEventStarted = false;
    hasEventClose = false;
    hasEventEnded = true;
    nextReward = 1;
	inTimeToEvent = false;
	hasAnnouncedEvent = false;

    // NEW!
    rotationMapIndex = 0;
}

BattleRoyaleMgr::~BattleRoyaleMgr()
{
	ep_Players.clear();
    ep_PlayersData.clear();
}

// Not Fly!
void BattleRoyaleMgr::HandleDismountFly(Player *player)
{
    // if (!inTimeToEvent || hasEventClose)
    //     return;
    // if (ep_Players.find(player->GetGUID().GetCounter()) == ep_Players.end())
    //     return;
    // if (!player->HasAura(31700))
    //     return;
    // player->Dismount();
    // player->RemoveAurasByType(SPELL_AURA_MOUNTED);
    // player->RemoveAurasDueToSpell(31700);
}

void BattleRoyaleMgr::HandlePlayerJoin(Player *player)
{
    uint32 guid = player->GetGUID().GetCounter();
    // if (!inTimeToEvent)
    // {
    //     ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r En este momento el evento no se esta efectuando, regresa el sabado entre las 8:00pm y las 10:00pm.");
    //     return;
    // }
    if (ep_Players.find(guid) != ep_Players.end())
    {
        ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r Ya estas en cola para el evento.");
        return;
    }
    // if (hasEventEnded)
    // {
    //     ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r El evento ha finalizado.");
    //     return;
    // }
    // if (hasEventClose)
    // {
    //     ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r El evento ya esta cerrado.");
    //     return;
    // }
    // uint32 count;
    // uint32 maxp = sConfigMgr->GetOption<int32>("BattleRoyale.MaxPlayers", 50);
    // count = ep_Players.size();
    // if (count >= maxp)
    // {
    //     ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r El evento esta lleno, intenta mas tarde.");
    //     return;
    // }

	ep_Players[guid] = player;
    ep_PlayersData[guid].SetPosition(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation());

    // count = ep_Players.size();
    // uint32 minp = sConfigMgr->GetOption<int32>("BattleRoyale.MinPlayersToStart", 25);
    // if(count >= minp && !hasTeleported)
    // {
    //     if (sConfigMgr->GetOption<bool>("BattleRoyale.Announce.Start", true))
    //     {
    //         std::ostringstream msg;
    //         msg << "|cff4CFF00BattleRoyale::|r " << player->GetName().c_str() << " ha completado la cola, teletransportando a los jugadores al evento. Los demas jugadores seran teletransportado a penas se unan a la cola.";
    //         sWorld->SendServerMessage(SERVER_MSG_STRING, msg.str().c_str());
    //     }
    //     TeleportToEvent(0);
    // }
    // else if (!hasTeleported)
    // {
    //     if (sConfigMgr->GetOption<bool>("BattleRoyale.Announce.Queue", true))
    //     {
    //         std::ostringstream msg;
    //         msg << "|cff4CFF00BattleRoyale::|r " << player->GetName().c_str() << " se ha unido a la cola, falta " << (minp - count) << " mas para comenzar.";
    //         sWorld->SendServerMessage(SERVER_MSG_STRING, msg.str().c_str());
    //     }
    // }
    // else
    // {
    //     if (sConfigMgr->GetOption<bool>("BattleRoyale.Announce.Join", true))
    //     {
    //         std::ostringstream msg;
    //         msg << "|cff4CFF00BattleRoyale::|r " << player->GetName().c_str() << " se ha unido al evento.";
    //         sWorld->SendServerMessage(SERVER_MSG_STRING, msg.str().c_str());
    //     }
    //     TeleportToEvent(player->GetGUID().GetCounter());
    // }
    StartEvent(guid);
}

void BattleRoyaleMgr::HandlePlayerLogout(Player *player)
{
    if (ep_Players.find(player->GetGUID().GetCounter()) == ep_Players.end()) return;
    uint32 guid = player->GetGUID().GetCounter();
    ExitFromPhaseEvent(guid);
    ep_Players.erase(guid);
    ep_PlayersData.erase(guid);
    // if (sConfigMgr->GetOption<bool>("BattleRoyale.Announce.Logout", false))
    // {
    //     std::ostringstream msg;
    //     msg << "|cff4CFF00BattleRoyale::|r " << player->GetName().c_str() << " se ha desconectado y ha dejado el evento.";
    //     sWorld->SendServerMessage(SERVER_MSG_STRING, msg.str().c_str());
    // }
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
	// if (!guid)
	// {
	// 	for (ParkourPlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it)
    //     {
    //         EnterToPhaseEvent((*it).first);
    //         ep_PlayersData[(*it).first].SetLast((*it).second->GetPositionZ());
    //     }
    // hasEventStarted = true;
    // hackCheckDelay = 5000;
	// }
	// else
    // {
    //     EnterToPhaseEvent(guid);
    //     ep_PlayersData[guid].SetLast(ep_Players[guid]->GetPositionZ());
    // }
    EnterToPhaseEvent(guid);
    ep_Players[guid]->TeleportTo(1, BRZonesCenter[0].GetPositionX(), BRZonesCenter[0].GetPositionY(), BRZonesCenter[0].GetPositionZ(), 0.0f);
    ep_Players[guid]->SetByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP);
    hasEventStarted = true;
    secureZoneIndex = 0;
    secureZoneDelay = 0;
    secureZoneAnnounced = false;
    secureZone = nullptr;
}

void BattleRoyaleMgr::TeleportToEvent(uint32 guid)
{
	// if (!guid)
	// {
    //     startDelay = sConfigMgr->GetOption<int32>("BattleRoyale.EventStartDelay", 60000);
	// 	secondsDelay = startDelay / 1000;
    //     for (ParkourPlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it)
	// 	{
	// 		(*it).second->TeleportTo(0, -13246.281f, 193.465f, 31.019f, 1.130f);
    //         EnterToPhaseDelay((*it).first);
    //         (*it).second->SaveToDB(false, false);
	// 	}
    //     hasTeleported = true;
	// }
	// else
	// {
    //     ep_Players[guid]->TeleportTo(0, -13246.281f, 193.465f, 31.019f, 1.130f);
    //     if (hasEventStarted) EnterToPhaseEvent(guid);
    //     else EnterToPhaseDelay(guid);
    //     ep_Players[guid]->SaveToDB(false, false);
	// }
}

void BattleRoyaleMgr::ExitFromEvent(uint32 guid)
{
	// if (!guid)
	// {
	// 	for (ParkourPlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it)
	// 	{
    //         if (!(*it).second->IsAlive()) ResurrectPlayer((*it).second);
    //         ExitFromPhaseEvent((*it).first);
    //         (*it).second->TeleportTo(571, 5804.149902f, 624.770996f, 647.767029f, 1.533971f);
    //         (*it).second->SaveToDB(false, false);
    //         ep_Players.erase((*it).first);
	// 	    ep_PlayersData.erase((*it).first);
	// 	}
	// }
	// else
	// {
    //     if (!ep_Players[guid]->IsAlive()) ResurrectPlayer(ep_Players[guid]);
    //     ExitFromPhaseEvent(guid);
    //     ep_Players[guid]->TeleportTo(571, 5804.149902f, 624.770996f, 647.767029f, 1.533971f);
    //     ep_Players[guid]->SaveToDB(false, false);
    //     ep_Players.erase(guid);
	//     ep_PlayersData.erase(guid);
	// }
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
    if (hasEventStarted) {
        if (secureZoneDelay <= 0) {
            if (secureZone) {
                secureZone->DespawnOrUnsummon();
                secureZone->Delete();
                secureZone = nullptr;
            }
            if (secureZoneIndex < 10) {
                for (ParkourPlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it)
                {
                    secureZone = (*it).second->SummonGameObject(500000 + secureZoneIndex, BRZonesCenter[0].GetPositionX(), BRZonesCenter[0].GetPositionY(), BRZonesCenter[0].GetPositionZ() + BRSecureZoneZPlus[secureZoneIndex], 0, 0, 0, 0, 0, 120);
                    secureZone->SetPhaseMask(2, true);
                    break;
                }
            }
            secureZoneIndex++;
            secureZoneDelay = 7000;
            secureZoneAnnounced = false;
        } else {
            if (secureZoneDelay <= 5000 && !secureZoneAnnounced) {
                SendNotification(0, 5);
                secureZoneAnnounced = true;
            }
            if (secureZoneIndex <= 10) {
                secureZoneDelay -= diff;
            }
        }
    }
}

bool BattleRoyaleMgr::ForceFFAPvPFlag(Player* player)
{
    if (ep_Players.find(player->GetGUID().GetCounter()) == ep_Players.end()) return false;
    return true;
}

// -- Private functions -- //
void BattleRoyaleMgr::EnterToPhaseDelay(uint32 guid)
{
	ep_Players[guid]->SetPhaseMask(4, false);
    ep_Players[guid]->UpdateObjectVisibility();
}

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

void BattleRoyaleMgr::CheckForHacks(uint32 guid)
{
    if (!guid)
        for (ParkourPlayerData::iterator it = ep_PlayersData.begin(); it != ep_PlayersData.end(); ++it)
		{
            if ((*it).second.IsHackingZ(ep_Players[(*it).first]->GetPositionZ()))
            {
                ep_Players[(*it).first]->TeleportTo(0, -13246.281f, 193.465f, 31.019f, 1.130f);
                ep_Players[(*it).first]->SaveToDB(false, false);
                (*it).second.SetLast(ep_Players[(*it).first]->GetPositionZ());
                ChatHandler(ep_Players[(*it).first]->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r Has sido transportado al inicio por movimiento sospechoso (posible hack).");
            }
        }
    else
    {
        if (ep_PlayersData[guid].IsHackingZ(ep_Players[guid]->GetPositionZ()))
        {
            ep_Players[guid]->TeleportTo(0, -13246.281f, 193.465f, 31.019f, 1.130f);
            ep_Players[guid]->SaveToDB(false, false);
            ep_PlayersData[guid].SetLast(ep_Players[guid]->GetPositionZ());
            ChatHandler(ep_Players[guid]->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r Has sido transportado al inicio por movimiento sospechoso (posible hack).");
        }
    }
}

void BattleRoyaleMgr::SendNotification(uint32 guid, uint32 delay)
{
    // if (!guid)
    //     for (ParkourPlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it)
	// 		(*it).second->GetSession()->SendNotification("|cff00ff00FALTA(N) |cffDA70D6%u|cff00ff00 SEGUNDO(S) PARA COMENZAR!", delay);
    // else ep_Players[guid]->GetSession()->SendNotification("|cff00ff00FALTA(N) |cffDA70D6%u|cff00ff00 SEGUNDO(S) PARA COMENZAR!", delay);
    if (!guid)
        for (ParkourPlayerList::iterator it = ep_Players.begin(); it != ep_Players.end(); ++it)
			(*it).second->GetSession()->SendNotification("|cff00ff00¡La zona segura se reducirá en |cffDA70D6%u|cff00ff00 segundos!", delay);
    else ep_Players[guid]->GetSession()->SendNotification("|cff00ff00¡La zona segura se reducirá en |cffDA70D6%u|cff00ff00 segundos!", delay);
}
