#include "MisionMgr.h"

void MisionMgr::CompletarRequerimiento(BRMision mision, BRMisionReq requiere, BRListaPersonajes playerList)
{
    if (playerList.size())
    {
        for (BRListaPersonajes::iterator it = playerList.begin(); it != playerList.end(); ++it)
        {
            if (it->second && it->second->HasQuest(mision) && it->second->GetQuestStatus(mision) == QUEST_STATUS_INCOMPLETE)
            {
                it->second->KilledMonsterCredit(requiere);
            }
        }
    }
}