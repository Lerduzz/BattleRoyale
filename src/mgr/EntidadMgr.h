#ifndef SC_BR_ENTIDAD_MGR_H
#define SC_BR_ENTIDAD_MGR_H

#include "Constantes.h"
#include "GameObject.h"

class EntidadMgr
{
    EntidadMgr();
    ~EntidadMgr(){};

public:
    static EntidadMgr *instance()
    {
        static EntidadMgr *instance = new EntidadMgr();
        return instance;
    }

    bool NaveCrear();
    bool NaveEncender();
    bool NaveEliminar();
    bool HayNave();
    bool EstaEnNave(Player *player);

    bool VendedorCrear();
    bool VendedorEliminar();

    bool ZonaCrear();
    bool ZonaActualizar();
    bool ZonaEliminar();
    bool HayZona();
    bool EstaEnZona(Player *player);

private:
    GameObject *obj_Nave;
    Creature *npc_Vendedor;
    Creature *npc_Zona;
};

#define sEntidadMgr EntidadMgr::instance()

#endif
