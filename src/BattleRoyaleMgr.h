#ifndef SC_BR_MGR_H
#define SC_BR_MGR_H

#include "BRConstantes.h"
#include "BRChatMgr.h"
#include "BRSonidosMgr.h"
#include "BRTitulosMgr.h"
#include "BattleRoyaleData.h"
#include "Common.h"
#include "SharedDefines.h"
#include "ScriptMgr.h"
#include "Chat.h"
#include "GameObject.h"
#include "Transport.h"

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
    bool DebeForzarJcJTcT(Player* player) 
    {
        if (!player) return false;
        if (estadoActual != ESTADO_BATALLA_EN_CURSO || !HayJugadores() || !EstaEnEvento(player)) return false;
        return !EstaEnLaNave(player);
    };
    void QuitarAlas(Player* player) { player->DestroyItemCount(17, 9999, true); };

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
    void SiguienteMapa() { if (++mapaActual == list_Mapas.end()) mapaActual = list_Mapas.begin(); };
    void SiguientePosicion() { if (++indiceDeVariacion >= CANTIDAD_DE_VARIACIONES) indiceDeVariacion = 0; };
    void DejarGrupo(Player* player)
    { 
        player->RemoveFromGroup(); 
        player->UninviteFromGroup();
    };
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
    void AlReducirseLaZona()
    {
        int chestCount = 0;
        if (indiceDeZona < CANTIDAD_DE_ZONAS && (*mapaActual).second->ubicacionesMapa.find(indiceDeZona) != (*mapaActual).second->ubicacionesMapa.end())
        {
            BR_UbicacionZona temp = (*mapaActual).second->ubicacionesMapa[indiceDeZona];
            for (BR_UbicacionZona::iterator it = temp.begin(); it != temp.end(); ++it)
            {
                int rnd = rand() % 100 + 1;
                if (rnd <= 35)
                {
                    obj_Centro->SummonGameObject(OBJETO_COFRE, it->second.GetPositionX(), it->second.GetPositionY(), it->second.GetPositionZ(), it->second.GetOrientation(), 0, 0, 0, 0, 60);
                    chestCount++;
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
                    if ((*it).second && (*it).second->IsAlive())
                    {
                        (*it).second->AddAura(HECHIZO_ANTI_INVISIBLES, (*it).second);
                        (*it).second->AddAura(HECHIZO_ANTI_SANADORES, (*it).second);
                        (*it).second->AddAura(HECHIZO_LENGUAJE_BINARIO, (*it).second);
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
                if ((*it).second && (*it).second->IsAlive())
                {
                    TodosLosMuertosEspectarme((*it).second);
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
                if (!EstaEnLaNave((*it).second) || !(*it).second->IsAlive())
                {
                    uint32 guid = (*it).first;
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
                if ((*it).second && (*it).second != player && !(*it).second->IsAlive() && !EstaEspectando((*it).second)) EspectarJugador((*it).second , player);
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
        InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 17, 1);
        if (msg == EQUIP_ERR_OK)
        {
            if (Item* item = player->StoreNewItem(dest, 17, true))
            {
                player->SendNewItem(item, 1, true, false);
                Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r Bienvenido a este modo de juego, se te han otorgado tus alas, con ellas podrás descender de manera segura durante la partida.");
                Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r Encuéntra la camisa en tu mochila y equípala. Puedes arrastrarla a la barra de acción para facilitar su uso o activarla con clic derecho en el inventario.");
                Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r |cffff0000Recuerda permanecer en la NAVE hasta que se anuncie que puedes saltar o serás descalificado y expulsado.|r");
            }
            else
            {
                Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡No has obtenido las alas porque no se ha podido crear el objeto! |cffff0000¡Descansa en paz! :(|r");
            }
        }
        else
        {
            Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡No has obtenido las alas porque no tienes espacio disponible! |cffff0000¡Descansa en paz! :(|r");
        }
    };
    void DarAlasProgramado()
    {
        if (list_DarAlas.size())
        {
            BR_ListaDePersonajes::iterator it = list_DarAlas.begin();
            while (it != list_DarAlas.end())
            {
                if ((*it).second && (*it).second->IsAlive())
                {
                    if ((*it).second->IsInWorld() && !(*it).second->IsBeingTeleported() && EstaEnLaNave((*it).second))
                    {
                        uint32 guid = (*it).first;
                        Player* player = (*it).second;
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
                if ((*it).second && (*it).second->IsAlive())
                {
                    if ((*it).second->IsInWorld() && !(*it).second->IsBeingTeleported())
                    {
                        uint32 guid = (*it).first;
                        Player* player = (*it).second;
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

    GameObject* obj_Zona;
    GameObject* obj_Centro;
    GameObject* obj_Nave;

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
    bool estaLaZonaActiva;

    uint32 conf_JugadoresMinimo;
    uint32 conf_JugadoresMaximo;
    uint32 conf_IntervaloDeZona;

};

#define sBattleRoyaleMgr BattleRoyaleMgr::instance()

#endif
