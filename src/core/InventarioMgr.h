#ifndef SC_BR_ITEM_MGR_H
#define SC_BR_ITEM_MGR_H

#include "Player.h"

class InventarioMgr
{
    InventarioMgr(){};
    ~InventarioMgr(){};

public:
    static InventarioMgr *instance()
    {
        static InventarioMgr *instance = new InventarioMgr();
        return instance;
    }

    bool DarObjetosIniciales(Player *player);
    void QuitarTodosLosObjetos(Player *player);
    bool EsEquipamientoDeBR(uint32 item);
    void Desnudar(Player *player);

private:
    bool QuitarEquipamiento(Player *player, uint32 item);
    bool DarEquipamiento(Player *player, uint32 item);
};

#define sInventarioMgr InventarioMgr::instance()

#endif
