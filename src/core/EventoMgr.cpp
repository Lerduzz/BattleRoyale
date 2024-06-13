#include "EventoMgr.h"
#include "CharacterCache.h"
#include "Config.h"

EventoMgr::EventoMgr()
{
    conf_EstaActivado = sConfigMgr->GetOption<bool>("BattleRoyale.Activado", true);
    conf_JugadoresMinimo = sConfigMgr->GetOption<uint32>("BattleRoyale.MinJugadores", 25);
    conf_JugadoresMaximo = sConfigMgr->GetOption<uint32>("BattleRoyale.MaxJugadores", 50);
    conf_IntervaloSinJugadores = sConfigMgr->GetOption<uint32>("BattleRoyale.Intervalo.SinJugadores", 1800);
    conf_IntervaloZonaSegura = sConfigMgr->GetOption<uint32>("BattleRoyale.Intervalo.ZonaSegura", 60);
    conf_IntervaloFinalDeRonda = sConfigMgr->GetOption<uint32>("BattleRoyale.Intervalo.FinalDeRonda", 10);
    conf_RequisitoAsesinatosTotales = sConfigMgr->GetOption<uint32>("BattleRoyale.Requisito.AsesinatosTotales", 5);
    conf_RequisitoAsesinatosPropios = sConfigMgr->GetOption<uint32>("BattleRoyale.Requisito.AsesinatosPropios", 1);

    conf_Recompensa = BRConfigRecompensa();
    conf_Recompensa.base = sConfigMgr->GetOption<uint32>("BattleRoyale.Recompensa.Base", 400);
    conf_Recompensa.victoria = sConfigMgr->GetOption<uint32>("BattleRoyale.Recompensa.Victoria", 600);
    conf_Recompensa.asesinar = sConfigMgr->GetOption<uint32>("BattleRoyale.Recompensa.AlAsesinar", 350);
    conf_Recompensa.serAsesinado = sConfigMgr->GetOption<uint32>("BattleRoyale.Recompensa.AlSerEAsesinado", 200);
    conf_Recompensa.morir = sConfigMgr->GetOption<uint32>("BattleRoyale.Recompensa.AlMorir", 100);
    conf_Recompensa.zona = sConfigMgr->GetOption<uint32>("BattleRoyale.Recompensa.ZonaReducida", 150);

    sMapaMgr->CargarMapasDesdeBD();
    RestablecerTodoElEvento();
}

EventoMgr::~EventoMgr()
{
    RestablecerTodoElEvento();
}

void EventoMgr::GestionarJugadorEntrando(Player *player)
{
    if (!player)
        return;
    uint32 guid = player->GetGUID().GetCounter();
    if (player->getLevel() != 80)
    {
        sMensajeMgr->AnunciarMensajeEntrada(player, BR_MENSAJE_ERROR_NIVEL);
        return;
    }
    if (player->getClass() == CLASS_DEATH_KNIGHT && player->GetMapId() == 609)
    {
        sMensajeMgr->AnunciarMensajeEntrada(player, BR_MENSAJE_ERROR_DK_INICIO);
        return;
    }
    if (EstaEnCola(player))
    {
        sMensajeMgr->AnunciarMensajeEntrada(player, BR_MENSAJE_ERROR_EN_COLA);
        return;
    }
    if (EstaEnEvento(player))
    {
        sMensajeMgr->AnunciarMensajeEntrada(player, BR_MENSAJE_ERROR_EN_EVENTO);
        return;
    }
    switch (estadoActual)
    {
    case BR_ESTADO_SIN_SUFICIENTES_JUGADORES:
    {
        list_Cola[guid] = player;
        if (HaySuficientesEnCola())
        {
            IniciarNuevaRonda();
        }
        else
        {
            sMensajeMgr->AnunciarJugadoresEnCola(player, conf_JugadoresMinimo, list_Cola);
        }
        break;
    }
    case BR_ESTADO_INVITANDO_JUGADORES:
    {
        if (EstaLlenoElEvento())
        {
            list_Cola[guid] = player;
            sMensajeMgr->AnunciarJugadoresEnCola(player, conf_JugadoresMinimo, list_Cola, BR_MENSAJE_ESTADO_EVENTO_LLENO);
        }
        else
        {
            uint32 tiempo = (tiempoRestanteInicio - 75) * IN_MILLISECONDS;
            list_Invitados[guid] = player;
            LlamarDentroDeNave(guid, tiempo);
        }
        break;
    }
    default:
    {
        list_Cola[guid] = player;
        sMensajeMgr->AnunciarJugadoresEnCola(player, conf_JugadoresMinimo, list_Cola, BR_MENSAJE_ESTADO_EVENTO_EN_CURSO);
        break;
    }
    }
}

void EventoMgr::GestionarJugadorDesconectar(Player *player)
{
    if (EstaEnEvento(player) || EstaEnCola(player) || EstaInvitado(player) || EstaEnListaDeAlas(player))
        SalirDelEvento(player->GetGUID().GetCounter(), true);
}

void EventoMgr::GestionarMuerteJcJ(Player *killer, Player *killed)
{
    if (HayJugadores() && estadoActual >= BR_ESTADO_ZONA_EN_ESPERA && estadoActual < BR_ESTADO_BATALLA_TERMINADA)
    {
        if (!killer || !killed || !EstaEnEvento(killer) || !EstaEnEvento(killed))
            return;
        sSonidoMgr->ReproducirSonidoParaTodos(BR_SONIDO_ALGUIEN_MUERE, list_Jugadores);
        if (killer == killed)
        {
            sBRRecompensaMgr->AcumularRecompensa(conf_Recompensa.morir, &(list_Datos[killed->GetGUID().GetCounter()]));
            sMensajeMgr->AnunciarMuerteJcJ(killer, killed, 0, list_Jugadores);
            return;
        }
        killer->AddAura(BR_HECHIZO_DESGARRO_ASESINO, killer);
        list_Datos[killer->GetGUID().GetCounter()].kills++;
        totalAsesinatosJcJ++;
        sBRRecompensaMgr->AcumularRecompensa(conf_Recompensa.asesinar, &(list_Datos[killer->GetGUID().GetCounter()]));
        sBRRecompensaMgr->AcumularRecompensa(conf_Recompensa.serAsesinado, &(list_Datos[killed->GetGUID().GetCounter()]));
        sMensajeMgr->AnunciarMuerteJcJ(killer, killed, list_Datos[killer->GetGUID().GetCounter()].kills, list_Jugadores);
        TodosLosMuertosEspectarme(killer);
    }
}

void EventoMgr::GestionarActualizacionMundo(uint32 diff)
{
    if (indicadorQuitarObjetosProgramado <= 0)
    {
        indicadorQuitarObjetosProgramado = 500;
        QuitarTodosLosObjetosProgramado();
    }
    else
    {
        indicadorQuitarObjetosProgramado -= diff;
    }
    if (!EstaActivado())
        return;
    if (indicadorDeSegundos <= 0)
    {
        indicadorDeSegundos = 1000;
        switch (estadoActual)
        {
        case BR_ESTADO_SIN_SUFICIENTES_JUGADORES:
        {
            if (--tiempoRestanteSinJugadores <= 0)
            {
                tiempoRestanteSinJugadores = conf_IntervaloSinJugadores;
                seHaAnunciadoInicioForzado = false;
                if (sEventoMgr->ForzarIniciarNuevaRonda())
                {
                    sMensajeMgr->AnunciarInicioForzado(list_Jugadores.size());
                }
                else
                {
                    sMensajeMgr->AnunciarErrorInicioForzado();
                }
            }
            else if (!seHaAnunciadoInicioForzado && tiempoRestanteSinJugadores <= 300 /* TODO: Configurable. */)
            {
                seHaAnunciadoInicioForzado = true;
                sMensajeMgr->AnunciarAvisoInicioForzado();
            }
            break;
        }
        case BR_ESTADO_INVITANDO_JUGADORES:
        case BR_ESTADO_ESPERANDO_JUGADORES:
        case BR_ESTADO_NAVE_EN_ESPERA:
        {
            tiempoRestanteInicio--;
            DarObjetosInicialesProgramado();
            if (tiempoRestanteInicio <= 45)
            {
                if (sEntidadMgr->EncenderNave())
                {
                    estadoActual = BR_ESTADO_NAVE_EN_MOVIMIENTO;
                    if (!list_Invitados.empty())
                    {
                        while (!list_Invitados.empty())
                        {
                            uint32 guid = list_Invitados.begin()->first;
                            list_Invitados.erase(guid);
                            sMapaMgr->RemoverVoto(guid);
                            sMapaMgr->LimpiarVoto(guid);
                        }
                    }
                }
                else
                {
                    RestablecerTodoElEvento();
                }
            }
            else if (tiempoRestanteInicio <= 75)
            {
                if (estadoActual == BR_ESTADO_ESPERANDO_JUGADORES)
                {
                    estadoActual = BR_ESTADO_NAVE_EN_ESPERA;
                }
            }
            else if (tiempoRestanteInicio <= 90)
            {
                if (estadoActual == BR_ESTADO_INVITANDO_JUGADORES)
                {
                    estadoActual = BR_ESTADO_ESPERANDO_JUGADORES;
                }
            }
            TODO_MejorarAnuncioEnNave();
            break;
        }
        case BR_ESTADO_NAVE_EN_MOVIMIENTO:
        {
            VerificarJugadoresEnNave();
            if (--tiempoRestanteInicio <= 20)
            {
                estadoActual = BR_ESTADO_NAVE_POR_DESAPARECER;
                escalaDeZona = 15.2f;
                tiempoRestanteZona = 0;
                estaZonaAnunciada5s = false;
                estaZonaAnunciada10s = false;
                if (!HayJugadores() || !sEntidadMgr->InvocarZona(sMapaMgr->MapaActual()->idMapa, sMapaMgr->MapaActual()->centroMapa))
                {
                    RestablecerTodoElEvento();
                    return;
                }
                else
                {
                    AlReducirseLaZona();
                }
            }
            TODO_MejorarAnuncioEnNave();
            break;
        }
        case BR_ESTADO_NAVE_POR_DESAPARECER:
        {
            if (--tiempoRestanteInicio <= 0)
            {
                estadoActual = BR_ESTADO_ZONA_EN_ESPERA;
                sBRRecompensaMgr->AcumularRecompensaVivos(conf_Recompensa.base, list_Jugadores, &list_Datos);
                sSonidoMgr->ReproducirSonidoParaTodos(BR_SONIDO_RONDA_INICIADA, list_Jugadores);
                sMensajeMgr->NotificarTiempoInicial(0, list_Jugadores, sMapaMgr->MapaActual()->nombreMapa);
                sBRMisionesMgr->CompletarRequerimiento(MISION_DIARIA_1, MISION_DIARIA_1_REQ_1, list_Jugadores);
                tiempoRestanteZona = conf_IntervaloZonaSegura;
                tiempoRestanteNave = 15;
            }
            else
            {
                TODO_MejorarAnuncioEnNave();
            }
            break;
        }
        case BR_ESTADO_ZONA_EN_ESPERA:
        case BR_ESTADO_ZONA_EN_REDUCCION:
        case BR_ESTADO_ZONA_DESAPARECIDA:
        {
            ControlDeReglas();
            if (!CondicionDeVictoria())
            {
                EfectoFueraDeZona();
                ActivarTodosContraTodos();
                VerificarEspectadores();
            }
            if (--tiempoRestanteNave <= 0)
            {
                if (sEntidadMgr->DesaparecerNave())
                {
                    sSonidoMgr->ReproducirSonidoParaTodos(BR_SONIDO_NAVE_RETIRADA, list_Jugadores);
                    sMensajeMgr->NotificarNaveRetirada(list_Jugadores);
                }
            }
            if (tiempoRestanteZona <= 0)
            {
                if (estadoActual == BR_ESTADO_ZONA_EN_ESPERA && sEntidadMgr->EstaLaZonaActiva())
                {
                    reducirZonaHasta = escalaDeZona - 3.0f;
                    if (reducirZonaHasta < 0.2f)
                        reducirZonaHasta = 0.2f;
                    estadoActual = BR_ESTADO_ZONA_EN_REDUCCION;
                }
                AlReducirseLaZona();
                sBRRecompensaMgr->AcumularRecompensaVivos(conf_Recompensa.zona, list_Jugadores, &list_Datos);
                sSonidoMgr->ReproducirSonidoParaTodos(BR_SONIDO_ZONA_REDUCIDA, list_Jugadores);
                sMensajeMgr->NotificarZonaEnReduccion(list_Jugadores);
                tiempoRestanteZona = conf_IntervaloZonaSegura;
                estaZonaAnunciada5s = false;
                estaZonaAnunciada10s = false;
            }
            else
            {
                if (estadoActual == BR_ESTADO_ZONA_EN_ESPERA)
                {
                    if (!estaZonaAnunciada5s && tiempoRestanteZona <= 5)
                    {
                        sSonidoMgr->ReproducirSonidoParaTodos(BR_SONIDO_ZONA_TIEMPO, list_Jugadores);
                        sMensajeMgr->NotificarAdvertenciaDeZona(5, list_Jugadores);
                        estaZonaAnunciada5s = true;
                    }
                    if (!estaZonaAnunciada10s && tiempoRestanteZona <= 10)
                    {
                        sSonidoMgr->ReproducirSonidoParaTodos(BR_SONIDO_ZONA_TIEMPO, list_Jugadores);
                        sMensajeMgr->NotificarAdvertenciaDeZona(10, list_Jugadores);
                        estaZonaAnunciada10s = true;
                    }
                    tiempoRestanteZona--;
                }
            }
            break;
        }
        case BR_ESTADO_BATALLA_TERMINADA:
        {
            if (--tiempoRestanteFinal <= 0)
            {
                while (HayJugadores())
                    SalirDelEvento(list_Jugadores.begin()->first);
                tiempoRestanteSinJugadores = conf_IntervaloSinJugadores;
                seHaAnunciadoInicioForzado = false;
                estadoActual = BR_ESTADO_SIN_SUFICIENTES_JUGADORES;
                if (HaySuficientesEnCola())
                    IniciarNuevaRonda();
            }
            break;
        }
        }
    }
    else
    {
        indicadorDeSegundos -= diff;
    }
    if (estadoActual == BR_ESTADO_ZONA_EN_REDUCCION)
    {
        if (indicadorDe100msZona <= 0)
        {
            indicadorDe100msZona = 100;
            if (reducirZonaHasta < escalaDeZona)
            {
                sEntidadMgr->ActualizarZona(escalaDeZona);
            }
            else
            {
                if (sEntidadMgr->EstaLaZonaActiva())
                {
                    estadoActual = BR_ESTADO_ZONA_EN_ESPERA;
                }
                else
                {
                    estadoActual = BR_ESTADO_ZONA_DESAPARECIDA;
                }
            }
        }
        else
        {
            indicadorDe100msZona -= diff;
        }
    }
}

void EventoMgr::PrevenirJcJEnLaNave(Player *player, bool state)
{
    if (state && HayJugadores() && EstaEnEvento(player) && !TodosContraTodos(player))
        player->SetPvP(false);
}

bool EventoMgr::PuedeReaparecerEnCementerio(Player *player)
{
    if (HayJugadores() && EstaEnEvento(player))
    {
        SalirDelEvento(player->GetGUID().GetCounter());
        return false;
    }
    return true;
}

void EventoMgr::RestablecerTodoElEvento()
{
    list_Cola.clear();
    list_Invitados.clear();
    list_Jugadores.clear();
    list_Datos.clear();
    list_DarObjetosIniciales.clear();
    list_QuitarTodosLosObjetos.clear();
    sMapaMgr->SiguienteMapa();
    indicadorDeSegundos = 1000;
    indicadorDe100msZona = 100;
    indicadorQuitarObjetosProgramado = 500;
    indiceDeVariacion = 0;
    totalAsesinatosJcJ = 0;
    sEntidadMgr->DesaparecerTodosLosObjetos();
    tiempoRestanteSinJugadores = conf_IntervaloSinJugadores;
    seHaAnunciadoInicioForzado = false;
    estadoActual = BR_ESTADO_SIN_SUFICIENTES_JUGADORES;
}

void EventoMgr::IniciarNuevaRonda()
{
    if (estadoActual == BR_ESTADO_SIN_SUFICIENTES_JUGADORES)
    {
        sMapaMgr->EstablecerMasVotado();
        tiempoRestanteInicio = 120; // TODO: Configurale: Es la suma rara de los tiempos iniciales.
        if (!HayCola() || !sEntidadMgr->InvocarNave(sMapaMgr->MapaActual()->idMapa, sMapaMgr->MapaActual()->inicioNave))
        {
            RestablecerTodoElEvento();
            return;
        }
        estadoActual = BR_ESTADO_INVITANDO_JUGADORES;
        totalAsesinatosJcJ = 0;
        while (HayCola() && !EstaLlenoElEvento() && tiempoRestanteInicio >= 90 /* TODO: Tiempo total - Tiempo BR_ESTADO_INVITANDO_JUGADORES. */)
        {
            uint32 guid = list_Cola.begin()->first;
            uint32 tiempo = (tiempoRestanteInicio - 75 /* TODO: Espera + Movimiento + Desaparecer. */) * IN_MILLISECONDS;
            list_Invitados[guid] = list_Cola[guid];
            LlamarDentroDeNave(guid, tiempo);
            list_Cola.erase(guid);
        }
    }
}

void EventoMgr::AlmacenarPosicionInicial(uint32 guid)
{
    if (!list_Jugadores[guid]->GetMap() || list_Jugadores[guid]->GetMap()->Instanceable() || list_Jugadores[guid]->GetTransport())
    {
        list_Datos[guid].SetPosition(list_Jugadores[guid]->m_homebindMapId, list_Jugadores[guid]->m_homebindX, list_Jugadores[guid]->m_homebindY, list_Jugadores[guid]->m_homebindZ, list_Jugadores[guid]->GetOrientation());
    }
    else
    {
        list_Datos[guid].SetPosition(list_Jugadores[guid]->GetMapId(), list_Jugadores[guid]->GetPositionX(), list_Jugadores[guid]->GetPositionY(), list_Jugadores[guid]->GetPositionZ(), list_Jugadores[guid]->GetOrientation());
    }
}

void EventoMgr::LlamarDentroDeNave(uint32 guid, uint32 tiempo /* = 20000*/)
{
    if (list_Invitados.find(guid) == list_Invitados.end())
        return;
    Player *player = list_Invitados[guid];
    float ox = BR_VARIACIONES_POSICION[indiceDeVariacion][0];
    float oy = BR_VARIACIONES_POSICION[indiceDeVariacion][1];
    BRMapa *brM = sMapaMgr->MapaActual();
    Position iN = brM->inicioNave;

    player->SetSummonPoint(brM->idMapa, iN.GetPositionX() + ox, iN.GetPositionY() + oy, iN.GetPositionZ() + 2.5f);
    WorldPacket data(SMSG_SUMMON_REQUEST, 8 + 4 + 4);
    data << sCharacterCache->GetCharacterGuidByName("BattleRoyale");
    data << uint32(brM->idZona);
    data << uint32(tiempo);
    player->GetSession()->SendPacket(&data);

    SiguientePosicion();
}

void EventoMgr::RespondeInvitacion(Player *player, bool agree, ObjectGuid summoner_guid)
{
    if (!player || !EstaInvitado(player))
        return;
    if (sCharacterCache->GetCharacterGuidByName("BattleRoyale") != summoner_guid)
        return;
    uint32 guid = player->GetGUID().GetCounter();
    if (!agree)
    {
        list_Invitados.erase(guid);
        sMapaMgr->RemoverVoto(guid);
        sMapaMgr->LimpiarVoto(guid);
        if ((estadoActual == BR_ESTADO_INVITANDO_JUGADORES || estadoActual == BR_ESTADO_ESPERANDO_JUGADORES) && tiempoRestanteInicio >= 60)
        {
            while (HayCola() && !EstaLlenoElEvento() && tiempoRestanteInicio >= 60)
            {
                uint32 guid = list_Cola.begin()->first;
                uint32 tiempo = (tiempoRestanteInicio - 55) * IN_MILLISECONDS;
                list_Invitados[guid] = list_Cola[guid];
                LlamarDentroDeNave(guid, tiempo);
                list_Cola.erase(guid);
            }
        }
        return;
    }
    list_Jugadores[guid] = list_Invitados[guid];
    list_Invitados.erase(guid);
    AlmacenarPosicionInicial(guid);
    if (player->HasAura(BR_HECHIZO_PARACAIDAS))
        player->RemoveAurasDueToSpell(BR_HECHIZO_PARACAIDAS);
    if (player->HasAura(BR_HECHIZO_PARACAIDAS_EFECTO))
        player->RemoveAurasDueToSpell(BR_HECHIZO_PARACAIDAS_EFECTO);
    DejarGrupo(player);
    Desmontar(player);
    player->SetPhaseMask(BR_VISIBILIDAD_EVENTO, true);
    player->SetOrientation(sMapaMgr->MapaActual()->inicioNave.GetOrientation() + M_PI / 2.0f);
    player->SetPvP(false);
    player->SaveToDB(false, false);
    list_DarObjetosIniciales[guid] = player;
}

void EventoMgr::SalirDelEvento(uint32 guid, bool logout /* = false*/)
{
    if (EstaEnListaDarObjetosIniciales(guid))
        list_DarObjetosIniciales.erase(guid);
    if (EstaEnCola(guid))
    {
        list_Cola.erase(guid);
        sMapaMgr->RemoverVoto(guid);
    }
    if (EstaInvitado(guid))
    {
        list_Invitados.erase(guid);
        sMapaMgr->RemoverVoto(guid);
    }
    sMapaMgr->LimpiarVoto(guid);
    if (EstaEnEvento(guid))
    {
        Player *player = list_Jugadores[guid];
        player->SetPhaseMask(BR_VISIBILIDAD_NORMAL, true);
        if (player->IsAlive())
        {
            player->RemoveAurasDueToSpell(BR_HECHIZO_ALAS_MAGICAS);
            player->RemoveAurasDueToSpell(BR_HECHIZO_ANTI_INVISIBLES);
            player->RemoveAurasDueToSpell(BR_HECHIZO_ANTI_SANADORES);
            player->RemoveAurasDueToSpell(BR_HECHIZO_RASTRILLO_LENTO);
            player->RemoveAurasDueToSpell(BR_HECHIZO_DESGARRO_ASESINO);
            player->RemoveAurasDueToSpell(BR_HECHIZO_ACIDO_ZONA);
            player->RemoveAurasDueToSpell(BR_HECHIZO_BENEFICIO_LIEBRE);
        }
        if (!logout)
        {
            if (!player->IsAlive())
                RevivirJugador(player);
            if (!player->isPossessing())
                player->StopCastingBindSight();
            list_QuitarTodosLosObjetos[guid] = player;
            player->AddAura(BR_HECHIZO_PARACAIDAS, player);
            player->TeleportTo(list_Datos[guid].GetMap(), list_Datos[guid].GetX(), list_Datos[guid].GetY(), list_Datos[guid].GetZ(), list_Datos[guid].GetO());
            player->SaveToDB(false, false);
        }
        sBRRecompensaMgr->DarRecompensas(player, list_Datos[guid].reward);
        list_Jugadores.erase(guid);
        list_Datos.erase(guid);
    }
    if (logout && EstaEnListaQuitarTodosLosObjetos(guid))
        list_QuitarTodosLosObjetos.erase(guid);
}

void EventoMgr::RevivirJugador(Player *player)
{
    player->ResurrectPlayer(1.0f);
    player->SpawnCorpseBones();
    player->SaveToDB(false, false);
}

void EventoMgr::EfectoFueraDeZona()
{
    if (HayJugadores())
    {
        for (BRListaPersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
        {
            if (it->second && it->second->IsAlive() && sEntidadMgr->EstaLaZonaActiva())
            {
                float distance = sEntidadMgr->DistanciaDelCentro(it->second);
                float f_RadioZona = escalaDeZona * 19.0f;
                if (estadoActual == BR_ESTADO_ZONA_DESAPARECIDA || f_RadioZona < distance)
                {
                    int tick = list_Datos[it->first].dmg_tick;
                    if ((tick <= 57 && tick % 3 == 0) || (tick > 57 && tick % 6 == 0))
                    {
                        it->second->AddAura(56710, it->second); // Aura de esperanza perdida: x20 -3% salud max.
                        it->second->AddAura(50196, it->second); // Toque de putrfaccion: x20 dmg 88-112.
                        it->second->AddAura(60084, it->second); // EL velo de las sombras: -50% sanaciones.
                    }
                    if (tick >= 57)
                    {
                        if ((tick <= 102 && tick % 3 == 0) || (tick > 102 && tick % 6 == 0))
                        {
                            it->second->AddAura(61460, it->second); // Aura de esperanza perdida: x15 -5% salud max.
                        }
                        if (tick >= 102)
                        {
                            if ((tick <= 132 && tick % 3 == 0) || (tick > 132 && tick % 24 == 0))
                            {
                                it->second->AddAura(19771, it->second); // Mordedura cerrada: x10 dmg 1500 en 30seg.
                            }
                        }
                    }
                    it->second->GetSession()->SendNotification("|cffff0000¡Entra en la zona segura!");
                    list_Datos[it->first].dmg_tick++;
                }
                else
                {
                    list_Datos[it->first].dmg_tick = 0;
                }
            }
        }
    }
}

void EventoMgr::ActivarTodosContraTodos()
{
    if (HayJugadores())
    {
        for (BRListaPersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
        {
            if (it->second && it->second->IsAlive() && TodosContraTodos(it->second) && !(it->second->HasByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP)))
            {
                it->second->SetByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP);
            }
        }
    }
}

void EventoMgr::ControlDeReglas()
{
    if (HayJugadores())
    {
        BRListaPersonajes::iterator it = list_Jugadores.begin();
        while (it != list_Jugadores.end())
        {
            if (it->second && it->second->IsAlive())
            {
                if (sEntidadMgr->DistanciaDelCentro(it->second) > 1147.0f)
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

bool EventoMgr::CondicionDeVictoria()
{
    if (estadoActual >= BR_ESTADO_ZONA_EN_ESPERA && estadoActual < BR_ESTADO_BATALLA_TERMINADA)
    {
        if (HayJugadores())
        {
            int cantidadVivos = 0;
            Player *vivo = nullptr;
            for (BRListaPersonajes::iterator it = list_Jugadores.begin(); it != list_Jugadores.end(); ++it)
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

void EventoMgr::FinalizarRonda(bool announce, Player *winner /* = nullptr*/)
{
    if (announce && winner && EstaEnEvento(winner))
    {
        if (winner->GetTeamId() == TEAM_ALLIANCE)
        {
            sSonidoMgr->ReproducirSonidoParaTodos(BR_SONIDO_GANADOR_ALIANZA, list_Jugadores);
        }
        else
        {
            sSonidoMgr->ReproducirSonidoParaTodos(BR_SONIDO_GANADOR_HORDA, list_Jugadores);
        }
        if (list_Datos.find(winner->GetGUID().GetCounter()) != list_Datos.end())
        {
            sBRRecompensaMgr->AcumularRecompensa(conf_Recompensa.victoria, &(list_Datos[winner->GetGUID().GetCounter()]));
        }
        sMensajeMgr->AnunciarGanador(winner, list_Datos[winner->GetGUID().GetCounter()].kills);
        TodosLosMuertosEspectarme(winner);
        if (conf_RequisitoAsesinatosTotales <= totalAsesinatosJcJ && conf_RequisitoAsesinatosPropios <= list_Datos[winner->GetGUID().GetCounter()].kills)
        {
            sTituloMgr->Ascender(winner);
        }
    }
    else
    {
        sMensajeMgr->AnunciarEmpate();
    }
    sEntidadMgr->DesaparecerTodosLosObjetos();
    tiempoRestanteFinal = conf_IntervaloFinalDeRonda;
    sMapaMgr->SiguienteMapa();
    estadoActual = BR_ESTADO_BATALLA_TERMINADA;
}
