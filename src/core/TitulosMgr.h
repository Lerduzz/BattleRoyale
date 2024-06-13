#ifndef SC_BR_TITLES_MGR_H
#define SC_BR_TITLES_MGR_H

#include "DBCStores.h"
#include "Player.h"
#include "SharedDefines.h"

class TitulosMgr
{
    TitulosMgr(){};
    ~TitulosMgr(){};

public:
    static TitulosMgr *instance()
    {
        static TitulosMgr *instance = new TitulosMgr();
        return instance;
    }

    void Ascender(Player* player);
};

#define sTitulosMgr TitulosMgr::instance()

#endif