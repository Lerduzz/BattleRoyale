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

// 49 Offsets para que los jugadores no salgan apilados.
const int BROffsetsCount = 49;
const float ShipOffsets[BROffsetsCount][2] = 
{
    { 0.0f, 0.0f },
    { 0.0f, 1.5f },
    { 0.0f, 3.0f },
    { 0.0f, 4.5f },
    { 0.0f, -1.5f },
    { 0.0f, -3.0f },
    { 0.0f, -4.5f },
    { 1.5f, 0.0f },
    { 1.5f, 1.5f },
    { 1.5f, 3.0f },
    { 1.5f, 4.5f },
    { 1.5f, -1.5f },
    { 1.5f, -3.0f },
    { 1.5f, -4.5f },
    { 3.0f, 0.0f },
    { 3.0f, 1.5f },
    { 3.0f, 3.0f },
    { 3.0f, 4.5f },
    { 3.0f, -1.5f },
    { 3.0f, -3.0f },
    { 3.0f, -4.5f },
    { 4.5f, 0.0f },
    { 4.5f, 1.5f },
    { 4.5f, 3.0f },
    { 4.5f, 4.5f },
    { 4.5f, -1.5f },
    { 4.5f, -3.0f },
    { 4.5f, -4.5f },
    { -1.5f, 0.0f },
    { -1.5f, 1.5f },
    { -1.5f, 3.0f },
    { -1.5f, 4.5f },
    { -1.5f, -1.5f },
    { -1.5f, -3.0f },
    { -1.5f, -4.5f },
    { -3.0f, 0.0f },
    { -3.0f, 1.5f },
    { -3.0f, 3.0f },
    { -3.0f, 4.5f },
    { -3.0f, -1.5f },
    { -3.0f, -3.0f },
    { -3.0f, -4.5f },
    { -4.5f, 0.0f },
    { -4.5f, 1.5f },
    { -4.5f, 3.0f },
    { -4.5f, 4.5f },
    { -4.5f, -1.5f },
    { -4.5f, -3.0f },
    { -4.5f, -4.5f }
};

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
    
    void HandlePlayerJoin(Player *player);
    void HandlePlayerLogout(Player *player);
    void HandleGiveReward(Player *player);
    void HandleReleaseGhost(Player *player, uint32 oldArea, uint32 newArea);
    void HandleOnWoldUpdate(uint32 diff);
    void StartEvent(uint32 guid);
    void TeleportToEvent(uint32 guid);
    void ExitFromEvent(uint32 guid);
    bool ForceFFAPvPFlag(Player* player);
    bool RestrictPlayerFunctions(Player* player);

    
private:
    void EnterToPhaseEvent(uint32 guid);
    void ExitFromPhaseEvent(uint32 guid);
    void ResurrectPlayer(Player *player);
    void NotifySecureZoneReduceWarn(uint32 delay);
    void NotifySecureZoneReduced();
    void NotifyTimeRemainingToStart(uint32 delay);
    bool SpawnTransportShip();
    bool SpawnTheCenterOfBattle();
    bool SpawnSecureZone();
    void StorePlayerStartPosition(uint32 guid);
    void TeleportPlayerBeforeShip(uint32 guid);
    void TeleportPlayerToShip(uint32 guid);
    void TeleportPlayersToShip();
    void OutOfZoneDamage();
    void ResetFullEvent();
    
    BattleRoyalePlayerList ep_PlayersQueue;
    BattleRoyalePlayerList ep_Players;
    BattleRoyalePlayerData ep_PlayersData;
    
    GameObject* go_SecureZone;
    GameObject* go_CenterOfBattle;    
    GameObject* go_TransportShip;

    int secureZoneIndex;
    int secureZoneDelay;
    bool secureZoneAnnounced;
    
    int rotationMapIndex;
    int eventCurrentStatus;

    int eventMinPlayers;
    int eventMaxPlayers;

    int secondsTicksHelper;
    int summonRemainingTime;

    int summonOffsetIndex;    
};

#define sBattleRoyaleMgr BattleRoyaleMgr::instance()

#endif