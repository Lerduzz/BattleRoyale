#ifndef SC_BR_MGR_H
#define SC_BR_MGR_H

#include "Common.h"
#include "SharedDefines.h"
#include "ScriptMgr.h"
#include "BattleRoyaleData.h"
#include "Chat.h"
#include "Player.h"

class BattleRoyaleData;

// GUIDLow es la llave, Lista de personajes en cola para el evento.
typedef std::map<uint32, Player*> BattleRoyalePlayerQueue;

// GUIDLow es la llave, Lista de personajes en el evento.
typedef std::map<uint32, Player*> BattleRoyalePlayerList;

// GUIDLow es la llave, Datos de los personajes en el evento.
typedef std::map<uint32, BattleRoyaleData> BattleRoyalePlayerData;

class BattleRoyaleMgr
{
    BattleRoyaleMgr();
    ~BattleRoyaleMgr();

public:
    static BattleRoyaleMgr *instance()
    {
        static BattleRoyaleMgr *instance = new BattleRoyaleMgr();
        return instance;
    }
    
    void HandleDismountFly(Player *player);
    void HandlePlayerJoin(Player *player);
    void HandlePlayerLogout(Player *player);
    void HandleGiveReward(Player *player);
    void HandleReleaseGhost(Player *player, uint32 oldArea, uint32 newArea);
    void HandleOnWoldUpdate(uint32 diff);
    void StartEvent(uint32 guid);
    void TeleportToEvent(uint32 guid);
    void ExitFromEvent(uint32 guid);
    bool ForceFFAPvPFlag(Player* player);
    
private:
    void EnterToPhaseEvent(uint32 guid);
    void ExitFromPhaseEvent(uint32 guid);
    void ResurrectPlayer(Player *player);
    void SendNotification(uint32 guid, uint32 delay);
    
    BattleRoyalePlayerList ep_PlayersQueue;
    BattleRoyalePlayerList ep_Players;
    BattleRoyalePlayerData ep_PlayersData;
    
    GameObject* secureZone;
    int secureZoneIndex;
    int secureZoneDelay;
    bool secureZoneAnnounced;
    
    int rotationMapIndex;
    int eventCurrentStatus;

    int eventMinPlayers;
    int eventMaxPlayers;
};

#define sBattleRoyaleMgr BattleRoyaleMgr::instance()

#endif