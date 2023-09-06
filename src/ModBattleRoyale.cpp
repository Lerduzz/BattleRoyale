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
            sBattleRoyaleMgr->GestionarJugadorDesconectar(player);
        }
    }

    void OnPVPKill(Player* killer, Player* killed) override
    {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
        {
            sBattleRoyaleMgr->GestionarMuerteJcJ(killer, killed);
        }
    }

    bool CanRepopAtGraveyard(Player *player) override
	{
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
        {
            return sBattleRoyaleMgr->PuedeReaparecerEnCementerio(player);
        }
        return true;
	}

    void OnPlayerPVPFlagChange(Player* player, bool state) override
    {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
        {
            sBattleRoyaleMgr->PrevenirJcJEnLaNave(player, state);
        }
    }

    bool CanJoinLfg(Player* player, uint8 /*roles*/, lfg::LfgDungeonSet& /*dungeons*/, const std::string& /*comment*/) override
    {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
        {
            return !sBattleRoyaleMgr->DebeRestringirFunciones(player);
        }
        return true;
    }

    bool CanGroupInvite(Player* player, std::string& /*membername*/) override
    {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
        {
            return !sBattleRoyaleMgr->DebeRestringirFunciones(player);
        }
        return true;
    }

    bool CanGroupAccept(Player* player, Group* /*group*/) override
    {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
        {
            return !sBattleRoyaleMgr->DebeRestringirFunciones(player);
        }
        return true;
    }

    bool CanBattleFieldPort(Player* player, uint8 /*arenaType*/, BattlegroundTypeId /*BGTypeID*/, uint8 /*action*/) override
    {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
        {
            return !sBattleRoyaleMgr->DebeRestringirFunciones(player);
        }
        return true;
    }

    bool CanJoinInArenaQueue(Player* player, ObjectGuid /*BattlemasterGuid*/, uint8 /*arenaslot*/, BattlegroundTypeId /*BGTypeID*/, uint8 /*joinAsGroup*/, uint8 /*IsRated*/, GroupJoinBattlegroundResult& err) override
    {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
        {
            err = GroupJoinBattlegroundResult::ERR_BATTLEGROUND_NOT_IN_BATTLEGROUND;
            return !sBattleRoyaleMgr->DebeRestringirFunciones(player);
        }
        return true;
    }

    bool CanJoinInBattlegroundQueue(Player* player, ObjectGuid /*BattlemasterGuid*/, BattlegroundTypeId /*BGTypeID*/, uint8 /*joinAsGroup*/, GroupJoinBattlegroundResult& err) override
    {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
        {
            err = GroupJoinBattlegroundResult::ERR_BATTLEGROUND_NOT_IN_BATTLEGROUND;
            return !sBattleRoyaleMgr->DebeRestringirFunciones(player);
        }
        return true;
    }

    bool BattleRoyaleForzarJcJTcTScript(Player* player) override {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
        {
            return sBattleRoyaleMgr->DebeForzarJcJTcT(player);
        }
        return false;
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
            sBattleRoyaleMgr->GestionarJugadorEntrando(player);
            break;
        case 2:
            sBattleRoyaleMgr->GestionarJugadorDesconectar(player);
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
		sBattleRoyaleMgr->GestionarActualizacionMundo(diff);
	}
};

void AddModBattleRoyaleScripts() {
    new ModBattleRoyalePlayer();
    new BattleRoyaleWorldScript();
    new npc_battleroyale();
}
