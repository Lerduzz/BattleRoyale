#ifndef SC_BR_MAPA_MGR_H
#define SC_BR_MAPA_MGR_H

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
    inline BRListaMapas ObtenerMapas() { return mapas; };
    void VotarPorMapa(uint32 guid, uint32 id);
    void RemoverVoto(uint32 guid);
    void EstablecerMasVotado();
    void SiguienteMapa();
    void CargarMapasDesdeBD();
    // bool TieneZonasParaCofres(int zona) { return MapaActual()->ubicacionesMapa.find(zona) != MapaActual()->ubicacionesMapa.end(); };
    // BR_UbicacionZona ObtenerZonasParaCofres(int zona) { return TieneZonasParaCofres(zona) ? MapaActual()->ubicacionesMapa[zona] : BR_UbicacionZona(); };

private:
    void RestablecerVotos();

    BRListaVotos votantes;
    BRListaMapas mapas;
    BRListaMapas::iterator mapaActual;

    bool usarVotos;
};

#define sMapaMgr MapaMgr::instance()

#endif
