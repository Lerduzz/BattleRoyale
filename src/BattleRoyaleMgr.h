#ifndef SC_EPMGR_H
#define SC_EPMGR_H

#include "Common.h"
#include "SharedDefines.h"
#include "ScriptMgr.h"
#include "BattleRoyaleData.h"
#include "Chat.h"
#include "Player.h"

class BattleRoyaleData;

// GUIDLow es la llave, Lista de personajes en el evento.
typedef std::map<uint32, Player*> ParkourPlayerList;

// GUIDLow es la llave, Datos de los personajes en el evento.
typedef std::map<uint32, BattleRoyaleData> ParkourPlayerData;

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
    
private:
    void EnterToPhaseDelay(uint32 guid);
    void EnterToPhaseEvent(uint32 guid);
    void ExitFromPhaseEvent(uint32 guid);
    void ResurrectPlayer(Player *player);
    void CheckForHacks(uint32 guid);
    void SendNotification(uint32 guid, uint32 delay);
    
    ParkourPlayerList ep_Players;
    ParkourPlayerData ep_PlayersData;
    
    bool hasTeleported;
    bool hasEventStarted;
    bool hasEventClose;
	bool inTimeToEvent;
	bool hasAnnouncedEvent;
	bool hasEventEnded;
    
    uint32 nextReward;
    uint32 startDelay;
    uint32 secondsDelay;
    
    uint32 hackCheckDelay;
    
};

#define sBattleRoyaleMgr BattleRoyaleMgr::instance()

#endif