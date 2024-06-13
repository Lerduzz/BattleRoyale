#include "MapaMgr.h"

MapaMgr::MapaMgr()
{
    CargarMapasDesdeBD();
    usarVotos = false;
}

MapaMgr::~MapaMgr()
{
    list_Votos.clear();
    list_Mapas.clear();
}

void MapaMgr::VotarPorMapa(uint32 guid, uint32 id)
{
    RemoverVoto(guid);
    BRListaMapas::iterator tmp = list_Mapas.find(id);
    if (tmp != list_Mapas.end())
    {
        tmp->second->votos++;
        list_Votos[guid] = id;
        if (!usarVotos)
            usarVotos = true;
    }
}

void MapaMgr::RemoverVoto(uint32 guid)
{
    if (usarVotos && list_Votos.find(guid) != list_Votos.end())
    {
        uint32 id = list_Votos[id];
        BRListaMapas::iterator tmp = list_Mapas.find(id);
        if (tmp != list_Mapas.end())
        {
            if (tmp->second->votos > 0)
                tmp->second->votos--;
        }
        list_Votos.erase(guid);
    }
}

void MapaMgr::EstablecerMasVotado()
{
    if (usarVotos && list_Mapas.size())
    {
        BRListaMapas::iterator tmpMap = list_Mapas.begin();
        int tmpVote = -1;
        for (BRListaMapas::iterator it = list_Mapas.begin(); it != list_Mapas.end(); ++it)
        {
            if (it->second && it->second->votos >= tmpVote)
            {
                tmpVote = it->second->votos;
                tmpMap = it;
            }
        }
        if (tmpMap != list_Mapas.end() && tmpVote > 0)
        {
            mapaActual = tmpMap;
        }
        RestablecerVotos();
    }
}

void MapaMgr::SiguienteMapa()
{
    int num = list_Mapas.size();
    mapaActual = list_Mapas.begin();
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
    list_Votos.clear();
    list_Mapas.clear();
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
            list_Mapas[id] = mapa;
        } while (result->NextRow());
    }
}

void MapaMgr::RestablecerVotos()
{
    list_Votos.clear();
    if (list_Mapas.size())
    {
        for (BRListaMapas::iterator it = list_Mapas.begin(); it != list_Mapas.end(); ++it)
        {
            if (it->second)
                it->second->votos = 0;
        }
    }
    if (usarVotos)
        usarVotos = false;
}
