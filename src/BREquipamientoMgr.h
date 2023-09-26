#ifndef SC_BR_ITEM_MGR_H
#define SC_BR_ITEM_MGR_H

#include "Player.h"

enum BR_Equipamiento
{
    EQUIPAMIENTO_CAMISA_ALAS                  = 17,
};

class BREquipamientoMgr
{
    BREquipamientoMgr(){};
    ~BREquipamientoMgr(){};

public:
    static BREquipamientoMgr *instance()
    {
        static BREquipamientoMgr *instance = new BREquipamientoMgr();
        return instance;
    }

    bool EntregarAlas(Player* player)
    {
        QuitarAlas(player);
        return player->StoreNewItemInBestSlots(EQUIPAMIENTO_CAMISA_ALAS, 1);
    };

    bool QuitarAlas(Player* player)
    {
        uint32 count = player->GetItemCount(EQUIPAMIENTO_CAMISA_ALAS, true);
        if (count > 0)
        {
            player->DestroyItemCount(EQUIPAMIENTO_CAMISA_ALAS, count, true);
            return true;
        }
        return false;
    };

};

#define sBREquipamientoMgr BREquipamientoMgr::instance()

#endif
