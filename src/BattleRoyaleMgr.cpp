#include "BattleRoyaleMgr.h"
#include "Config.h"
#include "Chat.h"
#include "Player.h"

BattleRoyaleMgr::BattleRoyaleMgr()
{
    conf_JugadoresMinimo = sConfigMgr->GetOption<int32>("BattleRoyale.MinPlayers", 25);
    conf_JugadoresMaximo = sConfigMgr->GetOption<int32>("BattleRoyale.MaxPlayers", 50);
    conf_IntervaloEntreRecuccionDeZona = sConfigMgr->GetOption<int32>("BattleRoyale.SecureZoneInterval", 60000);
    obj_Zona = nullptr;
    obj_Centro = nullptr;
    obj_Nave = nullptr;
    RestablecerTodoElEvento();
}

BattleRoyaleMgr::~BattleRoyaleMgr()
{
    RestablecerTodoElEvento();
}

// --- PUBLICO --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void BattleRoyaleMgr::GestionarJugadorEntrando(Player* player)
{
    if (EstaEnCola(player))
    {
        Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r Ya estas en cola para el evento.");
        return;
    }
    if (EstaEnEvento(player))
    {
        Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r Ya estas dentro del evento.");
        return;
    }
    switch (estadoActual)
    {
        case ESTADO_NO_HAY_SUFICIENTES_JUGADORES:
        {
            list_Cola[player->GetGUID().GetCounter()] = player;
            if (HaySuficientesEnCola()) {
                IniciarNuevaRonda();
            }
            else
            {
                Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r Te has unido a la cola del evento. Jugadores en cola: %u/%u.", list_Cola.size(), conf_JugadoresMinimo);
            }
            break;
        }
        case ESTADO_NAVE_EN_ESPERA:
        {
            if (EstaLlenoElEvento()) {
                list_Cola[player->GetGUID().GetCounter()] = player;
                Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r Te has unido a la cola del evento. Jugadores en cola: %u/%u. El evento esta lleno, espera la otra ronda.", list_Cola.size(), conf_JugadoresMinimo);
            }
            else
            {
                list_Jugadores[player->GetGUID().GetCounter()] = player;
                AlmacenarPosicionInicial(player->GetGUID().GetCounter());
                LlamarDentroDeNave(player->GetGUID().GetCounter());
            }
            break;
        }
        default:
        {
            list_Cola[player->GetGUID().GetCounter()] = player;
            Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r Te has unido a la cola del evento. Jugadores en cola: %u/%u. El evento esta en curso, espera la otra ronda.", list_Cola.size(), conf_JugadoresMinimo);
            break;
        }
    }
}

void BattleRoyaleMgr::GestionarJugadorDesconectar(Player* player)
{
    if (EstaEnEvento(player)) SalirDelEvento(player->GetGUID().GetCounter(), true);
}

void BattleRoyaleMgr::GestionarMuerteJcJ(Player* killer, Player* killed)
{
    if (!killer || !killed) return;
    if (!list_Jugadores.size() || estadoActual != ESTADO_BATALLA_EN_CURSO) return;
    if (!EstaEnEvento(killer) || !EstaEnEvento(killed)) return;
    list_Datos[killer->GetGUID().GetCounter()].kills++;
    killed->CastSpell(killer, 6277, true);
    NotificarMuerteJcJ(ChatHandler(killer->GetSession()).GetNameLink(killer), ChatHandler(killed->GetSession()).GetNameLink(killed), list_Datos[killer->GetGUID().GetCounter()].kills);
}

void BattleRoyaleMgr::GestionarActualizacionMundo(uint32 diff)
{
    switch(estadoActual)
    {
        case ESTADO_INVOCANDO_JUGADORES:
        case ESTADO_NAVE_EN_ESPERA:
        case ESTADO_NAVE_EN_MOVIMIENTO:
        case ESTADO_NAVE_CERCA_DEL_CENTRO:
        {
            if (indicadorDeSegundos <= 0) {
                indicadorDeSegundos = 1000;
                if (tiempoRestanteSeg <= 0) {
                    estadoActual = ESTADO_BATALLA_EN_CURSO;
                    NotificarTiempoParaIniciar(0);
                    tiempoRestanteZona = conf_IntervaloEntreRecuccionDeZona;
                } else {
                    if (tiempoRestanteSeg % 5 == 0) {
                        NotificarTiempoParaIniciar(tiempoRestanteSeg);
                    }
                    if (estadoActual == ESTADO_INVOCANDO_JUGADORES && tiempoRestanteSeg <= 60)
                    {
                        estadoActual = ESTADO_NAVE_EN_ESPERA;
                        if (!InvocarNave()) {
                            RestablecerTodoElEvento();
                            return;
                        }
                        LlamarTodosDentroDeNave();
                    }
                    if (estadoActual == ESTADO_NAVE_EN_ESPERA && tiempoRestanteSeg <= 30 && obj_Nave)
                    {
                        estadoActual = ESTADO_NAVE_EN_MOVIMIENTO;
                        uint32_t const autoCloseTime = obj_Nave->GetGOInfo()->GetAutoCloseTime() ? 10000u : 0u;
                        obj_Nave->SetLootState(GO_READY);
                        obj_Nave->UseDoorOrButton(autoCloseTime, false, nullptr);
                    }
                    if (estadoActual == ESTADO_NAVE_EN_MOVIMIENTO && tiempoRestanteSeg <= 5)
                    {
                        estadoActual = ESTADO_NAVE_CERCA_DEL_CENTRO;
                        indiceDeZona = 0;
                        tiempoRestanteZona = 0;
                        estaLaZonaAnunciada = false;
                        obj_Centro = nullptr;
                        obj_Zona = nullptr;
                        if (!InvocarCentroDelMapa()) {
                            RestablecerTodoElEvento();
                            return;
                        }
                        if (!InvocarZonaSegura()) {
                            RestablecerTodoElEvento();
                            return;
                        }
                        PonerTodosLosParacaidas();
                    }
                    tiempoRestanteSeg--;
                }
            } else {
                indicadorDeSegundos -= diff;
            }
            break;
        }
        case ESTADO_BATALLA_EN_CURSO:
        {
            if (indicadorDeSegundos <= 0) {
                indicadorDeSegundos = 1000;
                EfectoFueraDeZona();
                ActivarJcJTcT();
            } else {
                indicadorDeSegundos -= diff;
            }
            if (tiempoRestanteZona <= 0) {
                InvocarZonaSegura();
                NotificarZonaReducida();
                tiempoRestanteZona = conf_IntervaloEntreRecuccionDeZona;
                estaLaZonaAnunciada = false;
            } else {
                if (tiempoRestanteZona <= 5000 && !estaLaZonaAnunciada) {
                    NotificarAdvertenciaDeZona(5);
                    estaLaZonaAnunciada = true;
                }
                if (indiceDeZona <= CANTIDAD_DE_ZONAS) {
                    tiempoRestanteZona -= diff;
                }
            }
            break;
        }
    }
}

void BattleRoyaleMgr::PrevenirJcJEnLaNave(Player* player, bool state)
{
    if (!state || !list_Jugadores.size()) return;
    if (list_Jugadores.find(player->GetGUID().GetCounter()) != list_Jugadores.end() && !DebeForzarJcJTcT(player)) player->SetPvP(false);
}

bool BattleRoyaleMgr::PuedeReaparecerEnCementerio(Player *player)
{
    if (HayJugadores() && EstaEnEvento(player)) {
        if (!player->IsAlive()) RevivirJugador(player);
        SalirDelEvento(player->GetGUID().GetCounter());
    }
    return true;
}

// --- PRIVADO --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void BattleRoyaleMgr::RestablecerTodoElEvento()
{
    list_Cola.clear();
	list_Jugadores.clear();
    list_Datos.clear();
    indiceDelMapa = 0;
    estadoActual = ESTADO_NO_HAY_SUFICIENTES_JUGADORES;
    indicadorDeSegundos = 1000;
    indiceDeVariacion = 0;
    if (obj_Zona) {
        obj_Zona->DespawnOrUnsummon();
        obj_Zona->Delete();
        obj_Zona = nullptr;
    }
    if (obj_Centro) {
        obj_Centro->DespawnOrUnsummon();
        obj_Centro->Delete();
        obj_Centro = nullptr;
    }
    if (obj_Nave) {
        obj_Nave->DespawnOrUnsummon();
        obj_Nave->Delete();
        obj_Nave = nullptr;
    }
}

void BattleRoyaleMgr::IniciarNuevaRonda()
{
    if (estadoActual == ESTADO_NO_HAY_SUFICIENTES_JUGADORES)
    {
        estadoActual = ESTADO_INVOCANDO_JUGADORES;
        SiguienteMapa();
        tiempoRestanteSeg = 70;
        for (BR_ColaDePersonajes::iterator it = list_Cola.begin(); it != list_Cola.end(); ++it)
	    {
            if (!EstaLlenoElEvento())
            {
                list_Jugadores[(*it).first] = (*it).second;
                AlmacenarPosicionInicial((*it).first);
                LlamarAntesQueNave((*it).first);
            }            
	    }
        if (HayJugadores()) for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it) list_Cola.erase((*it).first);
    }
}

void BattleRoyaleMgr::AlmacenarPosicionInicial(uint32 guid)
{
    if (list_Jugadores[guid]->GetMap()->Instanceable())
    {
        list_Datos[guid].SetPosition(list_Jugadores[guid]->m_homebindMapId, list_Jugadores[guid]->m_homebindX, list_Jugadores[guid]->m_homebindY, list_Jugadores[guid]->m_homebindZ, list_Jugadores[guid]->GetOrientation());
    } 
    else
    {
        list_Datos[guid].SetPosition(list_Jugadores[guid]->GetMapId(), list_Jugadores[guid]->GetPositionX(), list_Jugadores[guid]->GetPositionY(), list_Jugadores[guid]->GetPositionZ(), list_Jugadores[guid]->GetOrientation());
    }
}

void BattleRoyaleMgr::LlamarAntesQueNave(uint32 guid)
{
    CambiarDimension_Entrar(guid);
    float ox = BR_VariacionesDePosicion[indiceDeVariacion][0];
    float oy = BR_VariacionesDePosicion[indiceDeVariacion][1];
    SiguientePosicion();
    Desmontar(list_Jugadores[guid]);
    list_Jugadores[guid]->SetPvP(false);
    list_Jugadores[guid]->TeleportTo(BR_IdentificadorDeMapas[indiceDelMapa], BR_InicioDeLaNave[indiceDelMapa][0] + ox, BR_InicioDeLaNave[indiceDelMapa][1] + oy, BR_InicioDeLaNave[indiceDelMapa][2] + 15.0f, 0.0f);
    list_Jugadores[guid]->SaveToDB(false, false);
    list_Jugadores[guid]->AddAura(HECHIZO_PARACAIDAS, list_Jugadores[guid]);
}

void BattleRoyaleMgr::LlamarDentroDeNave(uint32 guid)
{
    CambiarDimension_Entrar(guid);
    float ox = BR_VariacionesDePosicion[indiceDeVariacion][0];
    float oy = BR_VariacionesDePosicion[indiceDeVariacion][1];
    SiguientePosicion();
    Desmontar(list_Jugadores[guid]);
    list_Jugadores[guid]->SetPvP(false);
    list_Jugadores[guid]->TeleportTo(BR_IdentificadorDeMapas[indiceDelMapa], BR_InicioDeLaNave[indiceDelMapa][0] + ox, BR_InicioDeLaNave[indiceDelMapa][1] + oy, BR_InicioDeLaNave[indiceDelMapa][2] + 1.5f, 0.0f);
    list_Jugadores[guid]->SaveToDB(false, false);
}

void BattleRoyaleMgr::LlamarTodosDentroDeNave()
{
    if (HayJugadores())
    {
        for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
        {
            LlamarDentroDeNave((*it).first);
        }
    }
}

void BattleRoyaleMgr::SalirDelEvento(uint32 guid, bool logout)
{
    if (list_Cola.find(guid) != list_Cola.end()) {
        list_Cola.erase(guid);
    };
    if (list_Jugadores.find(guid) != list_Jugadores.end()) {
        CambiarDimension_Salir(guid);
        if(!logout)
        {
            if (!list_Jugadores[guid]->isPossessing()) list_Jugadores[guid]->StopCastingBindSight();
            list_Jugadores[guid]->TeleportTo(list_Datos[guid].GetMap(), list_Datos[guid].GetX(), list_Datos[guid].GetY(), list_Datos[guid].GetZ(), list_Datos[guid].GetO());
            list_Jugadores[guid]->SaveToDB(false, false);
        }
        list_Jugadores.erase(guid);
        list_Datos.erase(guid);
        
    }
}

void BattleRoyaleMgr::RevivirJugador(Player* player)
{
	player->ResurrectPlayer(1.0f);
    player->SpawnCorpseBones();
    player->SaveToDB(false, false);
}

bool BattleRoyaleMgr::InvocarNave()
{
    bool success = false;
    if (HayJugadores())
    {
        for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
        {
            if ((*it).second)
            {
                if (obj_Nave) {
                    obj_Nave->DespawnOrUnsummon();
                    obj_Nave->Delete();
                    obj_Nave = nullptr;
                }
                float x = BR_InicioDeLaNave[indiceDelMapa][0];
                float y = BR_InicioDeLaNave[indiceDelMapa][1];
                float z = BR_InicioDeLaNave[indiceDelMapa][2];
                float o = BR_InicioDeLaNave[indiceDelMapa][3];
                float rot2 = std::sin(o / 2);
                float rot3 = cos(o / 2);
                obj_Nave = (*it).second->SummonGameObject(OBJETO_NAVE, x, y, z, o, 0, 0, rot2, rot3, 2 * 60);
                success = true;
                break;
            }
        }
    }
    return success;
}

bool BattleRoyaleMgr::InvocarCentroDelMapa()
{
    if (obj_Nave)
    {
        if (obj_Centro) {
            obj_Centro->DespawnOrUnsummon();
            obj_Centro->Delete();
            obj_Centro = nullptr;
        }
        obj_Centro = obj_Nave->SummonGameObject(OBJETO_CENTRO_DEL_MAPA, BR_CentroDeMapas[indiceDelMapa].GetPositionX(), BR_CentroDeMapas[indiceDelMapa].GetPositionY(), BR_CentroDeMapas[indiceDelMapa].GetPositionZ(), 0, 0, 0, 0, 0, 15 * 60);
        return true;
    }
    return false;
}

bool BattleRoyaleMgr::InvocarZonaSegura()
{
    if (obj_Centro)
    {
        if (obj_Zona) {
            obj_Zona->DespawnOrUnsummon();
            obj_Zona->Delete();
            obj_Zona = nullptr;
        }
        if (indiceDeZona < CANTIDAD_DE_ZONAS) {
            obj_Zona = obj_Centro->SummonGameObject(OBJETO_ZONA_SEGURA_INICIAL + indiceDeZona, BR_CentroDeMapas[indiceDelMapa].GetPositionX(), BR_CentroDeMapas[indiceDelMapa].GetPositionY(), BR_CentroDeMapas[indiceDelMapa].GetPositionZ() + BR_EscalasDeZonaSegura[indiceDeZona] * 66.0f, 0, 0, 0, 0, 0, 2 * 60);
            obj_Zona->SetPhaseMask(2, true);
            obj_Zona->SetVisibilityDistanceOverride(VisibilityDistanceType::Infinite);
        }
        indiceDeZona++;
        return true;
    }
    return false;
}

void BattleRoyaleMgr::PonerTodosLosParacaidas()
{
    if (list_Jugadores.size())
    {
        for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
        {
            if (obj_Nave && (*it).second)
            {
                if ((*it).second->GetTransport() && (*it).second->GetExactDist(obj_Nave) < 25.0f && (*it).second->IsAlive())
                {
                    (*it).second->AddAura(HECHIZO_PARACAIDAS, (*it).second);
                }
                else
                {
                    SalirDelEvento((*it).first);
                }
            }
        }
    }
}

void BattleRoyaleMgr::EfectoFueraDeZona()
{
    if (list_Jugadores.size())
    {
        for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
        {
            if ((*it).second && (*it).second->IsAlive())
            {
                float distance = (*it).second->GetExactDist(obj_Centro);
                if (indiceDeZona > 0 && distance > BR_EscalasDeZonaSegura[indiceDeZona - 1] * 66.0f) {
                    list_Datos[(*it).first].dmg_tick++;
                    uint32 damage = (*it).second->GetMaxHealth() * (2 * sqrt(list_Datos[(*it).first].dmg_tick) + indiceDeZona) / 100;
                    (*it).second->GetSession()->SendNotification("|cffff0000¡Has recibido |cffDA70D6%u|cffff0000 de daño, adéntrate en la zona segura!", damage);
                    Unit::DealDamage(nullptr, (*it).second, damage, nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false, true);
                } else {
                    list_Datos[(*it).first].dmg_tick = 0;
                }
            }
        }
    }
}

void BattleRoyaleMgr::ActivarJcJTcT()
{
    if (list_Jugadores.size())
    {
        for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
        {
            if ((*it).second && (*it).second->IsAlive() && DebeForzarJcJTcT((*it).second) && !((*it).second->HasByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP)))
            {
                (*it).second->SetByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP);
            }
        }
    }
}
