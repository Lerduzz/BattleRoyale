#include "BattleRoyaleMgr.h"
#include "Config.h"

BattleRoyaleMgr::BattleRoyaleMgr()
{
    conf_JugadoresMinimo = sConfigMgr->GetOption<uint32>("BattleRoyale.MinPlayers", 25);
    conf_JugadoresMaximo = sConfigMgr->GetOption<uint32>("BattleRoyale.MaxPlayers", 50);
    conf_IntervaloDeZona = sConfigMgr->GetOption<uint32>("BattleRoyale.SecureZoneInterval", 60000);
    QueryResult result = WorldDatabase.Query("SELECT `id`, `map_id`, `map_name`, `center_x`, `center_y`, `center_z`, `center_o`, `ship_x`, `ship_y`, `ship_z`, `ship_o` FROM `battleroyale_maps`;");
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
            for (uint32 i = 0; i < CANTIDAD_DE_ZONAS; ++i)
            {
                QueryResult result_spawn = WorldDatabase.Query("SELECT `id`, `pos_x`, `pos_y`, `pos_z`, `pos_o` FROM `battleroyale_maps_spawns` WHERE `zone` = {} AND `map` = {};", i, id);
                if (result_spawn)
                {
                    do
                    {
                        Field* fields_spawn    = result_spawn->Fetch();
                        uint32 id_spawn    = fields_spawn[0].Get<uint32>();
                        mapa->ubicacionesMapa[i][id_spawn] = {
                            fields_spawn[1].Get<float>(),
                            fields_spawn[2].Get<float>(),
                            fields_spawn[3].Get<float>(),
                            fields_spawn[4].Get<float>()
                        };
                    } while (result_spawn->NextRow());
                }
            }
            list_Mapas[id] = mapa;
        } while (result->NextRow());
    }
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
    uint32 guid = player->GetGUID().GetCounter();
    std::string blr = sBRListaNegraMgr->EstaBloqueado(guid);
    if (blr != "")
    {
        // TODO: Mover mensaje a sBRChat.
        ChatHandler(player->GetSession()).PSendSysMessage("|cff4CFF00BattleRoyale::|r No puedes utilizar este modo de juego. Motivo: |cffff0000%s|r.", blr.c_str());
        return;
    }
    if (player->isUsingLfg())
    {
        sBRChatMgr->AnunciarMensajeEntrada(player, MENSAJE_ERROR_MAZMORRA);
        return;
    }
    if (player->InBattlegroundQueue())
    {
        sBRChatMgr->AnunciarMensajeEntrada(player, MENSAJE_ERROR_BG);
        return;
    }
    if (EstaEnCola(player))
    {
        sBRChatMgr->AnunciarMensajeEntrada(player, MENSAJE_ERROR_EN_COLA);
        return;
    }
    if (EstaEnEvento(player))
    {
        sBRChatMgr->AnunciarMensajeEntrada(player, MENSAJE_ERROR_EN_EVENTO);
        return;
    }
    switch (estadoActual)
    {
        case ESTADO_NO_HAY_SUFICIENTES_JUGADORES:
        {
            list_Cola[guid] = player;
            if (HaySuficientesEnCola()) {
                IniciarNuevaRonda();
            }
            else
            {
                sBRChatMgr->AnunciarJugadoresEnCola(player, conf_JugadoresMinimo, list_Cola);
            }
            break;
        }
        case ESTADO_INVOCANDO_JUGADORES:
        {
            if (EstaLlenoElEvento()) {
                list_Cola[guid] = player;
                sBRChatMgr->AnunciarJugadoresEnCola(player, conf_JugadoresMinimo, list_Cola, MENSAJE_ESTADO_EVENTO_LLENO);
            }
            else
            {
                if (tiempoRestanteInicio >= 60)
                {
                    list_Jugadores[guid] = player;
                    AlmacenarPosicionInicial(guid);
                    LlamarDentroDeNave(guid);
                }
                else
                {
                    list_Cola[guid] = player;
                    sBRChatMgr->AnunciarJugadoresEnCola(player, conf_JugadoresMinimo, list_Cola, MENSAJE_ESTADO_EVENTO_EN_CURSO);
                }
            }
            break;
        }
        default:
        {
            list_Cola[guid] = player;
            sBRChatMgr->AnunciarJugadoresEnCola(player, conf_JugadoresMinimo, list_Cola, MENSAJE_ESTADO_EVENTO_EN_CURSO);
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
        if (!killer || !killed || !EstaEnEvento(killer) || !EstaEnEvento(killed)) return;
        sBRSonidosMgr->ReproducirSonidoParaTodos(SONIDO_ALGUIEN_MUERE, list_Jugadores);
        if (killer == killed)
        {
            sBRChatMgr->AnunciarMuerteJcJ(killer, killed, 0, list_Jugadores);
            return;
        }
        list_Datos[killer->GetGUID().GetCounter()].kills++;
        sBRChatMgr->AnunciarMuerteJcJ(killer, killed, list_Datos[killer->GetGUID().GetCounter()].kills, list_Jugadores);
        TodosLosMuertosEspectarme(killer);
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
                    sBRSonidosMgr->ReproducirSonidoParaTodos(SONIDO_RONDA_INICIADA, list_Jugadores);
                    sBRChatMgr->NotificarTiempoInicial(0, list_Jugadores, mapaActual->second->nombreMapa);
                    tiempoRestanteZona = conf_IntervaloDeZona;
                    tiempoRestanteNave = 15;
                } else {
                    if (tiempoRestanteInicio % 5 == 0) {
                        if (estadoActual == ESTADO_NAVE_EN_MOVIMIENTO) VerificarJugadoresEnNave();
                        if (tiempoRestanteInicio == 45)
                        {
                            sBRSonidosMgr->ReproducirSonidoParaTodos(SONIDO_NAVE_EN_MOVIMIENTO, list_Jugadores);
                        }
                        sBRChatMgr->NotificarTiempoInicial(tiempoRestanteInicio, list_Jugadores);
                    }
                    if (estadoActual == ESTADO_INVOCANDO_JUGADORES) DarAlasProgramado();
                    if (estadoActual == ESTADO_INVOCANDO_JUGADORES && tiempoRestanteInicio <= 45)
                    {
                        if (sBRObjetosMgr->EncenderNave())
                        {
                            estadoActual = ESTADO_NAVE_EN_MOVIMIENTO;
                        }
                        else
                        {
                            RestablecerTodoElEvento();
                        }
                    }
                    if (estadoActual == ESTADO_NAVE_EN_MOVIMIENTO && tiempoRestanteInicio <= 20)
                    {
                        estadoActual = ESTADO_NAVE_CERCA_DEL_CENTRO;
                        indiceDeZona = 0;
                        tiempoRestanteZona = 0;
                        estaZonaAnunciada5s = false;
                        estaZonaAnunciada10s = false;
                        if (!HayJugadores() || !sBRObjetosMgr->InvocarCentroDelMapa(mapaActual->second->idMapa, mapaActual->second->centroMapa))
                        {
                            RestablecerTodoElEvento();
                            return;
                        }
                        if (!HayJugadores() || !sBRObjetosMgr->InvocarZonaSegura(mapaActual->second->idMapa, mapaActual->second->centroMapa, indiceDeZona))
                        {
                            RestablecerTodoElEvento();
                            return;
                        }
                        else
                        {
                            AlReducirseLaZona();
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
                if (--tiempoRestanteNave <= 0)
                {
                    if (sBRObjetosMgr->DesaparecerNave())
                    {
                        sBRSonidosMgr->ReproducirSonidoParaTodos(SONIDO_NAVE_RETIRADA, list_Jugadores);
                        sBRChatMgr->NotificarNaveRetirada(list_Jugadores);
                    }
                }
                QuitarAlasProgramado();
            } else indicadorDeSegundos -= diff;
            if (tiempoRestanteZona <= 0) {
                if (!HayJugadores() || !sBRObjetosMgr->InvocarZonaSegura(mapaActual->second->idMapa, mapaActual->second->centroMapa, indiceDeZona))
                {
                    RestablecerTodoElEvento();
                    return;
                }
                else
                {
                    AlReducirseLaZona();
                }
                sBRSonidosMgr->ReproducirSonidoParaTodos(SONIDO_ZONA_REDUCIDA, list_Jugadores);
                sBRChatMgr->NotificarZonaReducida(list_Jugadores);
                tiempoRestanteZona = conf_IntervaloDeZona;
                estaZonaAnunciada5s = false;
                estaZonaAnunciada10s = false;
            } else {
                if (!estaZonaAnunciada5s && tiempoRestanteZona <= 5000) {
                    sBRSonidosMgr->ReproducirSonidoParaTodos(SONIDO_ZONA_TIEMPO, list_Jugadores);
                    sBRChatMgr->NotificarAdvertenciaDeZona(5, list_Jugadores);
                    estaZonaAnunciada5s = true;
                }
                if (!estaZonaAnunciada10s && tiempoRestanteZona <= 10000) {
                    sBRSonidosMgr->ReproducirSonidoParaTodos(SONIDO_ZONA_TIEMPO, list_Jugadores);
                    sBRChatMgr->NotificarAdvertenciaDeZona(10, list_Jugadores);
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
                    while (HayJugadores()) SalirDelEvento((*list_Jugadores.begin()).first);
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
    sBRObjetosMgr->DesaparecerTodosLosObjetos();
    estadoActual = ESTADO_NO_HAY_SUFICIENTES_JUGADORES;
}

void BattleRoyaleMgr::IniciarNuevaRonda()
{
    if (estadoActual == ESTADO_NO_HAY_SUFICIENTES_JUGADORES)
    {
        tiempoRestanteInicio = 75;
        if (!HayCola() || !sBRObjetosMgr->InvocarNave(mapaActual->second->idMapa, mapaActual->second->inicioNave))
        {
            RestablecerTodoElEvento();
            return;
        }
        estadoActual = ESTADO_INVOCANDO_JUGADORES;
        while (HayCola() && !EstaLlenoElEvento() && tiempoRestanteInicio >= 60)
        {
            uint32 guid = (*list_Cola.begin()).first;
            if (list_Cola[guid]->IsInFlight())
            {
                sBRChatMgr->AnunciarMensajeEntrada(list_Cola[guid], MENSAJE_ERROR_EN_VUELO);
            }
            else if (list_Cola[guid]->IsInCombat())
            {
                sBRChatMgr->AnunciarMensajeEntrada(list_Cola[guid], MENSAJE_ERROR_EN_COMBATE);
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
    DejarGrupo(player);
    Desmontar(player);
    float ox = BR_VariacionesDePosicion[indiceDeVariacion][0];
    float oy = BR_VariacionesDePosicion[indiceDeVariacion][1];
    BR_Mapa* brM = mapaActual->second;
    Position iN = brM->inicioNave;
    player->SetPhaseMask(DIMENSION_EVENTO, true);
    player->TeleportTo(brM->idMapa, iN.GetPositionX() + ox, iN.GetPositionY() + oy, iN.GetPositionZ() + 2.5f, iN.GetOrientation() + M_PI / 2.0f);
    player->SetPvP(false);
    // TODO: Analizar si esto es factible: player->AddAura(HECHIZO_LENGUAJE_BINARIO, player);
    player->SaveToDB(false, false);
    list_DarAlas[guid] = player;
    SiguientePosicion();
}

void BattleRoyaleMgr::SalirDelEvento(uint32 guid, bool logout /* = false*/)
{
    if (EstaEnListaDarAlas(guid)) list_DarAlas.erase(guid);
    if (EstaEnCola(guid)) list_Cola.erase(guid);
    if (EstaEnEvento(guid))
    {
        Player* player = list_Jugadores[guid];
        player->SetPhaseMask(DIMENSION_NORMAL, true);
        if (player->IsAlive())
        {
            if (player->HasAura(HECHIZO_ALAS_MAGICAS)) player->RemoveAurasDueToSpell(HECHIZO_ALAS_MAGICAS);
            // TODO: Analizar si esto es factible: if (player->HasAura(HECHIZO_LENGUAJE_BINARIO)) player->RemoveAurasDueToSpell(HECHIZO_LENGUAJE_BINARIO);
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

void BattleRoyaleMgr::EfectoFueraDeZona()
{
    if (HayJugadores())
    {
        for (BR_ListaDePersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
        {
            if (it->second && it->second->IsAlive() && sBRObjetosMgr->HayCentro())
            {
                float distance = sBRObjetosMgr->DistanciaDelCentro(it->second);
                if (!sBRObjetosMgr->EstaLaZonaActiva() || (indiceDeZona > 0 && distance > BR_EscalasDeZonaSegura[indiceDeZona - 1] * 66.0f)) {
                    list_Datos[it->first].dmg_tick++;
                    uint32 damage = it->second->GetMaxHealth() * (2 * sqrt(list_Datos[it->first].dmg_tick) + indiceDeZona) / 100;
                    it->second->GetSession()->SendNotification("|cffff0000¡Has recibido |cffDA70D6%u|cffff0000 de daño, adéntrate en la zona segura!", damage); // TODO: Mover al sistema de mensajes.
                    Unit::DealDamage(it->second, it->second, damage, nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false, true);
                } else {
                    list_Datos[it->first].dmg_tick = 0;
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
            if (it->second && it->second->IsAlive() && DebeForzarJcJTcT(it->second) && !(it->second->HasByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP)))
            {
                it->second->SetByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP);
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
            if (it->second && it->second->IsAlive())
            {
                if
                (
                    (it->second->HasAura(31700)) ||
                    (sBRObjetosMgr->DistanciaDelCentro(it->second) > 1147.0f)
                )
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
                if (it->second && it->second->IsAlive())
                {
                    cantidadVivos++;
                    vivo = it->second;
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
    if (announce && winner && EstaEnEvento(winner))
    {
        if (winner->GetTeamId() == TEAM_ALLIANCE)
        {
            sBRSonidosMgr->ReproducirSonidoParaTodos(SONIDO_GANADOR_ALIANZA, list_Jugadores);
        }
        else
        {
            sBRSonidosMgr->ReproducirSonidoParaTodos(SONIDO_GANADOR_HORDA, list_Jugadores);
        }
        sBRChatMgr->AnunciarGanador(winner, list_Datos[winner->GetGUID().GetCounter()].kills);
        TodosLosMuertosEspectarme(winner);
        sBRTitulosMgr->Ascender(winner);
    }
    else
    {
        sBRChatMgr->AnunciarEmpate();
    }
    sBRObjetosMgr->DesaparecerTodosLosObjetos();
    tiempoRestanteFinal = 10;
    SiguienteMapa();
    estadoActual = ESTADO_BATALLA_TERMINADA;
}
