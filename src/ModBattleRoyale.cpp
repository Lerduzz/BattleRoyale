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
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
        {
            sBattleRoyaleMgr->HandlePlayerLogout(player);
        }
    }

    bool CanRepopAtGraveyard(Player *player) override
	{
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
        {
            return !sBattleRoyaleMgr->HandleReleaseGhost(player);
        }
	}

    bool BRForcePlayerFFAPvPFlag(Player* player) override {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
        {
            return sBattleRoyaleMgr->ForceFFAPvPFlag(player);
        }
        return false;
    }

    void OnPlayerPVPFlagChange(Player* player, bool state) override
    {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
        {
            sBattleRoyaleMgr->PreventPvPBeforeBattle(player, state);
        }
    }

    bool CanJoinLfg(Player* player, uint8 /*roles*/, lfg::LfgDungeonSet& /*dungeons*/, const std::string& /*comment*/) override
    {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
        {
            return !sBattleRoyaleMgr->RestrictPlayerFunctions(player);
        }
        return true;
    }

    bool CanGroupInvite(Player* player, std::string& /*membername*/) override
    {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
        {
            return !sBattleRoyaleMgr->RestrictPlayerFunctions(player);
        }
        return true;
    }

    bool CanGroupAccept(Player* player, Group* /*group*/) override
    {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
        {
            return !sBattleRoyaleMgr->RestrictPlayerFunctions(player);
        }
        return true;
    }

    bool CanBattleFieldPort(Player* player, uint8 /*arenaType*/, BattlegroundTypeId /*BGTypeID*/, uint8 /*action*/) override
    {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
        {
            return !sBattleRoyaleMgr->RestrictPlayerFunctions(player);
        }
        return true;
    }

    bool CanJoinInArenaQueue(Player* player, ObjectGuid /*BattlemasterGuid*/, uint8 /*arenaslot*/, BattlegroundTypeId /*BGTypeID*/, uint8 /*joinAsGroup*/, uint8 /*IsRated*/, GroupJoinBattlegroundResult& err) override
    {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
        {
            err = GroupJoinBattlegroundResult::ERR_BATTLEGROUND_NOT_IN_BATTLEGROUND;
            return !sBattleRoyaleMgr->RestrictPlayerFunctions(player);
        }
        return true;
    }

    bool CanJoinInBattlegroundQueue(Player* player, ObjectGuid /*BattlemasterGuid*/, BattlegroundTypeId /*BGTypeID*/, uint8 /*joinAsGroup*/, GroupJoinBattlegroundResult& err) override
    {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
        {
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
