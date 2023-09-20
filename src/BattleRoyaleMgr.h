#ifndef SC_BR_MGR_H
#define SC_BR_MGR_H

#include "BRConstantes.h"
#include "BRChatMgr.h"
#include "BRListaNegraMgr.h"
#include "BRObjetosMgr.h"
#include "BRSonidosMgr.h"
#include "BRTitulosMgr.h"
#include "BattleRoyaleData.h"
#include "Common.h"
#include "SharedDefines.h"
#include "ScriptMgr.h"
#include "Chat.h"

class BattleRoyaleData;
typedef std::map<uint32, Player*> BR_ListaDePersonajes;
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
    
    void GestionarJugadorEntrando(Player* player);
    void GestionarJugadorDesconectar(Player* player);
    void GestionarMuerteJcJ(Player* killer, Player* killed);
    void GestionarActualizacionMundo(uint32 diff);
    void PrevenirJcJEnLaNave(Player* player, bool state);
    bool PuedeReaparecerEnCementerio(Player* player);
    bool DebeRestringirFunciones(Player* player) { return estadoActual > ESTADO_NO_HAY_SUFICIENTES_JUGADORES && HayJugadores() && EstaEnEvento(player); };
    bool EstaEnCola(Player* player) { return EstaEnCola(player->GetGUID().GetCounter()); };
    bool EstaEnEvento(Player* player) { return EstaEnEvento(player->GetGUID().GetCounter()); };
    bool DebeForzarJcJTcT(Player* player) 
    {
        if (!player) return false;
        if (estadoActual != ESTADO_BATALLA_EN_CURSO || !HayJugadores() || !EstaEnEvento(player)) return false;
        return !sBRObjetosMgr->EstaEnLaNave(player);
    };
    void QuitarAlas(Player* player) { player->DestroyItemCount(INVENTARIO_CAMISA_ALAS, 9999, true); };

private:
    void RestablecerTodoElEvento();
    void IniciarNuevaRonda();
    void AlmacenarPosicionInicial(uint32 guid);
    void LlamarDentroDeNave(uint32 guid);
    void SalirDelEvento(uint32 guid, bool logout = false);
    void RevivirJugador(Player *player);
    void EfectoFueraDeZona();
    void ActivarJcJTcT();
    void ControlDeReglas();
    bool CondicionDeVictoria();
    void FinalizarRonda(bool announce, Player* winner = nullptr);
    bool HayJugadores() { return !list_Jugadores.empty(); };
    bool HayCola() { return !list_Cola.empty(); };
    bool EstaEnCola(uint32 guid) { return list_Cola.find(guid) != list_Cola.end(); };
    bool EstaEnEvento(uint32 guid) { return list_Jugadores.find(guid) != list_Jugadores.end(); };
    bool EstaEnListaDeAlas(Player* player) { return EstaEnListaDeAlas(player->GetGUID().GetCounter()); };
    bool EstaEnListaDeAlas(uint32 guid) { return EstaEnListaDarAlas(guid) || EstaEnListaQuitarAlas(guid); };
    bool EstaEnListaDarAlas(uint32 guid) { return list_DarAlas.find(guid) != list_DarAlas.end(); };
    bool EstaEnListaQuitarAlas(uint32 guid) { return list_QuitarAlas.find(guid) != list_QuitarAlas.end(); };
    bool EstaLlenoElEvento() { return list_Jugadores.size() >= conf_JugadoresMaximo; };
    bool EstaEspectando(Player* player)
    { 
        return HayJugadores() && EstaEnEvento(player) && list_Datos[player->GetGUID().GetCounter()].spect 
            && EstaEnEvento(list_Datos[player->GetGUID().GetCounter()].spect) && list_Jugadores[list_Datos[player->GetGUID().GetCounter()].spect]->IsAlive();
    };
    bool HaySuficientesEnCola() { return list_Cola.size() >= conf_JugadoresMinimo; };
    void SiguientePosicion() { if (++indiceDeVariacion >= CANTIDAD_DE_VARIACIONES) indiceDeVariacion = 0; };
    void SiguienteMapa()
    {
        // if (++mapaActual == list_Mapas.end()) mapaActual = list_Mapas.begin();
        int num = list_Mapas.size();
        if (num <= 1)
        {
            mapaActual = list_Mapas.begin();
        }
        else
        {
            int rnd = rand() % num;
            mapaActual = list_Mapas.begin();
            int temp = 0;
            while (++temp <= rnd) mapaActual++;
        }
    };
    void DejarGrupo(Player* player)
    { 
        player->RemoveFromGroup(); 
        player->UninviteFromGroup();
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
    void AlReducirseLaZona()
    {
        int chestCount = 0;
        if (indiceDeZona < CANTIDAD_DE_ZONAS && mapaActual->second->ubicacionesMapa.find(indiceDeZona) != mapaActual->second->ubicacionesMapa.end())
        {
            BR_UbicacionZona temp = mapaActual->second->ubicacionesMapa[indiceDeZona];
            for (BR_UbicacionZona::iterator it = temp.begin(); it != temp.end(); ++it)
            {
                int rnd = rand() % 100 + 1;
                if (rnd <= 35)
                {
                    if (sBRObjetosMgr->InvocarCofre(it->second))
                    {
                        chestCount++;
                    }
                }
            }
        }
        if (HayJugadores())
        {
            if (estadoActual == ESTADO_BATALLA_EN_CURSO)
            {
                int vivos = 0;
                for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
                {
                    if (it->second && it->second->IsAlive())
                    {
                        it->second->AddAura(HECHIZO_ANTI_INVISIBLES, it->second);
                        it->second->AddAura(HECHIZO_ANTI_SANADORES, it->second);
                        // TODO: Analizar si esto es factible: it->second->AddAura(HECHIZO_LENGUAJE_BINARIO, it->second);
                        vivos++;
                    }
                }
                sBRChatMgr->AnunciarEfectoZona(list_Jugadores, vivos);
            }
            if (chestCount) sBRChatMgr->AnunciarConteoCofres(chestCount, list_Jugadores);
        }
    };
    void VerificarEspectadores()
    {
        if (HayJugadores())
        {
            for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
            {
                if (it->second && it->second->IsAlive())
                {
                    TodosLosMuertosEspectarme(it->second);
                }
            }
        }
    };
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
        if (!HayJugadores()) FinalizarRonda(false);
    };
    void TodosLosMuertosEspectarme(Player* player)
    {
        if (HayJugadores() && player && player->IsAlive() && !EstaEspectando(player))
        {
            for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
            {
                if (it->second && it->second != player && !it->second->IsAlive() && !EstaEspectando(it->second)) EspectarJugador(it->second , player);
            }
        }
    };
    void EspectarJugador(Player* player, Player* target)
    {
        if (HayJugadores() && player && target && player != target && EstaEnEvento(player) && EstaEnEvento(target) && !player->IsAlive() && target->IsAlive() && !EstaEspectando(player) && player->GetExactDist(target) <= 666.0f)
        {
            list_Datos[player->GetGUID().GetCounter()].spect = target->GetGUID().GetCounter();
            player->CastSpell(target, 6277, true);
        }
    };
    void DarAlas(Player* player)
    {
        QuitarAlas(player);
        ItemPosCountVec dest;
        InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, INVENTARIO_CAMISA_ALAS, 1);
        if (msg == EQUIP_ERR_OK)
        {
            if (Item* item = player->StoreNewItem(dest, INVENTARIO_CAMISA_ALAS, true))
            {
                player->SendNewItem(item, 1, true, false);
                sBRChatMgr->AnunciarMensajeBienvenida(player);
            }
            else
            {
                sBRChatMgr->AnunciarErrorAlas(player);
            }
        }
        else
        {
            sBRChatMgr->AnunciarErrorAlas(player, true);
        }
    };
    void DarAlasProgramado()
    {
        if (list_DarAlas.size())
        {
            BR_ListaDePersonajes::iterator it = list_DarAlas.begin();
            while (it != list_DarAlas.end())
            {
                if (it->second && it->second->IsAlive())
                {
                    if (it->second->IsInWorld() && !it->second->IsBeingTeleported() && sBRObjetosMgr->EstaEnLaNave(it->second))
                    {
                        uint32 guid = it->first;
                        Player* player = it->second;
                        ++it;
                        DarAlas(player);
                        player->GetMotionMaster()->MoveFall();
                        list_DarAlas.erase(guid);
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
    void QuitarAlasProgramado()
    {
        if (list_QuitarAlas.size())
        {
            BR_ListaDePersonajes::iterator it = list_QuitarAlas.begin();
            while (it != list_QuitarAlas.end())
            {
                if (it->second && it->second->IsAlive())
                {
                    if (it->second->IsInWorld() && !it->second->IsBeingTeleported())
                    {
                        uint32 guid = it->first;
                        Player* player = it->second;
                        ++it;
                        QuitarAlas(player);
                        list_QuitarAlas.erase(guid);
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

    BR_ListaDePersonajes list_Cola;

    BR_ListaDePersonajes list_Jugadores;
    BR_DatosDePersonajes list_Datos;

    BR_ListaDePersonajes list_DarAlas;
    BR_ListaDePersonajes list_QuitarAlas;

    BR_ContenedorMapas list_Mapas;
    BR_ContenedorMapas::iterator mapaActual;

    int estadoActual;
    int tiempoRestanteInicio;
    int tiempoRestanteZona;
    int tiempoRestanteNave;
    int tiempoRestanteFinal;
    int indiceDeVariacion;

    int indiceDeZona;
    int indicadorDeSegundos;
    bool estaZonaAnunciada5s;
    bool estaZonaAnunciada10s;

    uint32 conf_JugadoresMinimo;
    uint32 conf_JugadoresMaximo;
    uint32 conf_IntervaloDeZona;

};

#define sBattleRoyaleMgr BattleRoyaleMgr::instance()

#endif
