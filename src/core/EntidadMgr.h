#ifndef SC_BR_OBJ_MGR_H
#define SC_BR_OBJ_MGR_H

#include "Constantes.h"
#include "GameObject.h"
#include "MapMgr.h"
#include "SpellAuras.h"
#include "Transport.h"

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

    void DesaparecerTodosLosObjetos();
    bool DesaparecerZona();
    bool DesaparecerNave();
    bool InvocarNave(uint32 mapID, Position pos);
    bool EncenderNave();
    bool EstaEnLaNave(Player *player);
    bool InvocarZona(uint32 mapID, Position pos);
    bool InvocarCofre(Position pos);
    bool ActualizarZona(float &scale);
    bool HechizoGuardian(uint32 spell, Player *player);
    inline float DistanciaDelCentro(Player *player) { return EstaLaZonaActiva() ? player->GetExactDist(npc_Centro) : 0.0f; };
    inline bool EstaLaZonaActiva() { return npc_Centro ? true : false; };
    inline bool HayNave() { return obj_Nave ? true : false; };
    inline bool HayGuardian() { return npc_Guardian ? true : false; };

private:
    GameObject *obj_Nave;

    Creature *npc_Vendedor;
    Creature *npc_Centro;
    Creature *npc_Guardian;

    bool zonaActiva;
};

#define sEntidadMgr EntidadMgr::instance()

#endif
