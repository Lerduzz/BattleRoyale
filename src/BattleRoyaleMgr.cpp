#include "BattleRoyaleMgr.h"
#include "CharacterCache.h"
#include "Config.h"
#include "Log.h"

BattleRoyaleMgr::BattleRoyaleMgr()
{
    conf_EstaActivado = sConfigMgr->GetOption<bool>("BattleRoyale.Activado", true);
    conf_JugadoresMinimo = sConfigMgr->GetOption<uint32>("BattleRoyale.MinJugadores", 25);
    conf_JugadoresMaximo = sConfigMgr->GetOption<uint32>("BattleRoyale.MaxJugadores", 50);
    conf_IntervaloSinJugadores = sConfigMgr->GetOption<uint32>("BattleRoyale.Intervalo.SinJugadores", 1800);
    conf_IntervaloZonaSegura = sConfigMgr->GetOption<uint32>("BattleRoyale.Intervalo.ZonaSegura", 60);
    conf_IntervaloFinalDeRonda = sConfigMgr->GetOption<uint32>("BattleRoyale.Intervalo.FinalDeRonda", 10);
    conf_RequisitoAsesinatosTotales = sConfigMgr->GetOption<uint32>("BattleRoyale.Requisito.AsesinatosTotales", 5);
    conf_RequisitoAsesinatosPropios = sConfigMgr->GetOption<uint32>("BattleRoyale.Requisito.AsesinatosPropios", 1);

    conf_Recompensa = BRConf_Recompensa();
    conf_Recompensa.base = sConfigMgr->GetOption<uint32>("BattleRoyale.Recompensa.Base", 400);
    conf_Recompensa.victoria = sConfigMgr->GetOption<uint32>("BattleRoyale.Recompensa.Victoria", 600);
    conf_Recompensa.asesinar = sConfigMgr->GetOption<uint32>("BattleRoyale.Recompensa.AlAsesinar", 350);
    conf_Recompensa.serAsesinado = sConfigMgr->GetOption<uint32>("BattleRoyale.Recompensa.AlSerEAsesinado", 200);
    conf_Recompensa.morir = sConfigMgr->GetOption<uint32>("BattleRoyale.Recompensa.AlMorir", 100);
    conf_Recompensa.zona = sConfigMgr->GetOption<uint32>("BattleRoyale.Recompensa.ZonaReducida", 150);

    sBRMapasMgr->CargarMapasDesdeBD();
    RestablecerTodoElEvento();
}

BattleRoyaleMgr::~BattleRoyaleMgr()
{
    RestablecerTodoElEvento();
}

void BattleRoyaleMgr::GestionarJugadorEntrando(Player *player)
{
    if (!player)
        return;
    uint32 guid = player->GetGUID().GetCounter();
    BR_Bloqueado *blr = sBRListaNegraMgr->EstaBloqueado(guid);
    if (blr->estaBloqueado)
    {
        sBRChatMgr->AnunciarMensajeEntrada(player, MENSAJE_ERROR_BLOQUEADO, blr->motivo);
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
        if (HaySuficientesEnCola())
        {
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
        if (EstaLlenoElEvento())
        {
            list_Cola[guid] = player;
            sBRChatMgr->AnunciarJugadoresEnCola(player, conf_JugadoresMinimo, list_Cola, MENSAJE_ESTADO_EVENTO_LLENO);
        }
        else
        {
            if (tiempoRestanteInicio >= 60)
            {
                uint32 tiempo = (tiempoRestanteInicio - 55) * IN_MILLISECONDS;
                list_Invitados[guid] = player;
                LlamarDentroDeNave(guid, tiempo);
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

void BattleRoyaleMgr::GestionarJugadorDesconectar(Player *player)
{
    if (EstaEnEvento(player) || EstaEnCola(player) || EstaInvitado(player) || EstaEnListaDeAlas(player))
        SalirDelEvento(player->GetGUID().GetCounter(), true);
}

void BattleRoyaleMgr::GestionarMuerteJcJ(Player *killer, Player *killed)
{
    if (HayJugadores() && estadoActual == ESTADO_BATALLA_EN_CURSO)
    {
        if (!killer || !killed || !EstaEnEvento(killer) || !EstaEnEvento(killed))
            return;
        sBRSonidosMgr->ReproducirSonidoParaTodos(SONIDO_ALGUIEN_MUERE, list_Jugadores);
        if (killer == killed)
        {
            sBRRecompensaMgr->AcumularRecompensa(conf_Recompensa.morir, &(list_Datos[killed->GetGUID().GetCounter()]));
            sBRChatMgr->AnunciarMuerteJcJ(killer, killed, 0, list_Jugadores);
            return;
        }
        killer->AddAura(HECHIZO_DESGARRO_ASESINO, killer);
        list_Datos[killer->GetGUID().GetCounter()].kills++;
        totalAsesinatosJcJ++;
        sBRRecompensaMgr->AcumularRecompensa(conf_Recompensa.asesinar, &(list_Datos[killer->GetGUID().GetCounter()]));
        sBRRecompensaMgr->AcumularRecompensa(conf_Recompensa.serAsesinado, &(list_Datos[killed->GetGUID().GetCounter()]));
        sBRChatMgr->AnunciarMuerteJcJ(killer, killed, list_Datos[killer->GetGUID().GetCounter()].kills, list_Jugadores);
        TodosLosMuertosEspectarme(killer);
    }
}

void BattleRoyaleMgr::GestionarActualizacionMundo(uint32 diff)
{
    if (indicadorDeSegundos <= 0)
    {
        indicadorDeSegundos = 1000;
        QuitarTodosLosObjetosProgramado();
        switch (estadoActual)
        {
        case ESTADO_NO_HAY_SUFICIENTES_JUGADORES:
        {
            if (--tiempoRestanteSinJugadores <= 0)
            {
                tiempoRestanteSinJugadores = conf_IntervaloSinJugadores;
                seHaAnunciadoInicioForzado = false;
                if (sBattleRoyaleMgr->ForzarIniciarNuevaRonda())
                {
                    sBRChatMgr->AnunciarInicioForzado(list_Jugadores.size());
                }
                else
                {
                    sBRChatMgr->AnunciarErrorInicioForzado();
                }
            }
            else if (!seHaAnunciadoInicioForzado && tiempoRestanteSinJugadores <= 300)
            {
                seHaAnunciadoInicioForzado = true;
                sBRChatMgr->AnunciarAvisoInicioForzado();
            }
            break;
        }
        case ESTADO_INVOCANDO_JUGADORES:
        {
            DarObjetosInicialesProgramado();
            if (--tiempoRestanteInicio <= 45)
            {
                if (sBRObjetosMgr->EncenderNave())
                {
                    estadoActual = ESTADO_NAVE_EN_MOVIMIENTO;
                    if (!list_Invitados.empty())
                    {
                        while (!list_Invitados.empty())
                        {
                            uint32 guid = list_Invitados.begin()->first;
                            list_Invitados.erase(guid);
                            sBRMapasMgr->RemoverVoto(guid);
                            sBRMapasMgr->LimpiarVoto(guid);
                        }
                    }
                }
                else
                {
                    RestablecerTodoElEvento();
                }
            }
            TODO_MejorarAnuncioEnNave();
            break;
        }
        case ESTADO_NAVE_EN_MOVIMIENTO:
        {
            VerificarJugadoresEnNave();
            if (--tiempoRestanteInicio <= 20)
            {
                estadoActual = ESTADO_NAVE_CERCA_DEL_CENTRO;
                indiceDeZona = 0;
                tiempoRestanteZona = 0;
                estaZonaAnunciada5s = false;
                estaZonaAnunciada10s = false;
                if (!HayJugadores() || !sBRObjetosMgr->InvocarCentroDelMapa(sBRMapasMgr->MapaActual()->idMapa, sBRMapasMgr->MapaActual()->centroMapa))
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
        case ESTADO_NAVE_CERCA_DEL_CENTRO:
        {
            if (--tiempoRestanteInicio <= 0)
            {
                estadoActual = ESTADO_BATALLA_EN_CURSO;
                sBRRecompensaMgr->AcumularRecompensaVivos(conf_Recompensa.base, list_Jugadores, &list_Datos);
                sBRSonidosMgr->ReproducirSonidoParaTodos(SONIDO_RONDA_INICIADA, list_Jugadores);
                sBRChatMgr->NotificarTiempoInicial(0, list_Jugadores, sBRMapasMgr->MapaActual()->nombreMapa);
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
        case ESTADO_BATALLA_EN_CURSO:
        {
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
            if (tiempoRestanteZona <= 0)
            {
                // if (!HayJugadores() || !sBRObjetosMgr->InvocarZonaSegura(sBRMapasMgr->MapaActual()->idMapa, sBRMapasMgr->MapaActual()->centroMapa, indiceDeZona))
                // {
                //     RestablecerTodoElEvento();
                //     return;
                // }
                AlReducirseLaZona();
                sBRRecompensaMgr->AcumularRecompensaVivos(conf_Recompensa.zona, list_Jugadores, &list_Datos);
                sBRSonidosMgr->ReproducirSonidoParaTodos(SONIDO_ZONA_REDUCIDA, list_Jugadores);
                sBRChatMgr->NotificarZonaReducida(list_Jugadores);
                tiempoRestanteZona = conf_IntervaloZonaSegura;
                estaZonaAnunciada5s = false;
                estaZonaAnunciada10s = false;
            }
            else
            {
                if (!estaZonaAnunciada5s && tiempoRestanteZona <= 5)
                {
                    sBRSonidosMgr->ReproducirSonidoParaTodos(SONIDO_ZONA_TIEMPO, list_Jugadores);
                    sBRChatMgr->NotificarAdvertenciaDeZona(5, list_Jugadores);
                    estaZonaAnunciada5s = true;
                }
                if (!estaZonaAnunciada10s && tiempoRestanteZona <= 10)
                {
                    sBRSonidosMgr->ReproducirSonidoParaTodos(SONIDO_ZONA_TIEMPO, list_Jugadores);
                    sBRChatMgr->NotificarAdvertenciaDeZona(10, list_Jugadores);
                    estaZonaAnunciada10s = true;
                }
                if (indiceDeZona <= CANTIDAD_DE_ZONAS)
                {
                    tiempoRestanteZona--;
                }
            }
            break;
        }
        case ESTADO_BATALLA_TERMINADA:
        {
            if (--tiempoRestanteFinal <= 0)
            {
                while (HayJugadores())
                    SalirDelEvento(list_Jugadores.begin()->first);
                tiempoRestanteSinJugadores = conf_IntervaloSinJugadores;
                seHaAnunciadoInicioForzado = false;
                estadoActual = ESTADO_NO_HAY_SUFICIENTES_JUGADORES;
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
}

void BattleRoyaleMgr::PrevenirJcJEnLaNave(Player *player, bool state)
{
    if (state && HayJugadores() && EstaEnEvento(player) && !DebeForzarJcJTcT(player))
        player->SetPvP(false);
}

bool BattleRoyaleMgr::PuedeReaparecerEnCementerio(Player *player)
{
    if (HayJugadores() && EstaEnEvento(player))
    {
        SalirDelEvento(player->GetGUID().GetCounter());
        return false;
    }
    return true;
}

void BattleRoyaleMgr::RestablecerTodoElEvento()
{
    list_Cola.clear();
    list_Invitados.clear();
    list_Jugadores.clear();
    list_Datos.clear();
    list_DarObjetosIniciales.clear();
    list_QuitarTodosLosObjetos.clear();
    sBRMapasMgr->SiguienteMapa();
    indicadorDeSegundos = 1000;
    indiceDeVariacion = 0;
    totalAsesinatosJcJ = 0;
    sBRObjetosMgr->DesaparecerTodosLosObjetos();
    tiempoRestanteSinJugadores = conf_IntervaloSinJugadores;
    seHaAnunciadoInicioForzado = false;
    estadoActual = ESTADO_NO_HAY_SUFICIENTES_JUGADORES;
}

void BattleRoyaleMgr::IniciarNuevaRonda()
{
    if (estadoActual == ESTADO_NO_HAY_SUFICIENTES_JUGADORES)
    {
        sBRMapasMgr->EstablecerMasVotado();
        tiempoRestanteInicio = 75;
        if (!HayCola() || !sBRObjetosMgr->InvocarNave(sBRMapasMgr->MapaActual()->idMapa, sBRMapasMgr->MapaActual()->inicioNave))
        {
            RestablecerTodoElEvento();
            return;
        }
        estadoActual = ESTADO_INVOCANDO_JUGADORES;
        totalAsesinatosJcJ = 0;
        while (HayCola() && !EstaLlenoElEvento() && tiempoRestanteInicio >= 60)
        {
            uint32 guid = list_Cola.begin()->first;
            if (list_Cola[guid]->IsInFlight())
            {
                sBRChatMgr->AnunciarMensajeEntrada(list_Cola[guid], MENSAJE_ERROR_EN_VUELO);
            }
            else
            {
                uint32 tiempo = (tiempoRestanteInicio - 55) * IN_MILLISECONDS;
                list_Invitados[guid] = list_Cola[guid];
                LlamarDentroDeNave(guid, tiempo);
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

void BattleRoyaleMgr::LlamarDentroDeNave(uint32 guid, uint32 tiempo /* = 20000*/)
{
    Player *player = list_Invitados[guid];
    float ox = BR_VariacionesDePosicion[indiceDeVariacion][0];
    float oy = BR_VariacionesDePosicion[indiceDeVariacion][1];
    BR_Mapa *brM = sBRMapasMgr->MapaActual();
    Position iN = brM->inicioNave;

    player->SetSummonPoint(brM->idMapa, iN.GetPositionX() + ox, iN.GetPositionY() + oy, iN.GetPositionZ() + 2.5f);
    WorldPacket data(SMSG_SUMMON_REQUEST, 8 + 4 + 4);
    data << sCharacterCache->GetCharacterGuidByName("BattleRoyale");
    data << uint32(brM->idZona);
    data << uint32(tiempo);
    player->GetSession()->SendPacket(&data);

    SiguientePosicion();
}

void BattleRoyaleMgr::OnSummonResponse(Player *player, bool agree, ObjectGuid summoner_guid)
{
    if (!player || !EstaInvitado(player))
        return;
    uint32 guid = player->GetGUID().GetCounter();
    if (!agree)
    {
        list_Invitados.erase(guid);
        sBRMapasMgr->RemoverVoto(guid);
        sBRMapasMgr->LimpiarVoto(guid);
        if (estadoActual == ESTADO_INVOCANDO_JUGADORES && tiempoRestanteInicio >= 60)
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
    if (player->HasAura(HECHIZO_PARACAIDAS))
        player->RemoveAurasDueToSpell(HECHIZO_PARACAIDAS);
    if (player->HasAura(HECHIZO_PARACAIDAS_EFECTO))
        player->RemoveAurasDueToSpell(HECHIZO_PARACAIDAS_EFECTO);
    DejarGrupo(player);
    Desmontar(player);
    player->SetPhaseMask(DIMENSION_EVENTO, true);
    player->SetOrientation(sBRMapasMgr->MapaActual()->inicioNave.GetOrientation() + M_PI / 2.0f);
    player->SetPvP(false);
    player->SaveToDB(false, false);
    list_DarObjetosIniciales[guid] = player;
}

void BattleRoyaleMgr::SalirDelEvento(uint32 guid, bool logout /* = false*/)
{
    if (EstaEnListaDarObjetosIniciales(guid))
        list_DarObjetosIniciales.erase(guid);
    if (EstaEnCola(guid))
    {
        list_Cola.erase(guid);
        sBRMapasMgr->RemoverVoto(guid);
    }
    if (EstaInvitado(guid))
    {
        list_Invitados.erase(guid);
        sBRMapasMgr->RemoverVoto(guid);
    }
    sBRMapasMgr->LimpiarVoto(guid);
    if (EstaEnEvento(guid))
    {
        Player *player = list_Jugadores[guid];
        player->SetPhaseMask(DIMENSION_NORMAL, true);
        if (player->IsAlive())
        {
            if (player->HasAura(HECHIZO_ALAS_MAGICAS))
                player->RemoveAurasDueToSpell(HECHIZO_ALAS_MAGICAS);
            if (player->HasAura(HECHIZO_ANTI_INVISIBLES))
                player->RemoveAurasDueToSpell(HECHIZO_ANTI_INVISIBLES);
            if (player->HasAura(HECHIZO_ANTI_SANADORES))
                player->RemoveAurasDueToSpell(HECHIZO_ANTI_SANADORES);
            if (player->HasAura(HECHIZO_RASTRILLO_LENTO))
                player->RemoveAurasDueToSpell(HECHIZO_RASTRILLO_LENTO);
            if (player->HasAura(HECHIZO_DESGARRO_ASESINO))
                player->RemoveAurasDueToSpell(HECHIZO_DESGARRO_ASESINO);
            if (player->HasAura(HECHIZO_ACIDO_ZONA))
                player->RemoveAurasDueToSpell(HECHIZO_ACIDO_ZONA);
        }
        if (!logout)
        {
            if (!player->IsAlive())
                RevivirJugador(player);
            if (!player->isPossessing())
                player->StopCastingBindSight();
            list_QuitarTodosLosObjetos[guid] = player;
            player->AddAura(HECHIZO_PARACAIDAS, player);
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

void BattleRoyaleMgr::RevivirJugador(Player *player)
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
                if (!sBRObjetosMgr->EstaLaZonaActiva() || (indiceDeZona > 0 && distance > BR_EscalasDeZonaSegura[indiceDeZona - 1] * 66.0f))
                {
                    list_Datos[it->first].dmg_tick++;
                    if (list_Datos[it->first].dmg_tick <= 15)
                    {
                        if (!sBRObjetosMgr->HechizoGuardian(HECHIZO_RAYO_DRAGON, it->second))
                        {
                            it->second->AddAura(HECHIZO_ACIDO_ZONA, it->second);
                        }
                    }
                    else
                    {
                        if (!sBRObjetosMgr->HechizoGuardian(HECHIZO_RAYO_DRAGON_FUERTE, it->second))
                        {
                            it->second->AddAura(HECHIZO_ACIDO_ZONA, it->second);
                        }
                    }
                    it->second->GetSession()->SendNotification("|cffff0000¡Estás fuera de la zona segura, el guardián te ataca!");
                }
                else
                {
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
    if (HayJugadores())
    {
        BR_ListaDePersonajes::iterator it = list_Jugadores.begin();
        while (it != list_Jugadores.end())
        {
            if (it->second && it->second->IsAlive())
            {
                if (sBRObjetosMgr->DistanciaDelCentro(it->second) > 1147.0f)
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
            Player *vivo = nullptr;
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

void BattleRoyaleMgr::FinalizarRonda(bool announce, Player *winner /* = nullptr*/)
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
        if (list_Datos.find(winner->GetGUID().GetCounter()) != list_Datos.end())
        {
            sBRRecompensaMgr->AcumularRecompensa(conf_Recompensa.victoria, &(list_Datos[winner->GetGUID().GetCounter()]));
        }
        sBRChatMgr->AnunciarGanador(winner, list_Datos[winner->GetGUID().GetCounter()].kills);
        TodosLosMuertosEspectarme(winner);
        if (conf_RequisitoAsesinatosTotales <= totalAsesinatosJcJ && conf_RequisitoAsesinatosPropios <= list_Datos[winner->GetGUID().GetCounter()].kills)
        {
            sBRTitulosMgr->Ascender(winner);
        }
    }
    else
    {
        sBRChatMgr->AnunciarEmpate();
    }
    sBRObjetosMgr->DesaparecerTodosLosObjetos();
    tiempoRestanteFinal = conf_IntervaloFinalDeRonda;
    sBRMapasMgr->SiguienteMapa();
    estadoActual = ESTADO_BATALLA_TERMINADA;
}
