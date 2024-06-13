#include "mgr/EventoMgr.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"

class BRScriptCriatura : public CreatureScript
{

public:
    BRScriptCriatura() : CreatureScript("BRScriptCriatura") {}

    bool OnGossipHello(Player *player, Creature *creature)
    {
        if (!sEventoMgr->EstaEnCola(player))
        {
            AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Unirme a un mapa aleatorio.", 0, 1);
            // AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Votar por un mapa específico.", 0, 2);
        }
        else
        {
            AddGossipItemFor(player, GOSSIP_ICON_TALK, "Salir de la cola.", 0, 3);
        }
        SendGossipMenuFor(player, BR_CRIATURA_GESTOR_COLA, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player *player, Creature *creature, uint32 sender, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
        case 1:
        {
            // if (sBattleRoyaleMgr->EstaActivado())
            // {
            //     sBattleRoyaleMgr->GestionarJugadorEntrando(player);
            // }
            // else
            // {
            //     ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡Este modo de juego se encuentra actualmente desactivado!");
            // }
            sEventoMgr->JugadorEntrando(player);
            CloseGossipMenuFor(player);
            break;
        }
        case 2:
        {
            // if (sBattleRoyaleMgr->EstaActivado())
            // {
            //     uint32 start = 5;
            //     BR_ContenedorMapas mapas = sBRMapasMgr->ObtenerMapas();
            //     for (BR_ContenedorMapas::iterator it = mapas.begin(); it != mapas.end(); ++it)
            //     {
            //         AddGossipItemFor(player, GOSSIP_ICON_BATTLE, it->second->nombreMapa, it->first, start++);
            //     }
            //     SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            // }
            // else
            // {
            //     ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡Este modo de juego se encuentra actualmente desactivado!");
            //     CloseGossipMenuFor(player);
            // }
            break;
        }
        case 3:
        {
            // if (sBattleRoyaleMgr->EstaActivado())
            // {
            //     if (sBattleRoyaleMgr->EstaEnCola(player))
            //     {
            //         sBattleRoyaleMgr->GestionarJugadorDesconectar(player);
            //         ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r Ya no estas en cola para el evento.");
            //     }
            //     else
            //     {
            //         ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡No estas en cola para el evento!");
            //     }
            // }
            // else
            // {
            //     ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡Este modo de juego se encuentra actualmente desactivado!");
            // }
            CloseGossipMenuFor(player);
            break;
        }
        default:
        {
            // if (sBattleRoyaleMgr->EstaActivado())
            // {
            //     // TODO: Primero votar por el mapa antes de llamarlo a la cola, se debe comprobar requisitos.
            //     sBattleRoyaleMgr->GestionarJugadorEntrando(player);
            //     if (sBattleRoyaleMgr->EstaEnCola(player))
            //     {
            //         sBRMapasMgr->VotarPorMapa(player->GetGUID().GetCounter(), sender);
            //     }
            // }
            // else
            // {
            //     ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡Este modo de juego se encuentra actualmente desactivado!");
            // }
            CloseGossipMenuFor(player);
            break;
        }
        }
        return true;
    }
};

void AddSC_BattleRoyale()
{
    new BRScriptCriatura();
}
