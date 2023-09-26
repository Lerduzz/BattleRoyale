#ifndef SC_BR_ITEM_MGR_H
#define SC_BR_ITEM_MGR_H

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

};

#define sBREquipamientoMgr BREquipamientoMgr::instance()

#endif