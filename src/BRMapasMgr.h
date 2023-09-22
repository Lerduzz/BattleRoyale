#ifndef SC_BR_MAPS_MGR_H
#define SC_BR_MAPS_MGR_H

#include "BRConstantes.h"

typedef std::map<uint32, uint32> BR_ListVotos;

typedef std::map<uint32, Position> BR_UbicacionZona;
typedef std::map<uint32, BR_UbicacionZona> BR_Ubicacion;
struct BR_Mapa
{
    uint32 idMapa;
    std::string nombreMapa;
    Position centroMapa;
    Position inicioNave;
    BR_Ubicacion ubicacionesMapa;
    int votos;
};
typedef std::map<uint32, BR_Mapa*> BR_ContenedorMapas;

class BRMapasMgr
{
    BRMapasMgr()
    {
        CargarMapasDesdeBD();
        usarVotos = false;
    };
    ~BRMapasMgr()
    {
        list_Votos.clear();
        list_Mapas.clear();
    };

public:
    static BRMapasMgr *instance()
    {
        static BRMapasMgr *instance = new BRMapasMgr();
        return instance;
    }

    BR_Mapa* MapaActual() { return mapaActual->second; };
    BR_ContenedorMapas ObtenerMapas() { return list_Mapas; };
    void VotarPorMapa(uint32 guid, uint32 id)
    {
        RemoverVoto(guid);
        BR_ContenedorMapas::iterator tmp = list_Mapas.find(id);
        if (tmp != list_Mapas.end())
        {
            tmp->second->votos++;
            list_Votos[guid] = id;
            if (!usarVotos) usarVotos = true;
        }
    };
    void RemoverVoto(uint32 guid)
    {
        if (usarVotos && list_Votos.find(guid) != list_Votos.end())
        {
            uint32 id = list_Votos[id];
            BR_ContenedorMapas::iterator tmp = list_Mapas.find(id);
            if (tmp != list_Mapas.end())
            {
                if (tmp->second->votos > 0) tmp->second->votos--;
            }
            LimpiarVoto(guid);
        }
    };
    void LimpiarVoto(uint32 guid) { if (list_Votos.find(guid) != list_Votos.end()) list_Votos.erase(guid); };
    void EstablecerMasVotado()
    {
        if (usarVotos && list_Mapas.size())
        {
            BR_ContenedorMapas::iterator tmpMap = list_Mapas.begin();
            int tmpVote = -1;
            for (BR_ContenedorMapas::iterator it = list_Mapas.begin(); it != list_Mapas.end(); ++it)
            {
                if (it->second && it->second->votos >= tmpVote) // >= (Cuando hay empate usar el ultimo mapa del empate), > (Cuando hay empate usar el primer mapa del empate).
                {
                    tmpVote = it->second->votos;
                    tmpMap = it;
                }
            }
            if (tmpMap != list_Mapas.end())
            {
                mapaActual = tmpMap;
                RestablecerVotos();
            }
        }
    };
    void SiguienteMapa()
    {
        int num = list_Mapas.size();
        mapaActual = list_Mapas.begin();
        if (num > 1)
        {
            int rnd = rand() % num;
            mapaActual = list_Mapas.begin();
            int temp = 0;
            while (++temp <= rnd) mapaActual++;
        }
    };
    void CargarMapasDesdeBD()
    {
        list_Votos.clear();
        list_Mapas.clear();
        QueryResult result = WorldDatabase.Query("SELECT `id`, `map_id`, `map_name`, `center_x`, `center_y`, `center_z`, `center_o`, `ship_x`, `ship_y`, `ship_z`, `ship_o` FROM `battleroyale_maps` ORDER BY `id` ASC;");
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
                    QueryResult result_spawn = WorldDatabase.Query("SELECT `id`, `pos_x`, `pos_y`, `pos_z`, `pos_o` FROM `battleroyale_maps_spawns` WHERE `zone` = {} AND `map` = {} ORDER BY `zone` ASC;", i, id);
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
                mapa->votos = 0;
                list_Mapas[id] = mapa;
            } while (result->NextRow());
        }
    };
    bool TieneZonasParaCofres(int zona) { return MapaActual()->ubicacionesMapa.find(zona) != MapaActual()->ubicacionesMapa.end(); };
    BR_UbicacionZona ObtenerZonasParaCofres(int zona) { return TieneZonasParaCofres(zona) ? MapaActual()->ubicacionesMapa[zona] : BR_UbicacionZona(); };

private:
    void RestablecerVotos()
    {
        list_Votos.clear();
        if (list_Mapas.size())
        {
            for (BR_ContenedorMapas::iterator it = list_Mapas.begin(); it != list_Mapas.end(); ++it)
            {
                if (it->second) it->second->votos = 0;
            }
        }
        if (usarVotos) usarVotos = false;
    };

    BR_ListVotos list_Votos;
    BR_ContenedorMapas list_Mapas;
    BR_ContenedorMapas::iterator mapaActual;

    bool usarVotos;

};

#define sBRMapasMgr BRMapasMgr::instance()

#endif
