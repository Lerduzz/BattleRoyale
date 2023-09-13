#include "BattleRoyaleMgr.h"
#include "Config.h"
#include "MapMgr.h"

BattleRoyaleMgr::BattleRoyaleMgr()
{
    conf_JugadoresMinimo = sConfigMgr->GetOption<uint32>("BattleRoyale.MinPlayers", 25);
    conf_JugadoresMaximo = sConfigMgr->GetOption<uint32>("BattleRoyale.MaxPlayers", 50);
    conf_IntervaloDeZona = sConfigMgr->GetOption<uint32>("BattleRoyale.SecureZoneInterval", 60000);
    QueryResult result = WorldDatabase.Query("SELECT * FROM `battleroyale_maps`;");
    if (result)
    {
        do
        {
            Field* fields    = result->Fetch();
            BR_Mapa* mapa    = new BR_Mapa();
            uint32 id        = fields[0].Get<uint32>();
            mapa->idMapa     = fields[1].Get<uint32>();
            mapa->nombreMapa = fields[2].Get<std::string>();
            mapa->centroMapa = { 
                fields[3].Get<float>(),
                fields[4].Get<float>(),
                fields[5].Get<float>(),
                fields[6].Get<float>()
            };
            mapa->inicioNave = { 
                fields[7].Get<float>(),
                fields[8].Get<float>(),
                fields[9].Get<float>(),
                fields[10].Get<float>()
            };

            // TODO: Cargar spawns para cada zona en cada mapa.
            list_Mapas[id] = mapa;
        } while (result->NextRow());
    }
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
                if (tiempoRestanteInicio >= 60)
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
    if (EstaEnEvento(player) || EstaEnCola(player) || EstaEnListaDeAlas(player)) SalirDelEvento(player->GetGUID().GetCounter(), true);
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
                    tiempoRestanteNave = 15;
                } else {
                    if (tiempoRestanteInicio % 5 == 0) {
                        if (estadoActual == ESTADO_NAVE_EN_MOVIMIENTO) VerificarJugadoresEnNave();
                        NotificarTiempoParaIniciar(tiempoRestanteInicio);
                    }
                    if (estadoActual == ESTADO_INVOCANDO_JUGADORES) DarAlasProgramado();
                    if (estadoActual == ESTADO_INVOCANDO_JUGADORES && tiempoRestanteInicio <= 45 && obj_Nave)
                    {
                        estadoActual = ESTADO_NAVE_EN_MOVIMIENTO;
                        uint32_t const autoCloseTime = obj_Nave->GetGOInfo()->GetAutoCloseTime() ? 10000u : 0u;
                        obj_Nave->SetLootState(GO_READY);
                        obj_Nave->UseDoorOrButton(autoCloseTime, false, nullptr);
                    }
                    if (estadoActual == ESTADO_NAVE_EN_MOVIMIENTO && tiempoRestanteInicio <= 20)
                    {
                        estadoActual = ESTADO_NAVE_CERCA_DEL_CENTRO;
                        indiceDeZona = 0;
                        tiempoRestanteZona = 0;
                        estaZonaAnunciada5s = false;
                        estaZonaAnunciada10s = false;
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
                QuitarAlasProgramado();
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
                QuitarAlasProgramado();
            } else indicadorDeSegundos -= diff;
            if (tiempoRestanteZona <= 0) {
                if (!InvocarZonaSegura()) {
                    RestablecerTodoElEvento();
                    return;
                }
                NotificarZonaReducida();
                tiempoRestanteZona = conf_IntervaloDeZona;
                estaZonaAnunciada5s = false;
                estaZonaAnunciada10s = false;
            } else {
                if (!estaZonaAnunciada5s && tiempoRestanteZona <= 5000) {
                    NotificarAdvertenciaDeZona(5);
                    estaZonaAnunciada5s = true;
                }
                if (!estaZonaAnunciada10s && tiempoRestanteZona <= 10000) {
                    NotificarAdvertenciaDeZona(10);
                    estaZonaAnunciada10s = true;
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
                QuitarAlasProgramado();
            } else indicadorDeSegundos -= diff;
            break;
        }
        default:
        {
            if (indicadorDeSegundos <= 0) {
                indicadorDeSegundos = 1000;
                QuitarAlasProgramado();
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
    mapaActual = list_Mapas.begin();
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
        tiempoRestanteInicio = 75;
        if (!InvocarNave()) {
            RestablecerTodoElEvento();
            return;
        }
        estadoActual = ESTADO_INVOCANDO_JUGADORES;
        while (HayCola() && !EstaLlenoElEvento() && tiempoRestanteInicio >= 60)
        {
            uint32 guid = (*list_Cola.begin()).first;
            if (list_Cola[guid]->IsInFlight())
            {
                Chat(list_Cola[guid]).SendSysMessage("|cff4CFF00BattleRoyale::|r ¡No has podido entrar al evento porque vas en ruta de vuelo! ¡Se te ha quitado de la cola!");
            }
            else
            {
                list_Jugadores[guid] = list_Cola[guid];
                AlmacenarPosicionInicial(guid);
                LlamarDentroDeNave(guid);
            }
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
    if (player->IsAlive())
    {
        if (player->HasAura(HECHIZO_PARACAIDAS)) player->RemoveAurasDueToSpell(HECHIZO_PARACAIDAS);
        if (player->HasAura(HECHIZO_PARACAIDAS_EFECTO)) player->RemoveAurasDueToSpell(HECHIZO_PARACAIDAS_EFECTO);
    }
    else
    {
        RevivirJugador(player);
    }
    player->SetPhaseMask(DIMENSION_EVENTO, true);
    DejarGrupo(player);
    float ox = BR_VariacionesDePosicion[indiceDeVariacion][0];
    float oy = BR_VariacionesDePosicion[indiceDeVariacion][1];
    SiguientePosicion();
    Desmontar(player);
    player->TeleportTo((*mapaActual).second->idMapa, (*mapaActual).second->inicioNave.GetPositionX() + ox, (*mapaActual).second->inicioNave.GetPositionY() + oy, (*mapaActual).second->inicioNave.GetPositionZ() + 2.5f, (*mapaActual).second->inicioNave.GetOrientation() + M_PI / 2.0f);
    player->SetPvP(false);
    player->AddAura(HECHIZO_LENGUAJE_BINARIO, player);
    player->SaveToDB(false, false);
    list_DarAlas[guid] = player;
}

void BattleRoyaleMgr::SalirDelEvento(uint32 guid, bool logout /* = false*/)
{
    if (EstaEnListaDarAlas(guid)) list_DarAlas.erase(guid);
    if (EstaEnCola(guid)) list_Cola.erase(guid);
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
        }
        if(!logout)
        {
            if (!player->IsAlive()) RevivirJugador(player);
            if (!player->isPossessing()) player->StopCastingBindSight();
            list_QuitarAlas[guid] = player;
            player->AddAura(HECHIZO_PARACAIDAS, player);
            player->TeleportTo(list_Datos[guid].GetMap(), list_Datos[guid].GetX(), list_Datos[guid].GetY(), list_Datos[guid].GetZ(), list_Datos[guid].GetO());
            player->SaveToDB(false, false);
        }
        list_Jugadores.erase(guid);
        list_Datos.erase(guid);
    }
    if (logout && EstaEnListaQuitarAlas(guid)) list_QuitarAlas.erase(guid);
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
        int mapID = (*mapaActual).second->idMapa;
        Map* map = sMapMgr->FindBaseNonInstanceMap(mapID);
        if (map)
        {
            DesaparecerNave();
            float x = (*mapaActual).second->inicioNave.GetPositionX();
            float y = (*mapaActual).second->inicioNave.GetPositionY();
            float z = (*mapaActual).second->inicioNave.GetPositionZ();
            float o = (*mapaActual).second->inicioNave.GetOrientation();
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
        int mapID = (*mapaActual).second->idMapa;
        Map* map = sMapMgr->FindBaseNonInstanceMap(mapID);
        if (map)
        {
            DesaparecerCentro();
            float x = (*mapaActual).second->centroMapa.GetPositionX();
            float y = (*mapaActual).second->centroMapa.GetPositionY();
            float z = (*mapaActual).second->centroMapa.GetPositionZ();
            float o = (*mapaActual).second->centroMapa.GetOrientation();
            map->LoadGrid(x, y);
            obj_Centro = new GameObject();
            if (obj_Centro->Create(map->GenerateLowGuid<HighGuid::GameObject>(), OBJETO_CENTRO_DEL_MAPA, map, DIMENSION_EVENTO, x, y, z, o, G3D::Quat(), 100, GO_STATE_READY))
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
        int mapID = (*mapaActual).second->idMapa;
        Map* map = sMapMgr->FindBaseNonInstanceMap(mapID);
        if (map)
        {
            AlReducirseLaZona();
            DesaparecerZona();
            if (indiceDeZona < CANTIDAD_DE_ZONAS)
            {
                float x = (*mapaActual).second->centroMapa.GetPositionX();
                float y = (*mapaActual).second->centroMapa.GetPositionY();
                float z = (*mapaActual).second->centroMapa.GetPositionZ() + BR_EscalasDeZonaSegura[indiceDeZona] * 66.0f;
                float o = (*mapaActual).second->centroMapa.GetOrientation();
                map->LoadGrid(x, y);
                obj_Zona = new GameObject();
                if (obj_Zona->Create(map->GenerateLowGuid<HighGuid::GameObject>(), OBJETO_ZONA_SEGURA_INICIAL + indiceDeZona, map, DIMENSION_EVENTO, x, y, z, o, G3D::Quat(), 100, GO_STATE_READY))
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
