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

// Estados del evento.
enum BREventStatus
{
    STATUS_NO_ENOUGH_PLAYERS                = 0,      // No hay suficientes jugadores.
    STATUS_SUMMONING_PLAYERS                = 1,      // Se ha comenzado a teletransportar jugadores a la zona central.
    STATUS_SHIP_WAITING                     = 2,      // Ahora se estan moviendo a los jugadores a la nave en espera.
    STATUS_SHIP_MOVING                      = 3,      // La nave esta en camino a su destino.
    STATUS_SHIP_NEAR_CENTER                 = 4,      // La nave se encuentra sobrevolando la zona segura.
    STATUS_BATTLE_STARTED                   = 5,      // La batalla ha iniciado.
};

// Hechizos utilizados.
enum BRSpells
{
    SPELL_PARACHUTE_DALARAN                 = 45472,  // Paracaidas que te ponen en Dalaran.
};

// Objetos de mapeado.
enum BRGameObjects
{
    GAMEOBJECT_SHIP                         = 194675, // El tren de Ulduar.
    GAMEOBJECT_MAP_CENTER                   = 500000, // CUSTOM: Centro del mapa (Para calculo de distancia de zona segura).
    GAMEOBJECT_SECURE_ZONE_BASE             = 500001, // CUSTOM: Cúpula de ulduar para identificar visualmente los límites de la zona segura.
    SECURE_ZONE_COUNT                       = 10,     // Cantidad de zonas seguras (ID consecutivos a partir de GAMEOBJECT_SECURE_ZONE_BASE).
};

const float BRSecureZoneScales[SECURE_ZONE_COUNT] = { 5.0f, 4.5f, 4.0f, 3.5f, 3.0f, 2.5f, 2.0f, 1.5f, 1.0f, 0.5f };

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
    void HandleOnPVPLill(Player* killer, Player* killed);
    bool HandleReleaseGhost(Player *player);
    void HandleOnWoldUpdate(uint32 diff);
    void TeleportToEvent(uint32 guid);
    bool ForceFFAPvPFlag(Player* player);
    void PreventPvPBeforeBattle(Player* player, bool state);
    bool RestrictPlayerFunctions(Player* player);

    
private:
    void EnterToPhaseEvent(uint32 guid);
    void ExitFromPhaseEvent(uint32 guid);
    void ResurrectPlayer(Player *player);
    void NotifySecureZoneReduceWarn(uint32 delay);
    void NotifySecureZoneReduced();
    void NotifyTimeRemainingToStart(uint32 delay);
    void NotifyPvPKill(std::string killer, std::string killed, int kills);
    bool SpawnTransportShip();
    bool SpawnTheCenterOfBattle();
    bool SpawnSecureZone();
    void StorePlayerStartPosition(uint32 guid);
    void TeleportPlayerBeforeShip(uint32 guid);
    void TeleportPlayerToShip(uint32 guid);
    void TeleportPlayersToShip();
    void Dismount(Player* player);
    void AddParachuteToAllPlayers();
    void OutOfZoneDamage();
    void AddFFAPvPFlag();
    void ExitFromEvent(uint32 guid, bool logout = false);
    void ResetFullEvent();

    bool IsInQueue(Player* player) { return ep_PlayersQueue.find(player->GetGUID().GetCounter()) != ep_PlayersQueue.end(); };
    bool IsInEvent(Player* player) { return ep_Players.find(player->GetGUID().GetCounter()) != ep_Players.end(); };
    bool IsQueuedEnoughPlayers() { return ep_PlayersQueue.size() >= eventMinPlayers; };
    bool IsEventFull() { return ep_Players.size() >= eventMaxPlayers; };
    
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

    uint32 eventMinPlayers;
    uint32 eventMaxPlayers;

    int secondsTicksHelper;
    int startRemainingTime;

    int summonOffsetIndex;    
};

#define sBattleRoyaleMgr BattleRoyaleMgr::instance()

#endif
