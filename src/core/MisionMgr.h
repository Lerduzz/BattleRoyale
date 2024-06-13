#ifndef SC_BR_QUEST_MGR_H
#define SC_BR_QUEST_MGR_H

#include "Constantes.h"

class MisionMgr
{
    MisionMgr(){};
    ~MisionMgr(){};

public:
    static MisionMgr *instance()
    {
        static MisionMgr *instance = new MisionMgr();
        return instance;
    }

    void CompletarRequerimiento(BRMision mision, BRMisionReq requiere, BRListaPersonajes playerList);
};

#define sMisionMgr MisionMgr::instance()

#endif
