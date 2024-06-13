#include "MapaMgr.h"

MapaMgr::MapaMgr()
{
    CargarMapasDesdeBD();
    usarVotos = false;
}

MapaMgr::~MapaMgr()
{
    mapas.clear();
    votantes.clear();
}

void MapaMgr::VotarPorMapa(uint32 guid, uint32 id)
{
    RemoverVoto(guid);
    BRListaMapas::iterator tmp = mapas.find(id);
    if (tmp != mapas.end())
    {
        tmp->second->votos++;
        votantes[guid] = id;
        if (!usarVotos)
            usarVotos = true;
    }
}

void MapaMgr::RemoverVoto(uint32 guid)
{
    if (usarVotos && votantes.find(guid) != votantes.end())
    {
        uint32 id = votantes[id];
        BRListaMapas::iterator tmp = mapas.find(id);
        if (tmp != mapas.end())
        {
            if (tmp->second->votos > 0)
                tmp->second->votos--;
        }
        if (votantes.find(guid) != votantes.end())
            votantes.erase(guid);
    }
}

void MapaMgr::EstablecerMasVotado()
{
    if (usarVotos && mapas.size())
    {
        BRListaMapas::iterator tmpMap = mapas.begin();
        int tmpVote = -1;
        for (BRListaMapas::iterator it = mapas.begin(); it != mapas.end(); ++it)
        {
            if (it->second && it->second->votos >= tmpVote)
            {
                tmpVote = it->second->votos;
                tmpMap = it;
            }
        }
        if (tmpMap != mapas.end() && tmpVote > 0)
        {
            mapaActual = tmpMap;
        }
        RestablecerVotos();
    }
}

void MapaMgr::SiguienteMapa()
{
    int num = mapas.size();
    mapaActual = mapas.begin();
    if (num > 1)
    {
        int rnd = rand() % num;
        int temp = 0;
        while (++temp <= rnd)
            mapaActual++;
    }
}

void MapaMgr::CargarMapasDesdeBD()
{
    votantes.clear();
    mapas.clear();
    QueryResult result = WorldDatabase.Query("SELECT `id`, `map_id`, `zone_id`, `map_name`, `center_x`, `center_y`, `center_z`, `center_o`, `ship_x`, `ship_y`, `ship_z`, `ship_o` FROM `battleroyale_maps` ORDER BY `id` ASC;");
    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            BRMapa *mapa = new BRMapa();
            uint32 id = fields[0].Get<uint32>();
            mapa->idMapa = fields[1].Get<uint32>();
            mapa->idZona = fields[2].Get<uint32>();
            mapa->nombreMapa = fields[3].Get<std::string>();
            mapa->centroMapa = {
                fields[4].Get<float>(),
                fields[5].Get<float>(),
                fields[6].Get<float>(),
                fields[7].Get<float>()};
            mapa->inicioNave = {
                fields[8].Get<float>(),
                fields[9].Get<float>(),
                fields[10].Get<float>(),
                fields[11].Get<float>()};
            // for (uint32 i = 0; i < CANTIDAD_DE_ZONAS; ++i)
            // {
            //     QueryResult result_spawn = WorldDatabase.Query("SELECT `id`, `pos_x`, `pos_y`, `pos_z`, `pos_o` FROM `battleroyale_maps_spawns` WHERE `zone` = {} AND `map` = {} ORDER BY `zone` ASC;", i, id);
            //     if (result_spawn)
            //     {
            //         do
            //         {
            //             Field* fields_spawn    = result_spawn->Fetch();
            //             uint32 id_spawn    = fields_spawn[0].Get<uint32>();
            //             mapa->ubicacionesMapa[i][id_spawn] = {
            //                 fields_spawn[1].Get<float>(),
            //                 fields_spawn[2].Get<float>(),
            //                 fields_spawn[3].Get<float>(),
            //                 fields_spawn[4].Get<float>()
            //             };
            //         } while (result_spawn->NextRow());
            //     }
            // }
            mapa->votos = 0;
            mapas[id] = mapa;
        } while (result->NextRow());
    }
}

void MapaMgr::RestablecerVotos()
{
    votantes.clear();
    if (mapas.size())
    {
        for (BRListaMapas::iterator it = mapas.begin(); it != mapas.end(); ++it)
        {
            if (it->second)
                it->second->votos = 0;
        }
    }
    if (usarVotos)
        usarVotos = false;
}
