#ifndef SC_BR_MGR_H
#define SC_BR_MGR_H

#include "Common.h"
#include "SharedDefines.h"
#include "ScriptMgr.h"
#include "BattleRoyaleData.h"
#include "Chat.h"
#include "Player.h"
#include "GameObject.h"
#include "Transport.h"

class BattleRoyaleData;
typedef std::map<uint32, Player*> BR_ColaDePersonajes;
typedef std::map<uint32, Player*> BR_ListaDePersonajes;
typedef std::map<uint32, BattleRoyaleData> BR_DatosDePersonajes;

enum BR_Dimensiones
{
    DIMENSION_NORMAL                        = 0x00000001,
    DIMENSION_EVENTO                        = 0x00000002,
};

enum BR_EstadosEvento
{
    ESTADO_NO_HAY_SUFICIENTES_JUGADORES     = 0,
    ESTADO_INVOCANDO_JUGADORES              = 1,
    ESTADO_NAVE_EN_MOVIMIENTO               = 2,
    ESTADO_NAVE_CERCA_DEL_CENTRO            = 3,
    ESTADO_BATALLA_EN_CURSO                 = 4,
    ESTADO_BATALLA_TERMINADA                = 5,
};

enum BR_Hechizos
{
    HECHIZO_PARACAIDAS                      = 45472,
    HECHIZO_ANTI_SANADORES                  = 55593,
};

enum BR_ObjetosMundo
{
    OBJETO_NAVE                             = 194675,
    OBJETO_COFRE                            = 499999,
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

const int CANTIDAD_DE_MAPAS = 4;
const int BR_IdentificadorDeMapas[CANTIDAD_DE_MAPAS] = { 1, 0, 1, 1 };

const Position BR_CentroDeMapas[CANTIDAD_DE_MAPAS] =
{
    { 5261.581055f, -2164.183105f, 1259.483765f },
    { -14614.625001f, -313.262604f, 0.000001f },
    { 157.995544f, -1948.086061f, 87.387062f },
    { -4695.309082f, 3411.214844f, 7.050505f }
};

const std::string BR_NombreDeMapas[CANTIDAD_DE_MAPAS] =
{
    "Kalimdor: Hyjal",
    "Reinos del Este: Isla Jaguero",
    "Kalimdor: Las Charcas del Olvido",
    "Kalimdor: Ruinas de Solarsal"
};

const float BR_InicioDeLaNave[CANTIDAD_DE_MAPAS][4] =
{
    { 2967.581055f, -2164.183105f, 1556.483765f, 0.0f - M_PI / 2.0f },
    { -12320.625001f, -313.262604f, 297.000001f, 0.0f + M_PI / 2.0f },
    { -2137.581055f, -1948.086061f, 394.387062f, 0.0f - M_PI / 2.0f },
    { -2401.309082f, 3411.214844f, 304.050505f, 0.0f + M_PI / 2.0f }
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
    bool DebeRestringirFunciones(Player* player) { return estadoActual > ESTADO_NO_HAY_SUFICIENTES_JUGADORES && HayJugadores() && EstaEnEvento(player); };
    bool DebeForzarJcJTcT(Player* player) 
    {
        if (!player) return false;
        if (estadoActual != ESTADO_BATALLA_EN_CURSO || !HayJugadores() || !EstaEnEvento(player)) return false;
        return !EstaEnLaNave(player);
    };
    
private:
    void RestablecerTodoElEvento();
    void IniciarNuevaRonda();
    void AlmacenarPosicionInicial(uint32 guid);
    void LlamarDentroDeNave(uint32 guid);
    void SalirDelEvento(uint32 guid, bool logout = false);
    void RevivirJugador(Player *player);
    bool InvocarNave();
    bool InvocarCentroDelMapa();
    bool InvocarZonaSegura();
    void PonerTodosLosParacaidas();
    void EfectoFueraDeZona();
    void ActivarJcJTcT();
    void ControlDeReglas();
    void CondicionDeVictoria();
    void FinalizarRonda(bool announce, Player* winner = nullptr);
    bool HayJugadores() { return !list_Jugadores.empty(); };
    bool HayCola() { return !list_Cola.empty(); };
    bool EstaEnCola(Player* player) { return list_Cola.find(player->GetGUID().GetCounter()) != list_Cola.end(); };
    bool EstaEnCola(uint32 guid) { return list_Cola.find(guid) != list_Cola.end(); };
    bool EstaEnEvento(Player* player) { return list_Jugadores.find(player->GetGUID().GetCounter()) != list_Jugadores.end(); };
    bool EstaEnEvento(uint32 guid) { return list_Jugadores.find(guid) != list_Jugadores.end(); };
    bool EstaEnLaNave(Player* player)
    {
        if (player && obj_Nave)
        {
            if (Transport* tp = obj_Nave->ToTransport())
            {
                if (Transport* playertp = player->GetTransport())
                {
                    if (tp == playertp) return true;
                }
            }
        }
        return false;
    };
    bool EstaLlenoElEvento() { return list_Jugadores.size() >= conf_JugadoresMaximo; };
    bool HaySuficientesEnCola() { return list_Cola.size() >= conf_JugadoresMinimo; };
    ChatHandler Chat(Player* player) { return ChatHandler(player->GetSession()); };
    void SiguienteMapa() { if (++indiceDelMapa >= CANTIDAD_DE_MAPAS) indiceDelMapa = 0; };
    void SiguientePosicion() { if (++indiceDeVariacion >= CANTIDAD_DE_VARIACIONES) indiceDeVariacion = 0; };
    void SalirDeGrupo(Player* player)
    { 
        player->RemoveFromGroup(); 
        player->UninviteFromGroup();
    };
    void CambiarDimension_Entrar(uint32 guid) { list_Jugadores[guid]->SetPhaseMask(DIMENSION_EVENTO, true); };
    void CambiarDimension_Salir(uint32 guid) { list_Jugadores[guid]->SetPhaseMask(DIMENSION_NORMAL, true); };
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
    void NotificarTiempoParaIniciar(uint32 delay)
    {
        if (HayJugadores())
        {
            for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it) {
                switch (delay)
                {
                    case 0:
                    {
                        (*it).second->GetSession()->SendNotification("|cff00ff00¡Que comience la batalla de |cffDA70D6%s|cff00ff00!", BR_NombreDeMapas[indiceDelMapa].c_str());
                        break;
                    }                
                    case 5:
                    {
                        (*it).second->GetSession()->SendNotification("|cff00ff00Ya tienes paracaidas. |cff0000ff¡PUEDES SALTAR!");
                        break;
                    }                
                    case 10:
                    case 15:
                    case 20:
                    case 25:
                    {
                        (*it).second->GetSession()->SendNotification("|cff00ff00Faltan |cffDA70D6%u|cff00ff00 segundos para llegar. |cffff0000¡NO TE TIRES!", delay - 5);
                        break;
                    }
                    case 30:
                    {
                        (*it).second->GetSession()->SendNotification("|cff00ff00La nave comienza a moverse. |cffff0000¡NO TE TIRES!");
                        break;
                    }
                    default:
                    {
                        if (delay > 30 && delay <= 60)
                        {
                            (*it).second->GetSession()->SendNotification("|cff00ff00Faltan |cffDA70D6%u|cff00ff00 segundos para encender motores. |cffff0000¡NO TE TIRES!", delay - 30);
                        }
                        break;
                    }
                }
            }
        }
    };
    void NotificarZonaReducida()
    {
        if (HayJugadores())
        {
            for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
            {
                (*it).second->GetSession()->SendNotification("|cffff0000¡ALERTA: La zona segura se ha actualizado!");
            }
        }
    };
    void NotificarAdvertenciaDeZona(uint32 delay)
    {
        if (HayJugadores())
        {
            for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
            {
                (*it).second->GetSession()->SendNotification("|cff00ff00¡La zona segura se reducirá en |cffDA70D6%u|cff00ff00 segundos!", delay);
            }
        }
    };
    void NotificarMuerteJcJ(std::string killer, std::string killed, int kills)
    {
        if (HayJugadores())
        {
            for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
            {
                Chat((*it).second).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡%s ha eliminado a %s!, racha: |cff4CFF00%i|r.", killer, killed, kills);
            }
        }
    };
    void NotificarNaveRetirada()
    {
        if (HayJugadores())
        {
            for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
            {
                (*it).second->GetSession()->SendNotification("|cff0000ff¡La nave se ha retirado!");
            }
        }
    };
    void NotificarGanadorAlMundo(Player* winner, int kills)
    {
        std::ostringstream msg;
        msg << "|cff4CFF00BattleRoyale::|r Ronda finalizada, ganador: " << Chat(winner).GetNameLink(winner) << ", víctimas: |cff4CFF00" << kills << "|r.";
        sWorld->SendServerMessage(SERVER_MSG_STRING, msg.str().c_str());
    }
    void DesaparecerTodosLosObjetos()
    {
        DesaparecerZona();
        DesaparecerCentro();
        DesaparecerNave();
    };
    bool DesaparecerZona()
    {
        if (obj_Zona) {
            obj_Zona->CleanupsBeforeDelete();
            delete obj_Zona;
            obj_Zona = nullptr;
            return true;
        }
        return false;
    };
    bool DesaparecerCentro()
    {
        if (obj_Centro) {
            obj_Centro->CleanupsBeforeDelete();
            delete obj_Centro;
            obj_Centro = nullptr;
            return true;
        }
        return false;
    };
    bool DesaparecerNave()
    {
        if (obj_Nave) {
            if(Transport* tp = obj_Nave->ToTransport())
            {
                tp->CleanupsBeforeDelete();
            }
            else
            {
                obj_Nave->CleanupsBeforeDelete();
            }
            delete obj_Nave;
            obj_Nave = nullptr;
            return true;
        }
        return false;
    };
    void HechizosDeZona()
    {
        if (estadoActual == ESTADO_BATALLA_EN_CURSO && HayJugadores())
        {
            for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
            {
                if ((*it).second && (*it).second->IsAlive())
                {
                    (*it).second->AddAura(HECHIZO_ANTI_SANADORES, (*it).second);
                }
            }
        }
    };
    bool EspectarPrimeroDisponible(Player* player)
    {
        if (HayJugadores() && player && !player->IsAlive() /*&& player->isPossessing()*/)
        {
            for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
            {
                if ((*it).second && (*it).second != player && (*it).second->IsAlive() && (*it).second->GetExactDist(player) <= 666.0f)
                {
                    player->CastSpell((*it).second, 6277, true);
                    return true;
                }
            }
        }
        return false;
    };
    void TodosLosMuertosEspectarme(Player* player)
    {
        if (HayJugadores() && player && player->IsAlive())
        {
            for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
            {
                if ((*it).second && (*it).second != player && !(*it).second->IsAlive() && (*it).second->isPossessing())
                {
                    LOG_ERROR("br", "TodosLosMuertosEspectarme: Nuevo espectador agregado GUID {}.", (*it).first);
                    (*it).second->CastSpell(player, 6277, true);
                }
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
    int tiempoRestanteInicio;
    int tiempoRestanteZona;
    int tiempoRestanteNave;
    int tiempoRestanteFinal;
    int indiceDeVariacion;
    int indiceDelMapa;
    int indiceDeZona;
    int indicadorDeSegundos;
    bool estaLaZonaAnunciada;
    bool estaLaZonaActiva;

    uint32 conf_JugadoresMinimo;
    uint32 conf_JugadoresMaximo;
    uint32 conf_IntervaloDeZona;

};

#define sBattleRoyaleMgr BattleRoyaleMgr::instance()

#endif
