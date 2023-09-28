#ifndef SC_BR_QUEST_MGR_H
#define SC_BR_QUEST_MGR_H

#include "Player.h"

enum BR_Mision
{
    MISION_DIARIA_1                         = 100000,
};

enum BR_MisionRequiere
{
    MISION_DIARIA_1_REQ_1                   = 200003,
};

typedef std::map<uint32, Player*> BR_ListaMision;

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

    void CompletarRequerimiento(BR_Mision mision, BR_MisionRequiere requiere, BR_ListaMision playerList)
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
