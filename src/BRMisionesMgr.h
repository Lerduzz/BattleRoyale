#ifndef SC_BR_QUEST_MGR_H
#define SC_BR_QUEST_MGR_H

#include "Constantes.h"

class BRMisionesMgr
{
    BRMisionesMgr(){};
    ~BRMisionesMgr(){};

public:
    static BRMisionesMgr *instance()
    {
        static BRMisionesMgr *instance = new BRMisionesMgr();
        return instance;
    }

    void CompletarRequerimiento(BRMision mision, BRMisionReq requiere, BRListaPersonajes playerList)
    {
        if (playerList.size())
        {
            for (BR_ListaMision::iterator it = playerList.begin(); it != playerList.end(); ++it)
            {
                if (it->second && it->second->HasQuest(mision) && it->second->GetQuestStatus(mision) == QUEST_STATUS_INCOMPLETE)
                {
                    it->second->KilledMonsterCredit(requiere);
                }
            }
        }
    };
};

#define sBRMisionesMgr BRMisionesMgr::instance()

#endif
