#ifndef SC_BR_ITEM_MGR_H
#define SC_BR_ITEM_MGR_H

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