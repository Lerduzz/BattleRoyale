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
                Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r Te has unido a la cola del evento. Jugadores en cola: |cff4CFF00%u|r/|cff4CFF00%u|r.", list_Cola.size(), conf_JugadoresMinimo);
            }
            break;
        }
        case ESTADO_NAVE_EN_ESPERA:
        {
            if (EstaLlenoElEvento()) {
                list_Cola[player->GetGUID().GetCounter()] = player;
                Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r Te has unido a la cola del evento. Jugadores en cola: |cff4CFF00%u|r/|cff4CFF00%u|r. Evento lleno, espera a que termine la ronda.", list_Cola.size(), conf_JugadoresMinimo);
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
            Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r Te has unido a la cola del evento. Jugadores en cola: |cff4CFF00%u|r/|cff4CFF00%u|r. Evento en curso, espera a que termine la ronda.", list_Cola.size(), conf_JugadoresMinimo);
            break;
        }
    }
}

void BattleRoyaleMgr::GestionarJugadorDesconectar(Player* player)
{
    if (EstaEnEvento(player) || EstaEnCola(player)) SalirDelEvento(player->GetGUID().GetCounter(), true);
}

void BattleRoyaleMgr::GestionarMuerteJcJ(Player* killer, Player* killed)
{
    if (!killer || !killed || !HayJugadores() || estadoActual != ESTADO_BATALLA_EN_CURSO || killer == killed || !EstaEnEvento(killer) || !EstaEnEvento(killed)) return;
    list_Datos[killer->GetGUID().GetCounter()].kills++;
    killed->CastSpell(killer, 6277, true);
    NotificarMuerteJcJ(Chat(killer).GetNameLink(killer), Chat(killed).GetNameLink(killed), list_Datos[killer->GetGUID().GetCounter()].kills);
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
                if (tiempoRestanteInicio <= 0) {
                    estadoActual = ESTADO_BATALLA_EN_CURSO;
                    NotificarTiempoParaIniciar(0);
                    tiempoRestanteZona = conf_IntervaloEntreRecuccionDeZona;
                    tiempoRestanteNave = 30;
                } else {
                    if (tiempoRestanteInicio % 5 == 0) {
                        NotificarTiempoParaIniciar(tiempoRestanteInicio);
                    }
                    if (estadoActual == ESTADO_INVOCANDO_JUGADORES && tiempoRestanteInicio <= 60)
                    {
                        estadoActual = ESTADO_NAVE_EN_ESPERA;
                        if (!InvocarNave()) {
                            RestablecerTodoElEvento();
                            return;
                        }
                        LlamarTodosDentroDeNave();
                    }
                    if (estadoActual == ESTADO_NAVE_EN_ESPERA && tiempoRestanteInicio <= 30 && obj_Nave)
                    {
                        estadoActual = ESTADO_NAVE_EN_MOVIMIENTO;
                        uint32_t const autoCloseTime = obj_Nave->GetGOInfo()->GetAutoCloseTime() ? 10000u : 0u;
                        obj_Nave->SetLootState(GO_READY);
                        obj_Nave->UseDoorOrButton(autoCloseTime, false, nullptr);
                    }
                    if (estadoActual == ESTADO_NAVE_EN_MOVIMIENTO && tiempoRestanteInicio <= 5)
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
                    tiempoRestanteInicio--;
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
                ControlDeReglas();
                CondicionDeVictoria();
                EfectoFueraDeZona();
                ActivarJcJTcT();
                if (--tiempoRestanteNave <= 0)
                {
                    if (DesaparecerNave()) NotificarNaveRetirada();
                }
            }
            else
            {
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
    if (state && HayJugadores() && EstaEnEvento(player) && !DebeForzarJcJTcT(player)) player->SetPvP(false);
}

bool BattleRoyaleMgr::PuedeReaparecerEnCementerio(Player *player)
{
    if (HayJugadores() && EstaEnEvento(player)) {
        SalirDelEvento(player->GetGUID().GetCounter());
        return false;
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
    DesaparecerTodosLosObjetos();
}

void BattleRoyaleMgr::IniciarNuevaRonda()
{
    if (estadoActual == ESTADO_NO_HAY_SUFICIENTES_JUGADORES)
    {
        estadoActual = ESTADO_INVOCANDO_JUGADORES;
        SiguienteMapa();
        tiempoRestanteInicio = 67;
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
    SalirDeGrupo(list_Jugadores[guid]);
    float ox = BR_VariacionesDePosicion[indiceDeVariacion][0];
    float oy = BR_VariacionesDePosicion[indiceDeVariacion][1];
    SiguientePosicion();
    Desmontar(list_Jugadores[guid]);
    list_Jugadores[guid]->TeleportTo(BR_IdentificadorDeMapas[indiceDelMapa], BR_InicioDeLaNave[indiceDelMapa][0] + ox, BR_InicioDeLaNave[indiceDelMapa][1] + oy, BR_InicioDeLaNave[indiceDelMapa][2] + 15.0f, 0.0f);
    list_Jugadores[guid]->SetPvP(false);
    list_Jugadores[guid]->SaveToDB(false, false);
    list_Jugadores[guid]->AddAura(HECHIZO_PARACAIDAS, list_Jugadores[guid]);
}

void BattleRoyaleMgr::LlamarDentroDeNave(uint32 guid)
{
    CambiarDimension_Entrar(guid);
    SalirDeGrupo(list_Jugadores[guid]);
    float ox = BR_VariacionesDePosicion[indiceDeVariacion][0];
    float oy = BR_VariacionesDePosicion[indiceDeVariacion][1];
    SiguientePosicion();
    Desmontar(list_Jugadores[guid]);
    list_Jugadores[guid]->TeleportTo(BR_IdentificadorDeMapas[indiceDelMapa], BR_InicioDeLaNave[indiceDelMapa][0] + ox, BR_InicioDeLaNave[indiceDelMapa][1] + oy, BR_InicioDeLaNave[indiceDelMapa][2] + 1.5f, 0.0f);
    list_Jugadores[guid]->SetPvP(false);
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
    if (EstaEnCola(guid))
    {
        list_Cola.erase(guid);
    };
    if (EstaEnEvento(guid))
    {
        CambiarDimension_Salir(guid);
        if(!logout)
        {
            if (!list_Jugadores[guid]->IsAlive()) RevivirJugador(list_Jugadores[guid]);
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
                DesaparecerNave();
                float x = BR_InicioDeLaNave[indiceDelMapa][0];
                float y = BR_InicioDeLaNave[indiceDelMapa][1];
                float z = BR_InicioDeLaNave[indiceDelMapa][2];
                float o = BR_InicioDeLaNave[indiceDelMapa][3];
                float rot2 = std::sin(o / 2);
                float rot3 = cos(o / 2);
                obj_Nave = (*it).second->SummonGameObject(OBJETO_NAVE, x, y, z, o, 0, 0, rot2, rot3, 2 * 60);
                obj_Nave->SetPhaseMask(2, true);
                obj_Nave->SetVisibilityDistanceOverride(VisibilityDistanceType::Infinite);
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
        DesaparecerCentro();
        obj_Centro = obj_Nave->SummonGameObject(OBJETO_CENTRO_DEL_MAPA, BR_CentroDeMapas[indiceDelMapa].GetPositionX(), BR_CentroDeMapas[indiceDelMapa].GetPositionY(), BR_CentroDeMapas[indiceDelMapa].GetPositionZ(), 0, 0, 0, 0, 0, 15 * 60);
        obj_Centro->SetPhaseMask(2, true);
        obj_Centro->SetVisibilityDistanceOverride(VisibilityDistanceType::Infinite);
        return true;
    }
    return false;
}

bool BattleRoyaleMgr::InvocarZonaSegura()
{
    if (obj_Centro)
    {
        DesaparecerZona();
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
    if (HayJugadores())
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
    if (HayJugadores())
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
    if (HayJugadores())
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

void BattleRoyaleMgr::ControlDeReglas()
{
    if(HayJugadores())
    {
        for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
        {
            if ((*it).second && (*it).second->IsAlive())
            {
                if ((*it).second->HasAura(31700)) SalirDelEvento((*it).first);
                if (obj_Centro && (*it).second->GetExactDist(obj_Centro) > 650.0f) SalirDelEvento((*it).first);
            }
        }
    }
}

void BattleRoyaleMgr::CondicionDeVictoria()
{
    if (estadoActual == ESTADO_BATALLA_EN_CURSO)
    {
        if (HayJugadores())
        {
            int cantidadVivos = 0;
            Player* vivo = nullptr;
            for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
            {
                if ((*it).second && (*it).second->IsAlive())
                {
                    cantidadVivos++;
                    vivo = (*it).second;
                }
            }
            if (cantidadVivos <= 1)
            {
                FinalizarRonda(cantidadVivos == 1, vivo);
            }
        }
        else
        {
            FinalizarRonda(false);
        }
    }
}

void BattleRoyaleMgr::FinalizarRonda(bool announce, Player* winner)
{
    if (announce && winner && EstaEnEvento(winner))
    {
        std::ostringstream msg;
        msg << "|cff4CFF00BattleRoyale::|r Ronda finalizada, ganador: " << Chat(winner).GetNameLink(winner) << ", racha: |cff4CFF00" << list_Datos[winner->GetGUID().GetCounter()].kills << "|r.";
        sWorld->SendServerMessage(SERVER_MSG_STRING, msg.str().c_str());
    }
    if (HayJugadores()) for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it) SalirDelEvento((*it).first);
    DesaparecerTodosLosObjetos();
    estadoActual = ESTADO_NO_HAY_SUFICIENTES_JUGADORES;
    if (HaySuficientesEnCola()) IniciarNuevaRonda();
}
