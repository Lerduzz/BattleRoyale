#ifndef SC_BR_BL_MGR_H
#define SC_BR_BL_MGR_H

#include "Player.h"

class BRListaNegraMgr
{
    BRListaNegraMgr(){};
    ~BRListaNegraMgr(){};

public:
    static BRListaNegraMgr *instance()
    {
        static BRListaNegraMgr *instance = new BRListaNegraMgr();
        return instance;
    }

    bool EstaBloqueado(Player* player);
    void BloquearPersonaje(Player* player);
    void CargarBloqueo(uint32 guid, std::string reason);


};

#define sBRListaNegraMgr BRListaNegraMgr::instance()

#endif