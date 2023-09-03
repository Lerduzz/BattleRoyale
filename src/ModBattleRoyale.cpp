/**
 *  Event: Parkour of Death
 */

#include "BattleRoyaleMgr.h"
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
// class DismountParkourScript : public MovementHandlerScript
// {
//     public:
//         DismountParkourScript() : MovementHandlerScript("DismountParkourScript") { }
// 
//         void OnPlayerMove(Player* player, MovementInfo /*movementInfo*/, uint32 /*opcode*/) override
//         {
//             sBattleRoyaleMgr->HandleDismountFly(player);
//         }
// };

class ModBattleRoyalePlayer : public PlayerScript{
public:

    ModBattleRoyalePlayer() : PlayerScript("ModBattleRoyalePlayer") { }

    void OnLogin(Player* player) override {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true)) {
            if (sConfigMgr->GetOption<bool>("BattleRoyale.Announce", true)) {
                ChatHandler(player->GetSession()).SendSysMessage("El modulo |cff4CFF00BattleRoyale|r ha sido activado.");
            }
        }
    }
    
    void OnLogout(Player* player) override {
        sBattleRoyaleMgr->HandlePlayerLogout(player);
    }

    void OnUpdateArea(Player *player, uint32 oldArea, uint32 newArea) override
	{
		// sBattleRoyaleMgr->HandleReleaseGhost(player, oldArea, newArea);
	}

    bool BRForcePlayerFFAPvPFlag(Player* player) override {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true)) {
            return sBattleRoyaleMgr->ForceFFAPvPFlag(player);
        }
        return false;
    }

    // -- RESTRICCIONES DE PERSONAJE -- //
    bool CanJoinLfg(Player* player, uint8 /*roles*/, lfg::LfgDungeonSet& /*dungeons*/, const std::string& /*comment*/) override
    {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true)) {
            return !sBattleRoyaleMgr->RestrictPlayerFunctions(player);
        }
        return true;
    }

    bool CanGroupInvite(Player* player, std::string& /*membername*/) override
    {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true)) {
            return !sBattleRoyaleMgr->RestrictPlayerFunctions(player);
        }
        return true;
    }

    bool CanGroupAccept(Player* player, Group* /*group*/) override
    {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true)) {
            return !sBattleRoyaleMgr->RestrictPlayerFunctions(player);
        }
        return true;
    }

    bool CanBattleFieldPort(Player* player, uint8 /*arenaType*/, BattlegroundTypeId /*BGTypeID*/, uint8 /*action*/) override
    {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true)) {
            return !sBattleRoyaleMgr->RestrictPlayerFunctions(player);
        }
        return true;
    }

    bool CanJoinInArenaQueue(Player* player, ObjectGuid /*BattlemasterGuid*/, uint8 /*arenaslot*/, BattlegroundTypeId /*BGTypeID*/, uint8 /*joinAsGroup*/, uint8 /*IsRated*/, GroupJoinBattlegroundResult& err) override
    {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true)) {
            err = GroupJoinBattlegroundResult::ERR_BATTLEGROUND_NOT_IN_BATTLEGROUND;
            return !sBattleRoyaleMgr->RestrictPlayerFunctions(player);
        }
        return true;
    }

    bool CanJoinInBattlegroundQueue(Player* player, ObjectGuid /*BattlemasterGuid*/, BattlegroundTypeId /*BGTypeID*/, uint8 /*joinAsGroup*/, GroupJoinBattlegroundResult& err) override
    {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true)) {
            err = GroupJoinBattlegroundResult::ERR_BATTLEGROUND_NOT_IN_BATTLEGROUND;
            return !sBattleRoyaleMgr->RestrictPlayerFunctions(player);
        }
        return true;
    }
};

class npc_battleroyale : public CreatureScript
{

public:

    npc_battleroyale() : CreatureScript("npc_battleroyale") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/Inv_misc_bone_skull_02:24:24:-18|tQuiero participar en el evento.", 0, 1);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/Inv_misc_bone_skull_02:24:24:-18|tQuiero salir de la cola del evento.", 0, 2);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/Inv_letter_09:24:24:-18|tSalir", 0, 3);
        SendGossipMenuFor(player, 200000, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
        case 1:
            sBattleRoyaleMgr->HandlePlayerJoin(player);
            break;
        case 2:
            sBattleRoyaleMgr->HandlePlayerLogout(player);
            ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r Ya no estas en cola para el evento.");
            break;
        case 3:
            break;
        }
        CloseGossipMenuFor(player);
        return true;
    }
};

// class npc_battleroyale_tele : public CreatureScript
// {
// 
// public:
// 
//     npc_battleroyale_tele() : CreatureScript("npc_battleroyale_tele") { }
// 
//     bool OnGossipHello(Player* player, Creature* creature)
//     {
//         if (!(player->GetTeamId() == TEAM_HORDE)) AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/Inv_misc_bone_skull_02:24:24:-18|tTeletransporte a Ventormenta.", 0, 1);
//         else AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/Inv_misc_bone_skull_02:24:24:-18|tTeletransporte a Orgrimmar.", 0, 2);
//         AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/Inv_misc_bone_skull_02:24:24:-18|tTeletransporte a Shattrath.", 0, 3);
//         AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/Inv_misc_bone_skull_02:24:24:-18|tTeletransporte a Dalaran.", 0, 4);
//         AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/Inv_letter_09:24:24:-18|tSalir", 0, 5);
//         SendGossipMenuFor(player, 190012, creature->GetGUID());
//         return true;
//     }
// 
//     bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action)
//     {
//         player->PlayerTalkClass->ClearMenus();
// 
//         switch (action)
//         {
//         case 1: // Teleport to Ventormenta
//             player->SetPhaseMask(1, false);
//             player->UpdateObjectVisibility();
//             player->TeleportTo(0, -8833.379883f, 628.627991f, 94.006599f, 1.06535f);
//             player->SaveToDB(false, false);
//             break;
//         case 2: // Teleport to Orgrimmar
//             player->SetPhaseMask(1, false);
//             player->UpdateObjectVisibility();
//             player->TeleportTo(1, 1629.359985f, -4373.390137f, 31.482317f, 3.548390f);
//             player->SaveToDB(false, false);
//             break;
//         case 3: // Teleport to Shattrath
//             player->SetPhaseMask(1, false);
//             player->UpdateObjectVisibility();
//             player->TeleportTo(530, -1838.160034f, 5301.790039f, -12.428f, 5.9517f);
//             player->SaveToDB(false, false);
//             break;
//         case 4: // Teleport to Dalaran
//             player->SetPhaseMask(1, false);
//             player->UpdateObjectVisibility();
//             player->TeleportTo(571, 5804.149902f, 624.770996f, 647.767029f, 1.533971f);
//             player->SaveToDB(false, false);
//             break;
//         case 5: // Close
//             break;
//         }
//         CloseGossipMenuFor(player);
//         return true;
//     }
// };
// 
// class npc_battleroyale_winner : public CreatureScript
// {
// 
// public:
// 
//     npc_battleroyale_winner() : CreatureScript("npc_battleroyale_winner") { }
// 
//     bool OnGossipHello(Player* player, Creature* creature)
//     {
//         AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/Inv_misc_bone_skull_02:24:24:-18|tDame mi recompensa.", 0, 1);
//         AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/Inv_letter_09:24:24:-18|tSalir", 0, 2);
//         SendGossipMenuFor(player, 190011, creature->GetGUID());
//         return true;
//     }
// 
//     bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action)
//     {
//         player->PlayerTalkClass->ClearMenus();
// 
//         switch (action)
//         {
//         case 1:
//             sBattleRoyaleMgr->HandleGiveReward(player);
//             break;
//         case 2:
//             break;
//         }
//         CloseGossipMenuFor(player);
//         return true;
//     }
// };

class BattleRoyaleWorldScript : public WorldScript
{
public:
	BattleRoyaleWorldScript()
		: WorldScript("BattleRoyaleWorldScript")
	{
	}
	void OnUpdate(uint32 diff) override
	{
		sBattleRoyaleMgr->HandleOnWoldUpdate(diff);
	}
};

class cs_battleroyale : public CommandScript
{
public:
    cs_battleroyale() : CommandScript("cs_battleroyale") {}

    Acore::ChatCommands::ChatCommandTable GetCommands() const override
    {
        static Acore::ChatCommands::ChatCommandTable BattleRoyaleCommandTable = {
            {"", HandleBattleYoyaleCommand, SEC_PLAYER, Acore::ChatCommands::Console::No}, 
            {"point", HandleBRPointCommand, SEC_PLAYER, Acore::ChatCommands::Console::No},
            {"distance", HandleBRDistanceCommand, SEC_PLAYER, Acore::ChatCommands::Console::No},
            {"test", HandleBRTestCommand, SEC_PLAYER, Acore::ChatCommands::Console::No}
        };

        static Acore::ChatCommands::ChatCommandTable BattleRoyaleBaseTable = {
            {"battleroyale", BattleRoyaleCommandTable}
        };

        return BattleRoyaleBaseTable;
    }

    static bool HandleBattleYoyaleCommand(ChatHandler *handler)
    {
        // handler->SendSysMessage("|cff4CFF00BRCommands::|r point: set point, distance: calculate distance.");
        return true;
    }

    static bool HandleBRPointCommand(ChatHandler *handler)
    {
        // sBattleRoyaleMgr->CreateReferencePoint(handler->GetSession()->GetPlayer());
        // handler->SendSysMessage("|cff4CFF00BRCommands::|r Punto de medicion establecido.");
        return true;
    }

    static bool HandleBRDistanceCommand(ChatHandler *handler)
    {
        // handler->PSendSysMessage("|cff4CFF00BRCommands::|r La distancia es: %f.", sBattleRoyaleMgr->GetDistanceFromPoint(handler->GetSession()->GetPlayer()));
        return true;
    }

    static bool HandleBRTestCommand(ChatHandler *handler)
    {
        // sBattleRoyaleMgr->CrearNave(handler->GetSession()->GetPlayer());
        // handler->SendSysMessage("|cff4CFF00BRCommands::|r Probando la nave.");
        return true;
    }
};

void AddModBattleRoyaleScripts() {
    new ModBattleRoyalePlayer();
    new BattleRoyaleWorldScript();
    new npc_battleroyale();
    new cs_battleroyale();
}
