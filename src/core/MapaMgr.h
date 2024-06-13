#ifndef SC_BR_MAPS_MGR_H
#define SC_BR_MAPS_MGR_H

#include "Constantes.h"

class MapaMgr
{
    MapaMgr();
    ~MapaMgr();

public:
    static MapaMgr *instance()
    {
        static MapaMgr *instance = new MapaMgr();
        return instance;
    }

    inline BRMapa *MapaActual() { return mapaActual->second; };
    inline BRListaMapas ObtenerMapas() { return list_Mapas; };
    void VotarPorMapa(uint32 guid, uint32 id);
    void RemoverVoto(uint32 guid);
    void EstablecerMasVotado();
    void SiguienteMapa();
    void CargarMapasDesdeBD();
    inline bool TieneZonasParaCofres(int zona) { return MapaActual()->ubicacionesMapa.find(zona) != MapaActual()->ubicacionesMapa.end(); };
    inline std::map<uint32, Position> ObtenerZonasParaCofres(int zona) { return TieneZonasParaCofres(zona) ? MapaActual()->ubicacionesMapa[zona] : std::map<uint32, Position>(); };

private:
    void RestablecerVotos();

    BRListaVotos list_Votos;
    BRListaMapas list_Mapas;
    BRListaMapas::iterator mapaActual;

    bool usarVotos;
};

#define sMapaMgr MapaMgr::instance()

#endif
