#include "BattleRoyaleMgr.h"
#include "Config.h"
#include "MapMgr.h"

BattleRoyaleMgr::BattleRoyaleMgr()
{
    conf_JugadoresMinimo = sConfigMgr->GetOption<int32>("BattleRoyale.MinPlayers", 25);
    conf_JugadoresMaximo = sConfigMgr->GetOption<int32>("BattleRoyale.MaxPlayers", 50);
    conf_IntervaloDeZona = sConfigMgr->GetOption<int32>("BattleRoyale.SecureZoneInterval", 60000);
    obj_Zona = nullptr;
    obj_Centro = nullptr;
    obj_Nave = nullptr;
    RestablecerTodoElEvento();
}

BattleRoyaleMgr::~BattleRoyaleMgr()
{
    RestablecerTodoElEvento();
}

// --- PUBLICO ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void BattleRoyaleMgr::GestionarJugadorEntrando(Player* player)
{
    if (!player) return;
    if (player->isUsingLfg())
    {
        Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡No puedes participar mientras utilizas el buscador de mazmorras!");
        return;
    }
    if (player->InBattlegroundQueue())
    {
        Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡No puedes participar mientras estás en cola para Campos de Batalla o Arenas!");
        return;
    }
    if (EstaEnCola(player))
    {
        Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡Ya estas en cola para el evento!");
        return;
    }
    if (EstaEnEvento(player))
    {
        Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r ¡Ya estas dentro del evento!");
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
                NotificarJugadoresEnCola(player);
            }
            break;
        }
        case ESTADO_INVOCANDO_JUGADORES:
        {
            if (EstaLlenoElEvento()) {
                list_Cola[player->GetGUID().GetCounter()] = player;
                Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r Te has unido a la cola del evento. Jugadores en cola: |cff4CFF00%u|r/|cff4CFF00%u|r. Evento lleno, espera a que termine la ronda.", list_Cola.size(), conf_JugadoresMinimo);
                NotificarJugadoresEnCola(player);
            }
            else
            {
                if (tiempoRestanteInicio >= 45)
                {
                    list_Jugadores[player->GetGUID().GetCounter()] = player;
                    AlmacenarPosicionInicial(player->GetGUID().GetCounter());
                    LlamarDentroDeNave(player->GetGUID().GetCounter());
                }
                else
                {
                    list_Cola[player->GetGUID().GetCounter()] = player;
                    Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r Te has unido a la cola del evento. Jugadores en cola: |cff4CFF00%u|r/|cff4CFF00%u|r. Evento en curso, espera a que termine la ronda.", list_Cola.size(), conf_JugadoresMinimo);
                    NotificarJugadoresEnCola(player);
                }
            }
            break;
        }
        default:
        {
            list_Cola[player->GetGUID().GetCounter()] = player;
            Chat(player).PSendSysMessage("|cff4CFF00BattleRoyale::|r Te has unido a la cola del evento. Jugadores en cola: |cff4CFF00%u|r/|cff4CFF00%u|r. Evento en curso, espera a que termine la ronda.", list_Cola.size(), conf_JugadoresMinimo);
            NotificarJugadoresEnCola(player);
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
    if (HayJugadores() && estadoActual == ESTADO_BATALLA_EN_CURSO)
    {
        if (!killer || !killed || killer == killed || !EstaEnEvento(killer) || !EstaEnEvento(killed)) return;
        list_Datos[killer->GetGUID().GetCounter()].kills++;
        TodosLosMuertosEspectarme(killer);
        NotificarMuerteJcJ(Chat(killer).GetNameLink(killer), Chat(killed).GetNameLink(killed), list_Datos[killer->GetGUID().GetCounter()].kills);
    }
}

void BattleRoyaleMgr::GestionarActualizacionMundo(uint32 diff)
{
    switch(estadoActual)
    {
        case ESTADO_INVOCANDO_JUGADORES:
        case ESTADO_NAVE_EN_MOVIMIENTO:
        case ESTADO_NAVE_CERCA_DEL_CENTRO:
        {
            if (indicadorDeSegundos <= 0) {
                indicadorDeSegundos = 1000;
                if (tiempoRestanteInicio <= 0) {
                    estadoActual = ESTADO_BATALLA_EN_CURSO;
                    NotificarTiempoParaIniciar(0);
                    tiempoRestanteZona = conf_IntervaloDeZona;
                    tiempoRestanteNave = 30;
                } else {
                    if (tiempoRestanteInicio % 5 == 0) {
                        if (estadoActual == ESTADO_NAVE_EN_MOVIMIENTO) VerificarJugadoresEnNave();
                        NotificarTiempoParaIniciar(tiempoRestanteInicio);
                    }
                    if (estadoActual == ESTADO_INVOCANDO_JUGADORES && tiempoRestanteInicio <= 30 && obj_Nave)
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
                if (!CondicionDeVictoria())
                {
                    EfectoFueraDeZona();
                    ActivarJcJTcT();
                    VerificarEspectadores();
                }
                if (--tiempoRestanteNave <= 0) if (DesaparecerNave()) NotificarNaveRetirada();
            } else indicadorDeSegundos -= diff;
            if (tiempoRestanteZona <= 0) {
                if (!InvocarZonaSegura()) {
                    RestablecerTodoElEvento();
                    return;
                }
                NotificarZonaReducida();
                tiempoRestanteZona = conf_IntervaloDeZona;
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
        case ESTADO_BATALLA_TERMINADA:
        {
            if (indicadorDeSegundos <= 0) {
                indicadorDeSegundos = 1000;
                if (--tiempoRestanteFinal <= 0)
                {
                    estadoActual = ESTADO_NO_HAY_SUFICIENTES_JUGADORES;
                    if (HaySuficientesEnCola()) IniciarNuevaRonda();
                }
            } else indicadorDeSegundos -= diff;
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

// --- PRIVADO ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void BattleRoyaleMgr::RestablecerTodoElEvento()
{
    list_Cola.clear();
	list_Jugadores.clear();
    list_Datos.clear();
    indiceDelMapa = 0;
    indicadorDeSegundos = 1000;
    indiceDeVariacion = 0;
    estaLaZonaActiva = false;
    DesaparecerTodosLosObjetos();
    estadoActual = ESTADO_NO_HAY_SUFICIENTES_JUGADORES;
}

void BattleRoyaleMgr::IniciarNuevaRonda()
{
    if (estadoActual == ESTADO_NO_HAY_SUFICIENTES_JUGADORES)
    {
        SiguienteMapa();
        tiempoRestanteInicio = 60;
        if (!InvocarNave()) {
            RestablecerTodoElEvento();
            return;
        }
        estadoActual = ESTADO_INVOCANDO_JUGADORES;
        while (HayCola() && !EstaLlenoElEvento() && tiempoRestanteInicio >= 35)
        {
            uint32 guid = (*list_Cola.begin()).first;
            list_Jugadores[guid] = list_Cola[guid];
            AlmacenarPosicionInicial(guid);
            LlamarDentroDeNave(guid);
            list_Cola.erase(guid);
        }
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

void BattleRoyaleMgr::LlamarDentroDeNave(uint32 guid)
{
    Player* player = list_Jugadores[guid];
    if (!player->IsAlive()) RevivirJugador(player);
    CambiarDimension_Entrar(guid);
    DejarGrupo(player);
    float ox = BR_VariacionesDePosicion[indiceDeVariacion][0];
    float oy = BR_VariacionesDePosicion[indiceDeVariacion][1];
    SiguientePosicion();
    Desmontar(player);
    player->TeleportTo(BR_IdentificadorDeMapas[indiceDelMapa], BR_InicioDeLaNave[indiceDelMapa][0] + ox, BR_InicioDeLaNave[indiceDelMapa][1] + oy, BR_InicioDeLaNave[indiceDelMapa][2] + 2.5f, BR_InicioDeLaNave[indiceDelMapa][3] + M_PI / 2.0f);
    player->SetPvP(false);
    player->AddAura(HECHIZO_LENGUAJE_BINARIO, player);
    player->SaveToDB(false, false);
    player->GetMotionMaster()->MoveFall();
    DarAlas(player);
}

void BattleRoyaleMgr::SalirDelEvento(uint32 guid, bool logout /* = false*/)
{
    if (EstaEnCola(guid))
    {
        list_Cola.erase(guid);
    };
    if (EstaEnEvento(guid))
    {
        CambiarDimension_Salir(guid);
        Player* player = list_Jugadores[guid];
        if (player->IsAlive())
        {
            if (player->HasAura(HECHIZO_ALAS_MAGICAS)) player->RemoveAurasDueToSpell(HECHIZO_ALAS_MAGICAS);
            if (player->HasAura(HECHIZO_LENGUAJE_BINARIO)) player->RemoveAurasDueToSpell(HECHIZO_LENGUAJE_BINARIO);
            if (player->HasAura(HECHIZO_ANTI_INVISIBLES)) player->RemoveAurasDueToSpell(HECHIZO_ANTI_INVISIBLES);
            if (player->HasAura(HECHIZO_ANTI_SANADORES)) player->RemoveAurasDueToSpell(HECHIZO_ANTI_SANADORES);
            player->AddAura(HECHIZO_PARACAIDAS, player);
        }
        if(!logout)
        {
            if (!player->IsAlive()) RevivirJugador(player);
            if (!player->isPossessing()) player->StopCastingBindSight();
            QuitarAlas(player);
            player->TeleportTo(list_Datos[guid].GetMap(), list_Datos[guid].GetX(), list_Datos[guid].GetY(), list_Datos[guid].GetZ(), list_Datos[guid].GetO());
            player->SaveToDB(false, false);
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
    if (HayCola())
    {
        int mapID = BR_IdentificadorDeMapas[indiceDelMapa];
        Map* map = sMapMgr->FindBaseNonInstanceMap(mapID);
        if (map)
        {
            DesaparecerNave();
            float x = BR_InicioDeLaNave[indiceDelMapa][0];
            float y = BR_InicioDeLaNave[indiceDelMapa][1];
            float z = BR_InicioDeLaNave[indiceDelMapa][2];
            float o = BR_InicioDeLaNave[indiceDelMapa][3];
            float rot2 = std::sin(o / 2);
            float rot3 = cos(o / 2);
            map->LoadGrid(x, y);
            obj_Nave = new StaticTransport();
            if (obj_Nave->Create(map->GenerateLowGuid<HighGuid::GameObject>(), OBJETO_NAVE, map, DIMENSION_EVENTO, x, y, z, o, G3D::Quat(0, 0, rot2, rot3), 100, GO_STATE_READY))
            {
                obj_Nave->SetVisibilityDistanceOverride(VisibilityDistanceType::Infinite);
                map->AddToMap(obj_Nave);
                LOG_ERROR("br.nave", "BattleRoyaleMgr::InvocarNave: Nave invocada en M: {}, X: {}, Y: {}, Z: {}!", mapID, x, y, z);
                return true;
            }
            else
            {
                LOG_ERROR("br.nave", "BattleRoyaleMgr::InvocarNave: No se ha podido invocar la nave (OBJETO = {})!", OBJETO_NAVE);
                delete obj_Nave;
                obj_Nave = nullptr;
            }
        }
        else
        {
            LOG_ERROR("br.nave", "BattleRoyaleMgr::InvocarNave: No se ha podido obtener el mapa para la nave (MAPA: {})!", mapID);
        }
    }
    else
    {
        LOG_ERROR("br.nave", "BattleRoyaleMgr::InvocarNave: No se ha invocado la nave (OBJETO = {}) porque no hay jugadores!", OBJETO_NAVE);
    }
    return false;
}

bool BattleRoyaleMgr::InvocarCentroDelMapa()
{
    if (HayJugadores())
    {
        int mapID = BR_IdentificadorDeMapas[indiceDelMapa];
        Map* map = sMapMgr->FindBaseNonInstanceMap(mapID);
        if (map)
        {
            DesaparecerCentro();
            float x = BR_CentroDeMapas[indiceDelMapa].GetPositionX();
            float y = BR_CentroDeMapas[indiceDelMapa].GetPositionY();
            float z = BR_CentroDeMapas[indiceDelMapa].GetPositionZ();
            map->LoadGrid(x, y);
            obj_Centro = new GameObject();
            if (obj_Centro->Create(map->GenerateLowGuid<HighGuid::GameObject>(), OBJETO_CENTRO_DEL_MAPA, map, DIMENSION_EVENTO, x, y, z, 0, G3D::Quat(), 100, GO_STATE_READY))
            {
                obj_Centro->SetVisibilityDistanceOverride(VisibilityDistanceType::Infinite);
                map->AddToMap(obj_Centro);
                return true;
            }
            else
            {
                LOG_ERROR("br.nave", "BattleRoyaleMgr::InvocarCentroDelMapa: No se ha podido invocar el centro (OBJETO = {})!", OBJETO_CENTRO_DEL_MAPA);
                delete obj_Centro;
                obj_Centro = nullptr;
            }
        }
        else
        {
            LOG_ERROR("br.nave", "BattleRoyaleMgr::InvocarCentroDelMapa: No se ha podido obtener el mapa para el centro (MAPA: {})!", mapID);
        }
    }
    else
    {
        LOG_ERROR("br.nave", "BattleRoyaleMgr::InvocarCentroDelMapa: No se ha invocado el centro (OBJETO = {}) porque no hay jugadores!", OBJETO_CENTRO_DEL_MAPA);
    }
    return false;
}

bool BattleRoyaleMgr::InvocarZonaSegura()
{
    if (HayJugadores())
    {
        int mapID = BR_IdentificadorDeMapas[indiceDelMapa];
        Map* map = sMapMgr->FindBaseNonInstanceMap(mapID);
        if (map)
        {
            HechizosDeZona();
            DesaparecerZona();
            if (indiceDeZona < CANTIDAD_DE_ZONAS)
            {
                float x = BR_CentroDeMapas[indiceDelMapa].GetPositionX();
                float y = BR_CentroDeMapas[indiceDelMapa].GetPositionY();
                float z = BR_CentroDeMapas[indiceDelMapa].GetPositionZ() + BR_EscalasDeZonaSegura[indiceDeZona] * 66.0f;
                map->LoadGrid(x, y);
                obj_Zona = new GameObject();
                if (obj_Zona->Create(map->GenerateLowGuid<HighGuid::GameObject>(), OBJETO_ZONA_SEGURA_INICIAL + indiceDeZona, map, DIMENSION_EVENTO, x, y, z, 0, G3D::Quat(), 100, GO_STATE_READY))
                {
                    obj_Zona->SetVisibilityDistanceOverride(VisibilityDistanceType::Infinite);
                    map->AddToMap(obj_Zona);
                    indiceDeZona++;
                    estaLaZonaActiva = true;
                    return true;
                }
                else
                {
                    LOG_ERROR("br.nave", "BattleRoyaleMgr::InvocarZonaSegura: No se ha podido invocar la zona (OBJETO = {})!", OBJETO_ZONA_SEGURA_INICIAL + indiceDeZona);
                    delete obj_Zona;
                    obj_Zona = nullptr;
                }
            }
            else
            {
                estaLaZonaActiva = false;
                return true;
            }
        }
        else
        {
            LOG_ERROR("br.nave", "BattleRoyaleMgr::InvocarZonaSegura: No se ha podido obtener el mapa para la zona (MAPA: {})!", mapID);
        }
    }
    else
    {
        LOG_ERROR("br.nave", "BattleRoyaleMgr::InvocarZonaSegura: No se ha invocado la zona (OBJETO = {}) porque no hay jugadores!", OBJETO_ZONA_SEGURA_INICIAL + indiceDeZona);
    }
    return false;
}

void BattleRoyaleMgr::EfectoFueraDeZona()
{
    if (HayJugadores())
    {
        for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
        {
            if (obj_Centro && (*it).second && (*it).second->IsAlive())
            {
                float distance = (*it).second->GetExactDist(obj_Centro);
                if (!estaLaZonaActiva || (indiceDeZona > 0 && distance > BR_EscalasDeZonaSegura[indiceDeZona - 1] * 66.0f)) {
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
        BR_ListaDePersonajes::iterator it = list_Jugadores.begin();
        while (it != list_Jugadores.end())
        {
            if ((*it).second && (*it).second->IsAlive())
            {
                if
                (
                    ((*it).second->HasAura(31700)) ||
                    (obj_Centro && (*it).second->GetExactDist(obj_Centro) > 1147.0f)
                )
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
            else
            {
                ++it;
            }
        }
    }
}

bool BattleRoyaleMgr::CondicionDeVictoria()
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
                return true;
            }
        }
        else
        {
            FinalizarRonda(false);
            return true;
        }
    }
    return false;
}

void BattleRoyaleMgr::FinalizarRonda(bool announce, Player* winner /* = nullptr*/)
{
    if (announce && winner && EstaEnEvento(winner)) NotificarGanadorAlMundo(winner, list_Datos[winner->GetGUID().GetCounter()].kills);
    DesaparecerTodosLosObjetos();
    while (HayJugadores()) SalirDelEvento((*list_Jugadores.begin()).first);
    tiempoRestanteFinal = 10;
    estadoActual = ESTADO_BATALLA_TERMINADA;
}
