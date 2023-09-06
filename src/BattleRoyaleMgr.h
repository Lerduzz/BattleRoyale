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
    ESTADO_NO_HAY_SUFICIENTES_JUGADORES     = 0,
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
    
    void GestionarJugadorEntrando(Player* player);
    void GestionarJugadorDesconectar(Player* player);
    void GestionarMuerteJcJ(Player* killer, Player* killed);
    void GestionarActualizacionMundo(uint32 diff);
    void PrevenirJcJEnLaNave(Player* player, bool state);
    bool PuedeReaparecerEnCementerio(Player* player);
    bool DebeRestringirFunciones(Player* player) { return estadoActual > ESTADO_NO_HAY_SUFICIENTES_JUGADORES && EstaEnEvento(player); };
    bool DebeForzarJcJTcT(Player* player) { return !(estadoActual != ESTADO_BATALLA_EN_CURSO || !HayJugadores() || !EstaEnEvento(player)) || !(obj_Nave && player->GetTransport() && player->GetExactDist(obj_Nave) < 25.0f); };
    
private:
    void RestablecerTodoElEvento();
    void IniciarNuevaRonda();
    void AlmacenarPosicionInicial(uint32 guid);
    void LlamarAntesQueNave(uint32 guid);    
    void LlamarDentroDeNave(uint32 guid);
    void SalirDelEvento(uint32 guid, bool logout = false);
    void RevivirJugador(Player *player);
    





    void NotifySecureZoneReduceWarn(uint32 delay);
    void NotifySecureZoneReduced();
    void NotifyTimeRemainingToStart(uint32 delay);
    void NotifyPvPKill(std::string killer, std::string killed, int kills);
    bool SpawnTransportShip();
    bool SpawnTheCenterOfBattle();
    bool SpawnSecureZone();
    void TeleportPlayersToShip();
    void AddParachuteToAllPlayers();
    void OutOfZoneDamage();
    void AddFFAPvPFlag();
    




    bool HayJugadores() { return list_Jugadores.size() > 0; };
    bool EstaEnCola(Player* player) { return list_Cola.find(player->GetGUID().GetCounter()) != list_Cola.end(); };
    bool EstaEnEvento(Player* player) { return list_Jugadores.find(player->GetGUID().GetCounter()) != list_Jugadores.end(); };
    bool EstaEnEvento(uint32 guid) { return list_Jugadores.find(guid) != list_Jugadores.end(); };
    bool EstaLlenoElEvento() { return list_Jugadores.size() >= conf_JugadoresMaximo; };
    bool HaySuficientesEnCola() { return list_Cola.size() >= conf_JugadoresMinimo; };
    ChatHandler Chat(Player* player) { return ChatHandler(player->GetSession()); };
    void SiguienteMapa() { if (indiceDelMapa++ >= CANTIDAD_DE_MAPAS) indiceDelMapa = 0; };
    void SiguientePosicion() { if (indiceDeVariacion++ >= CANTIDAD_DE_VARIACIONES) indiceDeVariacion = 0; };
    void CambiarDimension_Entrar(uint32 guid)
    {
        list_Jugadores[guid]->SetPhaseMask(2, false);
        list_Jugadores[guid]->UpdateObjectVisibility();
    };
    void CambiarDimension_Salir(uint32 guid)
    {
        list_Jugadores[guid]->SetPhaseMask(1, false);
        list_Jugadores[guid]->UpdateObjectVisibility();
    };
    void Desmontar(Player* player)
    {
        if (player && player->IsAlive() && player->IsMounted())
        {
            if (!player->IsInFlight())
            {
                player->Dismount();
                player->RemoveAurasByType(SPELL_AURA_MOUNTED);
                player->SetSpeed(MOVE_RUN, 1, true);
                player->SetSpeed(MOVE_FLIGHT, 1, true);
            }
        }
    };
        
    BR_ListaDePersonajes list_Cola;
    BR_ListaDePersonajes list_Jugadores;
    BR_DatosDePersonajes list_Datos;
    
    GameObject* obj_Zona;
    GameObject* obj_Centro;    
    GameObject* obj_Nave;

    int estadoActual;
    int tiempoRestanteSeg;    
    int indiceDeVariacion;
    int indiceDelMapa;

    uint32 conf_JugadoresMinimo;
    uint32 conf_JugadoresMaximo;
    uint32 conf_IntervaloEntreRecuccionDeZona;


    


    int secureZoneIndex;
    int secureZoneDelay;
    bool secureZoneAnnounced; 
    int secondsTicksHelper;

};

#define sBattleRoyaleMgr BattleRoyaleMgr::instance()

#endif
