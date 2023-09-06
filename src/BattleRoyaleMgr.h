#ifndef SC_BR_MGR_H
#define SC_BR_MGR_H

#include "Common.h"
#include "SharedDefines.h"
#include "ScriptMgr.h"
#include "BattleRoyaleData.h"
#include "Chat.h"
#include "Player.h"

class BattleRoyaleData;
typedef std::map<uint32, Player*> BR_ColaDePersonajes;
typedef std::map<uint32, Player*> BR_ListaDePersonajes;
typedef std::map<uint32, BattleRoyaleData> BR_DatosDePersonajes;

enum BR_EstadosEvento
{
    ESTADO_NO_SUFICIENTES_JUGADORES         = 0,
    ESTADO_INVOCANDO_JUGADORES              = 1,
    ESTADO_NAVE_EN_ESPERA                   = 2,
    ESTADO_NAVE_EN_MOVIMIENTO               = 3,
    ESTADO_NAVE_CERCA_DEL_CENTRO            = 4,
    ESTADO_BATALLA_EN_CURSO                 = 5,
};

enum BR_Hechizos
{
    HECHIZO_PARACAIDAS                      = 45472,
};

enum BR_ObjetosMundo
{
    OBJETO_NAVE                             = 194675,
    OBJETO_CENTRO_DEL_MAPA                  = 500000,
    OBJETO_ZONA_SEGURA_INICIAL              = 500001,
};

const int CANTIDAD_DE_ZONAS                 = 10;
const float BR_EscalasDeZonaSegura[CANTIDAD_DE_ZONAS] = { 5.0f, 4.5f, 4.0f, 3.5f, 3.0f, 2.5f, 2.0f, 1.5f, 1.0f, 0.5f };

const int CANTIDAD_DE_VARIACIONES = 49;
const float BR_VariacionesDePosicion[CANTIDAD_DE_VARIACIONES][2] = 
{
    { -4.5f,  4.5f }, { -3.0f,  4.5f }, { -1.5f,  4.5f }, {  0.0f,  4.5f }, {  1.5f,  4.5f }, {  3.0f,  4.5f }, {  4.5f,  4.5f },
    { -4.5f,  3.0f }, { -3.0f,  3.0f }, { -1.5f,  3.0f }, {  0.0f,  3.0f }, {  1.5f,  3.0f }, {  3.0f,  3.0f }, {  4.5f,  3.0f },
    { -4.5f,  1.5f }, { -3.0f,  1.5f }, { -1.5f,  1.5f }, {  0.0f,  1.5f }, {  1.5f,  1.5f }, {  3.0f,  1.5f }, {  4.5f,  1.5f },
    { -4.5f,  0.0f }, { -3.0f,  0.0f }, { -1.5f,  0.0f }, {  0.0f,  0.0f }, {  1.5f,  0.0f }, {  3.0f,  0.0f }, {  4.5f,  0.0f },
    { -4.5f, -1.5f }, { -3.0f, -1.5f }, { -1.5f, -1.5f }, {  0.0f, -1.5f }, {  1.5f, -1.5f }, {  3.0f, -1.5f }, {  4.5f, -1.5f },
    { -4.5f, -3.0f }, { -3.0f, -3.0f }, { -1.5f, -3.0f }, {  0.0f, -3.0f }, {  1.5f, -3.0f }, {  3.0f, -3.0f }, {  4.5f, -3.0f },
    { -4.5f, -4.5f }, { -3.0f, -4.5f }, { -1.5f, -4.5f }, {  0.0f, -4.5f }, {  1.5f, -4.5f }, {  3.0f, -4.5f }, {  4.5f, -4.5f }
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
    
    void GestionarJugadorEntrando(Player *player);
    void GestionarJugadorDesconectar(Player *player);
    void GestionarMuesteJcJ(Player* killer, Player* killed);
    bool GestionarLiberarEspiritu(Player *player);
    void GestionarActualizacionMundo(uint32 diff);
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
    
    BR_ListaDePersonajes ep_PlayersQueue;
    BR_ListaDePersonajes ep_Players;
    BR_DatosDePersonajes ep_PlayersData;
    
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
