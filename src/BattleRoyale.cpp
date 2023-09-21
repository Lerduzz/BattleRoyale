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
#include "Spell.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"

class BattleRoyalePlayer : public PlayerScript
{
public:

    BattleRoyalePlayer() : PlayerScript("BattleRoyalePlayer") { }

    void OnLogin(Player* player) override {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
        {
            if (sConfigMgr->GetOption<bool>("BattleRoyale.Announce", true))
            {
                ChatHandler(player->GetSession()).SendSysMessage("El modo |cff4CFF00BattleRoyale|r ha sido activado.");
            }
            sBattleRoyaleMgr->QuitarAlas(player);
            if (sBRListaNegraMgr->EstaBloqueado(player->GetGUID().GetCounter()) != "") sBRTitulosMgr->Quitar(player);
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
            if (sBattleRoyaleMgr->EstaEnCola(player) || sBattleRoyaleMgr->DebeRestringirFunciones(player))
            {
                ChatHandler(player->GetSession()).SendSysMessage("|cff4CFF00BattleRoyale::|r ¡No puedes hacer eso mientras participas en el modo Battle Royale!");
                return false;
            }
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
            if (sBattleRoyaleMgr->EstaEnCola(player) || sBattleRoyaleMgr->DebeRestringirFunciones(player))
            {
                ChatHandler(player->GetSession()).SendSysMessage("|cff4CFF00BattleRoyale::|r ¡No puedes hacer eso mientras participas en el modo Battle Royale!");
                return false;
            }
        }
        return true;
    }

    bool CanJoinInArenaQueue(Player* player, ObjectGuid /*BattlemasterGuid*/, uint8 /*arenaslot*/, BattlegroundTypeId /*BGTypeID*/, uint8 /*joinAsGroup*/, uint8 /*IsRated*/, GroupJoinBattlegroundResult& err) override
    {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
        {
            if (sBattleRoyaleMgr->EstaEnCola(player) || sBattleRoyaleMgr->DebeRestringirFunciones(player))
            {
                err = GroupJoinBattlegroundResult::ERR_BATTLEGROUND_NOT_IN_BATTLEGROUND;
                ChatHandler(player->GetSession()).SendSysMessage("|cff4CFF00BattleRoyale::|r ¡No puedes hacer eso mientras participas en el modo Battle Royale!");
                return false;
            }
        }
        return true;
    }

    bool CanJoinInBattlegroundQueue(Player* player, ObjectGuid /*BattlemasterGuid*/, BattlegroundTypeId /*BGTypeID*/, uint8 /*joinAsGroup*/, GroupJoinBattlegroundResult& err) override
    {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
        {
            if (sBattleRoyaleMgr->EstaEnCola(player) || sBattleRoyaleMgr->DebeRestringirFunciones(player))
            {
                err = GroupJoinBattlegroundResult::ERR_BATTLEGROUND_NOT_IN_BATTLEGROUND;
                ChatHandler(player->GetSession()).SendSysMessage("|cff4CFF00BattleRoyale::|r ¡No puedes hacer eso mientras participas en el modo Battle Royale!");
                return false;
            }
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

class BattleRoyaleCreature : public CreatureScript
{

public:

    BattleRoyaleCreature() : CreatureScript("BattleRoyaleCreature") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "¿Cuales son las recompensas?", 0, 1);
        if (!sBattleRoyaleMgr->EstaEnCola(player))
        {
            AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Quiero unirme a la cola.", 0, 2);
            if (player->IsGameMaster())
            {
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "<MJ> Quiero elegir el siguiente mapa.", 0, 3);
            }
        }
        else
        {
            AddGossipItemFor(player, GOSSIP_ICON_TALK, "Quiero salir de la cola.", 0, 4);
        }
        SendGossipMenuFor(player, 200000, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case 1:
            {
                CloseGossipMenuFor(player);
                break;
            }
            case 2:
            {
                if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
                {
                    sBattleRoyaleMgr->GestionarJugadorEntrando(player);
                }
                else
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡Este modo de juego se encuentra actualmente desactivado!");
                }
                CloseGossipMenuFor(player);
                break;
            }   
            case 3:
            {
                if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
                {           
                    if (player->IsGameMaster())
                    {
                        uint32 start = 5;
                        BR_ContenedorMapas mapas = sBattleRoyaleMgr->ObtenerMapas();
                        for (BR_ContenedorMapas::iterator it = mapas.begin(); it != mapas.end(); ++it)
                        {
                            AddGossipItemFor(player, GOSSIP_ICON_BATTLE, it->second->nombreMapa, it->first, start++);
                        }
                        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                    }
                    else
                    {
                        CloseGossipMenuFor(player);
                    }   
                }
                else
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡Este modo de juego se encuentra actualmente desactivado!");
                    CloseGossipMenuFor(player);
                }
                break;
            }
            case 4:
            {
                if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
                {
                    if (sBattleRoyaleMgr->EstaEnCola(player))
                    {
                        sBattleRoyaleMgr->GestionarJugadorDesconectar(player);
                        ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r Ya no estas en cola para el evento.");
                    }
                    else
                    {
                        ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡No estas en cola para el evento!");
                    }
                }
                else
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡Este modo de juego se encuentra actualmente desactivado!");
                }
                CloseGossipMenuFor(player);
                break;
            }            
            default:
            {
                if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
                {
                    if (player->IsGameMaster())
                    {
                        sBattleRoyaleMgr->EstablecerMapa(sender);
                    }
                    sBattleRoyaleMgr->GestionarJugadorEntrando(player);
                }
                else
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡Este modo de juego se encuentra actualmente desactivado!");
                }
                CloseGossipMenuFor(player);
                break;
            }
        }
        return true;
    }
};

class BattleRoyaleWorld : public WorldScript
{
public:
	BattleRoyaleWorld()
		: WorldScript("BattleRoyaleWorld")
	{
	}
	void OnUpdate(uint32 diff) override
	{
		sBattleRoyaleMgr->GestionarActualizacionMundo(diff);
	}
};

class BattleRoyaleItem : public ItemScript
{
public:
    BattleRoyaleItem() : ItemScript("BattleRoyaleItem") { }

    bool OnUse(Player* player, Item* /*item*/, const SpellCastTargets &) override
    {
        if (!sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
        {
            ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡Este modo de juego se encuentra actualmente desactivado!");
            return false;
        }
        if (!sBattleRoyaleMgr->EstaEnEvento(player))
        {
            ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡Solo se puede utilizar mientras participas en este modo de juego!");
            return false;
        }
        if (sBRObjetosMgr->EstaEnLaNave(player))
        {
            ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡No se puede utilizar hasta que saltas de la nave!");
            return false;
        }
        if (player->HasAura(HECHIZO_ALAS_MAGICAS))
        {
            player->RemoveAurasDueToSpell(HECHIZO_ALAS_MAGICAS);
            return true;
        }
        else
        {
            player->AddAura(HECHIZO_ALAS_MAGICAS, player);
            return false;
        }
    }
};

class BattleRoyaleCommand : public CommandScript
{
public:
    BattleRoyaleCommand() : CommandScript("BattleRoyaleCommand") {}

    Acore::ChatCommands::ChatCommandTable GetCommands() const override
    {
        static Acore::ChatCommands::ChatCommandTable commandTable = {
            {"", HandleBRCommand, SEC_PLAYER, Acore::ChatCommands::Console::No}, 
            {"recargar", HandleReloadCommand, 5, Acore::ChatCommands::Console::Yes}
        };

        static Acore::ChatCommands::ChatCommandTable baseTable = {
            {"br", commandTable}
        };

        return baseTable;
    }

    static bool HandleBRCommand(ChatHandler *handler)
    {
        if (sConfigMgr->GetOption<bool>("BattleRoyale.Enabled", true))
        {
            handler->SendSysMessage("El modo |cff4CFF00BattleRoyale|r se encuentra |cff00ff00activado|r.");
        }
        else
        {
            handler->SendSysMessage("El modo |cff4CFF00BattleRoyale|r se encuentra |cffff0000desactivado|r.");
        }
        return true;
    }

    static bool HandleReloadCommand(ChatHandler *handler)
    {
        sBRListaNegraMgr->RecargarLista();
        handler->SendSysMessage("Se ha recargado la lista negra del modo Battle Royale.");
        return true;
    }
};

class BattleRoyaleSpell : public SpellSC
{
public:

    BattleRoyaleSpell() : SpellSC("BattleRoyaleSpell") {}

    void OnSpellCheckCast(Spell* spell, bool /*strict*/, SpellCastResult& res) override
    {
        if (spell)
        {
            Unit* uCaster = spell->GetCaster();
            Player* pCaster = uCaster && uCaster->GetTypeId() == TYPEID_PLAYER ? uCaster->ToPlayer() : nullptr;
            if (pCaster && sBattleRoyaleMgr->EstaEnEvento(pCaster))
            {
                if (const SpellInfo* spInf = spell->GetSpellInfo())
                {
                    if (spInf->HasAura(SPELL_AURA_FLY) || spInf->HasAura(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED)) res = SPELL_FAILED_NOT_HERE;
                }
            }
        }
    }
};

void AddBattleRoyaleScripts() {
    new BattleRoyalePlayer();
    new BattleRoyaleCreature();
    new BattleRoyaleWorld();
    new BattleRoyaleItem();
    new BattleRoyaleCommand();
    new BattleRoyaleSpell();
}
