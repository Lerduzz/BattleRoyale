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
    BattleRoyalePlayer() : PlayerScript("BattleRoyalePlayer") {}

    void OnLogin(Player *player) override
    {
        if (sBattleRoyaleMgr->EstaActivado())
        {
            if (sConfigMgr->GetOption<bool>("BattleRoyale.Anunciar", true))
            {
                ChatHandler(player->GetSession()).SendSysMessage("El modo |cff4CFF00BattleRoyale|r ha sido activado.");
            }
        }
        sBREquipamientoMgr->QuitarTodosLosObjetos(player);
    }

    void OnLogout(Player *player) override
    {
        if (sBattleRoyaleMgr->EstaActivado())
        {
            sBattleRoyaleMgr->GestionarJugadorDesconectar(player);
        }
    }

    void OnPVPKill(Player *killer, Player *killed) override
    {
        if (sBattleRoyaleMgr->EstaActivado())
        {
            sBattleRoyaleMgr->GestionarMuerteJcJ(killer, killed);
        }
    }

    bool CanEquipItem(Player *player, uint8 /*slot*/, uint16 & /*dest*/, Item *pItem, bool /*swap*/, bool not_loading) override
    {
        if (player && pItem && not_loading)
        {
            if (sBattleRoyaleMgr->EstadoActual() > ESTADO_BR_SIN_SUFICIENTES_JUGADORES && sBattleRoyaleMgr->EstaEnEvento(player))
            {
                return sBREquipamientoMgr->EsEquipamientoDeBR(pItem->GetEntry());
            }
            else
            {
                return !sBREquipamientoMgr->EsEquipamientoDeBR(pItem->GetEntry());
            }
        }
        return true;
    }

    bool CanRepopAtGraveyard(Player *player) override
    {
        if (sBattleRoyaleMgr->EstaActivado())
        {
            return sBattleRoyaleMgr->PuedeReaparecerEnCementerio(player);
        }
        return true;
    }

    void OnPlayerPVPFlagChange(Player *player, bool state) override
    {
        if (sBattleRoyaleMgr->EstaActivado())
        {
            sBattleRoyaleMgr->PrevenirJcJEnLaNave(player, state);
        }
    }

    bool CanJoinLfg(Player *player, uint8 /*roles*/, lfg::LfgDungeonSet & /*dungeons*/, const std::string & /*comment*/) override
    {
        if (sBattleRoyaleMgr->EstaActivado())
        {
            if (sBattleRoyaleMgr->EstaEnCola(player) || sBattleRoyaleMgr->DebeRestringirFunciones(player))
            {
                ChatHandler(player->GetSession()).SendSysMessage("|cff4CFF00BattleRoyale::|r ¡No puedes hacer eso mientras participas en el modo Battle Royale!");
                return false;
            }
        }
        return true;
    }

    bool CanGroupInvite(Player *player, std::string & /*membername*/) override
    {
        if (sBattleRoyaleMgr->EstaActivado())
        {
            return !sBattleRoyaleMgr->DebeRestringirFunciones(player);
        }
        return true;
    }

    bool CanGroupAccept(Player *player, Group * /*group*/) override
    {
        if (sBattleRoyaleMgr->EstaActivado())
        {
            return !sBattleRoyaleMgr->DebeRestringirFunciones(player);
        }
        return true;
    }

    bool CanBattleFieldPort(Player *player, uint8 /*arenaType*/, BattlegroundTypeId /*BGTypeID*/, uint8 /*action*/) override
    {
        if (sBattleRoyaleMgr->EstaActivado())
        {
            if (sBattleRoyaleMgr->EstaEnCola(player) || sBattleRoyaleMgr->DebeRestringirFunciones(player))
            {
                ChatHandler(player->GetSession()).SendSysMessage("|cff4CFF00BattleRoyale::|r ¡No puedes hacer eso mientras participas en el modo Battle Royale!");
                return false;
            }
        }
        return true;
    }

    bool CanJoinInArenaQueue(Player *player, ObjectGuid /*BattlemasterGuid*/, uint8 /*arenaslot*/, BattlegroundTypeId /*BGTypeID*/, uint8 /*joinAsGroup*/, uint8 /*IsRated*/, GroupJoinBattlegroundResult &err) override
    {
        if (sBattleRoyaleMgr->EstaActivado())
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

    bool CanJoinInBattlegroundQueue(Player *player, ObjectGuid /*BattlemasterGuid*/, BattlegroundTypeId /*BGTypeID*/, uint8 /*joinAsGroup*/, GroupJoinBattlegroundResult &err) override
    {
        if (sBattleRoyaleMgr->EstaActivado())
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

    bool BRTodosContraTodos(Player *player) override
    {
        if (sBattleRoyaleMgr->EstaActivado())
        {
            return sBattleRoyaleMgr->TodosContraTodos(player);
        }
        return false;
    }

    void BRRespondeInvitacion(Player* player, bool agree, ObjectGuid summoner_guid) override
    {
        if (sBattleRoyaleMgr->EstaActivado())
        {
            sBattleRoyaleMgr->RespondeInvitacion(player, agree, summoner_guid);
        }
    }
};

class BattleRoyaleCreature : public CreatureScript
{

public:
    BattleRoyaleCreature() : CreatureScript("BattleRoyaleCreature") {}

    bool OnGossipHello(Player *player, Creature *creature)
    {
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Quiero saber más.", 0, 1);
        if (!sBattleRoyaleMgr->EstaEnCola(player))
        {
            AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Quiero unirme a la cola.", 0, 2);
            AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Votar para elegir mapa.", 0, 3);
        }
        else
        {
            AddGossipItemFor(player, GOSSIP_ICON_TALK, "Quiero salir de la cola.", 0, 4);
        }
        SendGossipMenuFor(player, 200000, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player *player, Creature *creature, uint32 sender, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
        case 1:
        {
            ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r Este modo de juego se encuentra en desarrollo (Versión Beta Actual v0.9.7). Los ganadores obtienen un título JcJ exclusivo que va mejorando cuanto más ganes. Además en algunos mapas aparecen cofres con recompensas aleatorias (Pociones, Emblemas y Honor). Por otro lado ya hay una misión diaria de participación, aunque se espera tener más misiones tanto diarias como semanales proximamente. Ahora puedes utilizar el comando '.br' para unirse al evento desde dónde estés o para salir de la cola.");
            CloseGossipMenuFor(player);
            break;
        }
        case 2:
        {
            if (sBattleRoyaleMgr->EstaActivado())
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
            if (sBattleRoyaleMgr->EstaActivado())
            {
                uint32 start = 5;
                BR_ContenedorMapas mapas = sBRMapasMgr->ObtenerMapas();
                for (BR_ContenedorMapas::iterator it = mapas.begin(); it != mapas.end(); ++it)
                {
                    AddGossipItemFor(player, GOSSIP_ICON_BATTLE, it->second->nombreMapa, it->first, start++);
                }
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
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
            if (sBattleRoyaleMgr->EstaActivado())
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
            if (sBattleRoyaleMgr->EstaActivado())
            {
                // TODO: Primero votar por el mapa antes de llamarlo a la cola, se debe comprobar requisitos.
                sBattleRoyaleMgr->GestionarJugadorEntrando(player);
                if (sBattleRoyaleMgr->EstaEnCola(player))
                {
                    sBRMapasMgr->VotarPorMapa(player->GetGUID().GetCounter(), sender);
                }
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
        if (sBattleRoyaleMgr->EstaActivado())
        {
            sBattleRoyaleMgr->GestionarActualizacionMundo(diff);
        }
    }
};

class BattleRoyaleItem : public ItemScript
{
public:
    BattleRoyaleItem() : ItemScript("BattleRoyaleItem") {}

    bool OnUse(Player *player, Item * /*item*/, const SpellCastTargets &) override
    {
        if (!sBattleRoyaleMgr->EstaActivado())
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
            {"unirme", HandleJoinCommand, SEC_PLAYER, Acore::ChatCommands::Console::No},
            {"salir", HandleLeaveCommand, SEC_PLAYER, Acore::ChatCommands::Console::No},
            {"recargar", HandleReloadCommand, 5, Acore::ChatCommands::Console::Yes},
            {"encender", HandleTurnOnCommand, 5, Acore::ChatCommands::Console::Yes},
            {"apagar", HandleTurnOffCommand, 5, Acore::ChatCommands::Console::Yes},
            {"iniciar", HandleStartCommand, 5, Acore::ChatCommands::Console::Yes}};

        static Acore::ChatCommands::ChatCommandTable baseTable = {
            {"br", commandTable}};

        return baseTable;
    }

    static bool HandleBRCommand(ChatHandler *handler)
    {
        if (sBattleRoyaleMgr->EstaActivado())
        {
            handler->SendSysMessage("El modo |cff4CFF00BattleRoyale|r se encuentra |cff00ff00activado|r.");
            handler->SendSysMessage("Puedes unirte a la cola mediante el comando '.br unirme'.");
            handler->SendSysMessage("Puedes salir de la cola mediante el comando '.br salir'.");
        }
        else
        {
            handler->SendSysMessage("El modo |cff4CFF00BattleRoyale|r se encuentra |cffff0000desactivado|r.");
        }
        return true;
    }

    static bool HandleJoinCommand(ChatHandler *handler)
    {
        if (sBattleRoyaleMgr->EstaActivado())
        {
            Player *me = handler->GetSession()->GetPlayer();
            if (!me)
                return false;
            sBattleRoyaleMgr->GestionarJugadorEntrando(me);
        }
        else
        {
            handler->SendSysMessage("El modo |cff4CFF00BattleRoyale|r se encuentra |cffff0000desactivado|r.");
        }
        return true;
    }

    static bool HandleLeaveCommand(ChatHandler *handler)
    {
        if (sBattleRoyaleMgr->EstaActivado())
        {
            Player *me = handler->GetSession()->GetPlayer();
            if (!me)
                return false;
            if (sBattleRoyaleMgr->EstaEnCola(me))
            {
                sBattleRoyaleMgr->GestionarJugadorDesconectar(me);
                handler->SendSysMessage("|cff4CFF00BattleRoyale::|r Ya no estas en cola para el evento.");
            }
            else
            {
                handler->SendSysMessage("|cff4CFF00BattleRoyale::|r ¡No estas en cola para el evento!");
            }
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

    static bool HandleTurnOnCommand(ChatHandler *handler)
    {
        if (sBattleRoyaleMgr->EstaActivado())
        {
            handler->SendSysMessage("El modo Battle Royale se encuentra activado.");
        }
        else
        {
            sBattleRoyaleMgr->ActivarSistema();
            handler->SendSysMessage("Se ha activado el modo Battle Royale.");
        }
        return true;
    }

    static bool HandleTurnOffCommand(ChatHandler *handler)
    {
        if (sBattleRoyaleMgr->EstaActivado())
        {
            sBattleRoyaleMgr->DesactivarSistema();
            handler->SendSysMessage("Se ha desactivado el modo Battle Royale.");
        }
        else
        {
            handler->SendSysMessage("El modo Battle Royale se encuentra desactivado.");
        }
        return true;
    }

    static bool HandleStartCommand(ChatHandler *handler)
    {
        if (sBattleRoyaleMgr->EstadoActual() == ESTADO_BR_SIN_SUFICIENTES_JUGADORES)
        {
            sBattleRoyaleMgr->ForzarIniciarNuevaRonda();
            if (sBattleRoyaleMgr->EstadoActual() != ESTADO_BR_SIN_SUFICIENTES_JUGADORES)
            {
                handler->SendSysMessage("Se ha forzado el inicio de la ronda de Battle Royale sin haber suficientes jugadores.");
            }
            else
            {
                handler->SendSysMessage("No se ha podido forzar el inicio de la ronda de Battle Royale. No funciona si no hay nadie en cola.");
            }
        }
        else
        {
            handler->SendSysMessage("Solo se puede forzar el inicio de una ronda de Battle Royale cuando no hay suficientes jugadores.");
        }
        return true;
    }
};

class BattleRoyaleSpell : public SpellSC
{
public:
    BattleRoyaleSpell() : SpellSC("BattleRoyaleSpell") {}

    void OnSpellCheckCast(Spell *spell, bool /*strict*/, SpellCastResult &res) override
    {
        if (spell)
        {
            Unit *uCaster = spell->GetCaster();
            Player *pCaster = uCaster && uCaster->GetTypeId() == TYPEID_PLAYER ? uCaster->ToPlayer() : nullptr;
            if (pCaster && sBattleRoyaleMgr->EstaEnEvento(pCaster))
            {
                if (const SpellInfo *spInf = spell->GetSpellInfo())
                {
                    if (spInf->HasAura(SPELL_AURA_FLY) || spInf->HasAura(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED))
                        res = SPELL_FAILED_NOT_HERE;
                }
            }
        }
    }
};

void AddSC_BattleRoyale()
{
    new BattleRoyalePlayer();
    new BattleRoyaleCreature();
    new BattleRoyaleWorld();
    new BattleRoyaleItem();
    new BattleRoyaleCommand();
    new BattleRoyaleSpell();
}
