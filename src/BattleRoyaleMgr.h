#ifndef SC_BR_MGR_H
#define SC_BR_MGR_H

#include "Constantes.h"
#include "BREquipamientoMgr.h"
#include "BRChatMgr.h"
#include "BRListaNegraMgr.h"
#include "BRMapasMgr.h"
#include "BRMisionesMgr.h"
#include "BRObjetosMgr.h"
#include "BRRecompensaMgr.h"
#include "BRSonidosMgr.h"
#include "BRTitulosMgr.h"
#include "BattleRoyaleData.h"
#include "Common.h"
#include "SharedDefines.h"
#include "ScriptMgr.h"
#include "Chat.h"

class BattleRoyaleData;
typedef std::map<uint32, Player *> BR_ListaDePersonajes;
typedef std::map<uint32, BattleRoyaleData> BR_DatosDePersonajes;

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

    inline bool EstaActivado() { return conf_EstaActivado; };
    inline void ActivarSistema() { conf_EstaActivado = true; };

    void DesactivarSistema()
    {
        conf_EstaActivado = false;
        while (HayJugadores())
            SalirDelEvento(list_Jugadores.begin()->first);
        while (HayCola())
            SalirDelEvento(list_Cola.begin()->first);
        RestablecerTodoElEvento();
    }

    void GestionarJugadorEntrando(Player *player);
    void GestionarJugadorDesconectar(Player *player);
    void GestionarMuerteJcJ(Player *killer, Player *killed);
    void GestionarActualizacionMundo(uint32 diff);
    void PrevenirJcJEnLaNave(Player *player, bool state);
    bool PuedeReaparecerEnCementerio(Player *player);

    inline bool DebeRestringirFunciones(Player *player) { return estadoActual > ESTADO_BR_SIN_SUFICIENTES_JUGADORES && HayJugadores() && EstaEnEvento(player); };
    inline bool EstaEnCola(Player *player) { return EstaEnCola(player->GetGUID().GetCounter()); };
    inline bool EstaInvitado(Player *player) { return EstaInvitado(player->GetGUID().GetCounter()); };
    inline bool EstaEnEvento(Player *player) { return EstaEnEvento(player->GetGUID().GetCounter()); };

    bool TodosContraTodos(Player *player)
    {
        if (!player)
            return false;
        if ((estadoActual < ESTADO_BR_ZONA_EN_ESPERA || estadoActual == ESTADO_BR_BATALLA_TERMINADA) || !HayJugadores() || !EstaEnEvento(player))
            return false;
        return !sBRObjetosMgr->EstaEnLaNave(player);
    }

    inline BREstado EstadoActual() { return estadoActual; };

    bool ForzarIniciarNuevaRonda()
    {
        if (HayCola())
        {
            IniciarNuevaRonda();
            return true;
        }
        return false;
    }

    void RespondeInvitacion(Player *player, bool agree, ObjectGuid summoner_guid);
    
    void ProgramarQuitarObjetos(Player *player)
    {
        uint32 guid = player->GetGUID().GetCounter();
        list_QuitarTodosLosObjetos[guid] = player;
    }

private:
    void RestablecerTodoElEvento();
    void IniciarNuevaRonda();
    void AlmacenarPosicionInicial(uint32 guid);
    void LlamarDentroDeNave(uint32 guid, uint32 tiempo = 20000);
    void SalirDelEvento(uint32 guid, bool logout = false);
    void RevivirJugador(Player *player);
    void EfectoFueraDeZona();
    void ActivarTodosContraTodos();
    void ControlDeReglas();
    bool CondicionDeVictoria();
    void FinalizarRonda(bool announce, Player *winner = nullptr);

    inline bool HayJugadores() { return !list_Jugadores.empty(); };
    inline bool HayCola() { return !list_Cola.empty(); };
    inline bool EstaEnCola(uint32 guid) { return list_Cola.find(guid) != list_Cola.end(); };
    inline bool EstaInvitado(uint32 guid) { return list_Invitados.find(guid) != list_Invitados.end(); };
    inline bool EstaEnEvento(uint32 guid) { return list_Jugadores.find(guid) != list_Jugadores.end(); };
    inline bool EstaEnListaDeAlas(Player *player) { return EstaEnListaDeAlas(player->GetGUID().GetCounter()); };
    inline bool EstaEnListaDeAlas(uint32 guid) { return EstaEnListaDarObjetosIniciales(guid) || EstaEnListaQuitarTodosLosObjetos(guid); };
    inline bool EstaEnListaDarObjetosIniciales(uint32 guid) { return list_DarObjetosIniciales.find(guid) != list_DarObjetosIniciales.end(); };
    inline bool EstaEnListaQuitarTodosLosObjetos(uint32 guid) { return list_QuitarTodosLosObjetos.find(guid) != list_QuitarTodosLosObjetos.end(); };
    inline bool EstaLlenoElEvento() { return list_Jugadores.size() + list_Invitados.size() >= conf_JugadoresMaximo; };

    bool EstaEspectando(Player *player)
    {
        return HayJugadores() && EstaEnEvento(player) && list_Datos[player->GetGUID().GetCounter()].spect && EstaEnEvento(list_Datos[player->GetGUID().GetCounter()].spect) && list_Jugadores[list_Datos[player->GetGUID().GetCounter()].spect]->IsAlive();
    }

    inline bool HaySuficientesEnCola() { return list_Cola.size() >= conf_JugadoresMinimo; };

    void SiguientePosicion()
    {
        if (++indiceDeVariacion >= CANTIDAD_DE_VARIACIONES)
        {
            indiceDeVariacion = 0;
        }
    }

    void DejarGrupo(Player *player)
    {
        player->RemoveFromGroup();
        player->UninviteFromGroup();
    }

    void Desmontar(Player *player)
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
    }

    void AlReducirseLaZona()
    {
        int chestCount = 0;
        // if (escalaDeZona < CANTIDAD_DE_ZONAS && sBRMapasMgr->TieneZonasParaCofres(escalaDeZona))
        // {
        //     BR_UbicacionZona temp = sBRMapasMgr->ObtenerZonasParaCofres(escalaDeZona);
        //     if (temp.size())
        //     {
        //         for (BR_UbicacionZona::iterator it = temp.begin(); it != temp.end(); ++it)
        //         {
        //             int rnd = rand() % 100 + 1;
        //             if (rnd <= 35)
        //             {
        //                 if (sBRObjetosMgr->InvocarCofre(it->second))
        //                 {
        //                     chestCount++;
        //                 }
        //             }
        //         }
        //     }
        // }
        if (HayJugadores())
        {
            if (estadoActual >= ESTADO_BR_ZONA_EN_ESPERA && estadoActual < ESTADO_BR_BATALLA_TERMINADA)
            {
                int vivos = 0;
                int rndEfecto = rand() % 10 + 1;
                for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
                {
                    if (it->second && it->second->IsAlive())
                    {
                        switch (rndEfecto)
                        {
                        case 1:
                        case 2:
                        {
                            if (it->second->CastSpell(it->second, HECHIZO_ANTI_INVISIBLES, true) != SPELL_CAST_OK)
                            {
                                it->second->AddAura(HECHIZO_ANTI_INVISIBLES, it->second);
                            }
                            break;
                        }
                        case 3:
                        case 4:
                        {
                            it->second->AddAura(HECHIZO_ANTI_SANADORES, it->second);
                            break;
                        }
                        case 5:
                        case 6:
                        {
                            it->second->AddAura(HECHIZO_RASTRILLO_LENTO, it->second);
                            break;
                        }
                        case 7:
                        {
                            if (!sBRObjetosMgr->HechizoGuardian(HECHIZO_RAYO_DRAGON, it->second))
                            {
                                it->second->AddAura(HECHIZO_DESGARRO_ASESINO, it->second);
                            }
                            break;
                        }
                        case 8:
                        case 9:
                        {
                            if (!sBRObjetosMgr->HechizoGuardian(HECHIZO_RAYO_DRAGON_FUERTE, it->second))
                            {
                                it->second->AddAura(HECHIZO_RASTRILLO_LENTO, it->second);
                            }
                            break;
                        }
                        default:
                        {
                            if (it->second->CastSpell(it->second, HECHIZO_BENEFICIO_LIEBRE, true) != SPELL_CAST_OK)
                            {
                                it->second->AddAura(HECHIZO_BENEFICIO_LIEBRE, it->second);
                            }
                            break;
                        }
                        }
                        vivos++;
                    }
                }
                sBRChatMgr->AnunciarEfectoZona(list_Jugadores, vivos);
            }
            if (chestCount)
                sBRChatMgr->AnunciarConteoCofres(chestCount, list_Jugadores);
        }
    }

    void VerificarEspectadores()
    {
        if (HayJugadores())
        {
            Player *vivo = nullptr;
            for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
            {
                if (it->second && it->second->IsAlive())
                {
                    if (!vivo)
                    {
                        vivo = it->second;
                    }
                    if (EstaEspectando(it->second))
                    {
                        it->second->StopCastingBindSight();
                        list_Datos[it->second->GetGUID().GetCounter()].spect = 0;
                    }
                }
            }
            if (vivo && vivo->IsAlive())
            {
                TodosLosMuertosEspectarme(vivo);
            }
        }
    }

    void VerificarJugadoresEnNave()
    {
        if (HayJugadores())
        {
            BR_ListaDePersonajes::iterator it = list_Jugadores.begin();
            while (it != list_Jugadores.end())
            {
                if (!sBRObjetosMgr->EstaEnLaNave(it->second) || !it->second->IsAlive())
                {
                    uint32 guid = it->first;
                    ++it;
                    SalirDelEvento(guid);
                }
                else
                {
                    ++it;
                }
            }
        }
        if (!HayJugadores())
            FinalizarRonda(false);
    }

    void TodosLosMuertosEspectarme(Player *player)
    {
        if (HayJugadores() && player && player->IsAlive() && !EstaEspectando(player))
        {
            for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
            {
                if (it->second && it->second != player && !it->second->IsAlive() && !EstaEspectando(it->second))
                    EspectarJugador(it->second, player);
            }
        }
    }

    void EspectarJugador(Player *player, Player *target)
    {
        if (HayJugadores() && player && target && player != target && EstaEnEvento(player) && EstaEnEvento(target) && !player->IsAlive() && target->IsAlive() && !EstaEspectando(player) && player->GetExactDist(target) <= 666.0f)
        {
            list_Datos[player->GetGUID().GetCounter()].spect = target->GetGUID().GetCounter();
            player->CastSpell(target, 6277, true);
        }
    }

    void DarObjetosInicialesProgramado()
    {
        if (list_DarObjetosIniciales.size())
        {
            BR_ListaDePersonajes::iterator it = list_DarObjetosIniciales.begin();
            while (it != list_DarObjetosIniciales.end())
            {
                if (it->second && it->second->IsAlive())
                {
                    if (it->second->IsInWorld() && !it->second->IsBeingTeleported() && sBRObjetosMgr->EstaEnLaNave(it->second))
                    {
                        uint32 guid = it->first;
                        Player *player = it->second;
                        ++it;
                        sBREquipamientoMgr->Desnudar(player);
                        if (sBREquipamientoMgr->DarObjetosIniciales(player))
                        {
                            sBRChatMgr->AnunciarMensajeBienvenida(player);
                        }
                        else
                        {
                            sBRChatMgr->AnunciarErrorAlas(player);
                        }
                        player->GetMotionMaster()->MoveFall();
                        list_DarObjetosIniciales.erase(guid);
                    }
                    else
                    {
                        ++it;
                    }
                }
                else
                {
                    ++it;
                }
            }
        }
    }

    void QuitarTodosLosObjetosProgramado()
    {
        if (list_QuitarTodosLosObjetos.size())
        {
            BR_ListaDePersonajes::iterator it = list_QuitarTodosLosObjetos.begin();
            while (it != list_QuitarTodosLosObjetos.end())
            {
                if (it->second && it->second->IsAlive())
                {
                    if (it->second->IsInWorld() && !it->second->IsBeingTeleported())
                    {
                        uint32 guid = it->first;
                        Player *player = it->second;
                        ++it;
                        sBREquipamientoMgr->QuitarTodosLosObjetos(player);
                        list_QuitarTodosLosObjetos.erase(guid);
                    }
                    else
                    {
                        ++it;
                    }
                }
                else
                {
                    ++it;
                }
            }
        }
    }

    void TODO_MejorarAnuncioEnNave()
    {
        if (tiempoRestanteInicio % 5 == 0)
        {
            if (tiempoRestanteInicio == 45)
            {
                sBRSonidosMgr->ReproducirSonidoParaTodos(SONIDO_NAVE_EN_MOVIMIENTO, list_Jugadores);
            }
            sBRChatMgr->NotificarTiempoInicial(tiempoRestanteInicio, list_Jugadores);
        }
    }

    BR_ListaDePersonajes list_Cola;
    BR_ListaDePersonajes list_Invitados;

    BR_ListaDePersonajes list_Jugadores;
    BR_DatosDePersonajes list_Datos;

    BR_ListaDePersonajes list_DarObjetosIniciales;
    BR_ListaDePersonajes list_QuitarTodosLosObjetos;

    BREstado estadoActual;

    int tiempoRestanteSinJugadores;
    int tiempoRestanteInicio;
    int tiempoRestanteZona;
    int tiempoRestanteNave;
    int tiempoRestanteFinal;

    int indiceDeVariacion;
    float escalaDeZona;
    float reducirZonaHasta;
    uint32 totalAsesinatosJcJ;

    int indicadorDeSegundos;
    int indicadorDe100msZona;
    int indicadorQuitarObjetosProgramado;
    bool seHaAnunciadoInicioForzado;
    bool estaZonaAnunciada5s;
    bool estaZonaAnunciada10s;

    bool conf_EstaActivado;
    uint32 conf_JugadoresMinimo;
    uint32 conf_JugadoresMaximo;
    uint32 conf_IntervaloSinJugadores;
    uint32 conf_IntervaloZonaSegura;
    uint32 conf_IntervaloFinalDeRonda;
    uint32 conf_RequisitoAsesinatosTotales;
    uint32 conf_RequisitoAsesinatosPropios;
    BRConf_Recompensa conf_Recompensa;
};

#define sBattleRoyaleMgr BattleRoyaleMgr::instance()

#endif
