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

const int CANTIDAD_DE_MAPAS = 1;
const int BR_IdentificadorDeMapas[CANTIDAD_DE_MAPAS] = { 1 };

const Position BR_CentroDeMapas[CANTIDAD_DE_MAPAS] =
{
    { 5261.581055f, -2164.183105f, 1259.483765f }
};

const std::string BR_NombreDeMapas[CANTIDAD_DE_MAPAS] =
{
    "Kalimdor: Hyjal"
};

const float BR_InicioDeLaNave[CANTIDAD_DE_MAPAS][4] =
{
    { 2967.581055f, -2164.183105f, 1556.483765f, 0.0f - M_PI / 2.0f }
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
    void GestionarMuerteJcJ(Player* killer, Player* killed);
    void GestionarActualizacionMundo(uint32 diff);
    void PrevenirJcJEnLaNave(Player* player, bool state);
    bool PuedeReaparecerEnCementerio(Player *player);
    bool DebeRestringirFunciones(Player* player) { return eventCurrentStatus > ESTADO_NO_SUFICIENTES_JUGADORES && IsInEvent(player); };
    bool DebeForzarJcJTcT(Player* player) { return !(eventCurrentStatus != ESTADO_BATALLA_EN_CURSO || !HayJugadores() || !IsInEvent(player)) || !(obj_Nave && player->GetTransport() && player->GetExactDist(obj_Nave) < 25.0f); };
    
private:
    void RestablecerTodoElEvento();
    void TeleportToEvent(uint32 guid);
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
    bool IsInQueue(Player* player) { return ep_PlayersQueue.find(player->GetGUID().GetCounter()) != ep_PlayersQueue.end(); };
    bool IsInEvent(Player* player) { return ep_Players.find(player->GetGUID().GetCounter()) != ep_Players.end(); };
    bool IsQueuedEnoughPlayers() { return ep_PlayersQueue.size() >= conf_JugadoresMinimo; };
    bool IsEventFull() { return ep_Players.size() >= conf_JugadoresMaximo; };
    bool HayJugadores() { return ep_Players.size() > 0; };
    
    BR_ListaDePersonajes ep_PlayersQueue;
    BR_ListaDePersonajes ep_Players;
    BR_DatosDePersonajes ep_PlayersData;
    
    GameObject* obj_Zona;
    GameObject* obj_Centro;    
    GameObject* obj_Nave;

    int secureZoneIndex;
    int secureZoneDelay;
    bool secureZoneAnnounced;
    
    int rotationMapIndex;
    int eventCurrentStatus;

    int secondsTicksHelper;
    int startRemainingTime;

    int summonOffsetIndex;

    uint32 conf_JugadoresMinimo;
    uint32 conf_JugadoresMaximo;
    uint32 conf_IntervaloEntreRecuccionDeZona;
};

#define sBattleRoyaleMgr BattleRoyaleMgr::instance()

#endif
