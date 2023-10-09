#ifndef SC_BR_REWARD_MGR_H
#define SC_BR_REWARD_MGR_H

#include "BattleRoyaleData.h"
#include "Player.h"

typedef std::map<uint32, Player*> BR_ListaRecompensa;
typedef std::map<uint32, BattleRoyaleData> BR_ListaDatos;

class BRRecompensaMgr
{
    BRRecompensaMgr(){};
    ~BRRecompensaMgr(){};

public:
    static BRRecompensaMgr *instance()
    {
        static BRRecompensaMgr *instance = new BRRecompensaMgr();
        return instance;
    }

    void AcumularRecompensaVivos(uint32 rew, BR_ListaRecompensa list, BR_ListaDatos* data)
    {
        if (list.size())
        {
            for (BR_ListaRecompensa::iterator it = list.begin(); it != list.end(); ++it)
            {
                BR_ListaDatos::iterator dt = data->find(it->first);
                if (it->second && it->second->IsAlive() && dt != data->end())
                {
                    dt->second.reward += rew;
                }
            }
        }
    }

    void AcumularRecompensa(uint32 rew, BattleRoyaleData* data)
    {
        if (data)
        {
            data->reward += rew;
        }
    }

};

#define sBRRecompensaMgr BRRecompensaMgr::instance()

#endif
