/**
 *  Event: Parkour of Death
 */

#include "EventParkourMgr.h"
#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GameEventMgr.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Unit.h"
#include "GameObject.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "InstanceScript.h"
#include "CombatAI.h"
#include "PassiveAI.h"
#include "DBCStructure.h"
#include "DBCStores.h"
#include "ObjectMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"

// Not Fly!
class DismountParkourScript : public MovementHandlerScript
{
    public:
        DismountParkourScript() : MovementHandlerScript("DismountParkourScript") { }

        void OnPlayerMove(Player* player, MovementInfo /*movementInfo*/, uint32 /*opcode*/) override
        {
            sEventParkourMgr->HandleDismountFly(player);
        }
};

class ModEventParkourPlayer : public PlayerScript{
public:

    ModEventParkourPlayer() : PlayerScript("ModEventParkourPlayer") { }

    void OnLogin(Player* player) override {
        if (sConfigMgr->GetOption<bool>("EventParkour.Announce", true)) {
            ChatHandler(player->GetSession()).SendSysMessage("El modulo |cff4CFF00EventParkour|r ha sido activado.");
        }
    }

    void OnLogout(Player* player) override {
        sEventParkourMgr->HandlePlayerLogout(player);
    }

    void OnUpdateArea(Player *player, uint32 oldArea, uint32 newArea) override
	{
		sEventParkourMgr->HandleReleaseGhost(player, oldArea, newArea);
	}
};

class npc_eventparkour : public CreatureScript
{

public:

    npc_eventparkour() : CreatureScript("npc_eventparkour") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/Inv_misc_bone_skull_02:24:24:-18|tQuiero participar en el evento.", 0, 1);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/Inv_letter_09:24:24:-18|tSalir", 0, 2);
        SendGossipMenuFor(player, 190011, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();

        switch (action)
        {
        case 1:
            sEventParkourMgr->HandlePlayerJoin(player);
            break;
        case 2:
            break;
        }
        CloseGossipMenuFor(player);
        return true;
    }
};

class npc_eventparkour_tele : public CreatureScript
{

public:

    npc_eventparkour_tele() : CreatureScript("npc_eventparkour_tele") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (!(player->GetTeamId() == TEAM_HORDE)) AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/Inv_misc_bone_skull_02:24:24:-18|tTeletransporte a Ventormenta.", 0, 1);
        else AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/Inv_misc_bone_skull_02:24:24:-18|tTeletransporte a Orgrimmar.", 0, 2);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/Inv_misc_bone_skull_02:24:24:-18|tTeletransporte a Shattrath.", 0, 3);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/Inv_misc_bone_skull_02:24:24:-18|tTeletransporte a Dalaran.", 0, 4);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/Inv_letter_09:24:24:-18|tSalir", 0, 5);
        SendGossipMenuFor(player, 190012, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();

        switch (action)
        {
        case 1: // Teleport to Ventormenta
            player->SetPhaseMask(1, false);
            player->UpdateObjectVisibility();
            player->TeleportTo(0, -8833.379883f, 628.627991f, 94.006599f, 1.06535f);
            player->SaveToDB(false, false);
            break;
        case 2: // Teleport to Orgrimmar
            player->SetPhaseMask(1, false);
            player->UpdateObjectVisibility();
            player->TeleportTo(1, 1629.359985f, -4373.390137f, 31.482317f, 3.548390f);
            player->SaveToDB(false, false);
            break;
        case 3: // Teleport to Shattrath
            player->SetPhaseMask(1, false);
            player->UpdateObjectVisibility();
            player->TeleportTo(530, -1838.160034f, 5301.790039f, -12.428f, 5.9517f);
            player->SaveToDB(false, false);
            break;
        case 4: // Teleport to Dalaran
            player->SetPhaseMask(1, false);
            player->UpdateObjectVisibility();
            player->TeleportTo(571, 5804.149902f, 624.770996f, 647.767029f, 1.533971f);
            player->SaveToDB(false, false);
            break;
        case 5: // Close
            break;
        }
        CloseGossipMenuFor(player);
        return true;
    }
};

class npc_eventparkour_winner : public CreatureScript
{

public:

    npc_eventparkour_winner() : CreatureScript("npc_eventparkour_winner") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/Inv_misc_bone_skull_02:24:24:-18|tDame mi recompensa.", 0, 1);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/Inv_letter_09:24:24:-18|tSalir", 0, 2);
        SendGossipMenuFor(player, 190011, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();

        switch (action)
        {
        case 1:
            sEventParkourMgr->HandleGiveReward(player);
            break;
        case 2:
            break;
        }
        CloseGossipMenuFor(player);
        return true;
    }
};

class EventParkourWorldScript : public WorldScript
{
public:
	EventParkourWorldScript()
		: WorldScript("EventParkourWorldScript")
	{
	}
	void OnUpdate(uint32 diff) override
	{
		sEventParkourMgr->HandleOnWoldUpdate(diff);
	}
};

void AddModEventParkourScripts() {
    new DismountParkourScript();
    new ModEventParkourPlayer();
    new EventParkourWorldScript();
    new npc_eventparkour();
    new npc_eventparkour_tele();
    new npc_eventparkour_winner();
}
