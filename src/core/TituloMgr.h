#ifndef SC_BR_TITLES_MGR_H
#define SC_BR_TITLES_MGR_H

#include "DBCStores.h"
#include "Player.h"
#include "SharedDefines.h"

class TituloMgr
{
    TituloMgr(){};
    ~TituloMgr(){};

public:
    static TituloMgr *instance()
    {
        static TituloMgr *instance = new TituloMgr();
        return instance;
    }

    void Ascender(Player* player);
};

#define sTituloMgr TituloMgr::instance()

#endif
