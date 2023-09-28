#ifndef SC_BR_ITEM_MGR_H
#define SC_BR_ITEM_MGR_H

#include "Player.h"

enum BR_Equipamiento
{
    EQUIPAMIENTO_CAMISA_ALAS                  = 17,
    EQUIPAMIENTO_PIERNAS_BASE                 = 20902,
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
        return DarEquipamiento(player, EQUIPAMIENTO_CAMISA_ALAS);
    };

    bool QuitarAlas(Player* player)
    {
        return QuitarEquipamiento(player, EQUIPAMIENTO_CAMISA_ALAS);
    };

private:
    bool QuitarEquipamiento(Player* player, BR_Equipamiento item)
    {
        uint32 count = player->GetItemCount(item, true);
        if (count > 0)
        {
            player->DestroyItemCount(item, count, true);
            return true;
        }
        return false;
    };

    bool DarEquipamiento(Player* player, BR_Equipamiento item)
    {
        QuitarEquipamiento(player, item);
        return player->StoreNewItemInBestSlots(item, 1);
    };

};

#define sBREquipamientoMgr BREquipamientoMgr::instance()

#endif
