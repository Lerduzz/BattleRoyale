#ifndef SC_BR_REWARD_MGR_H
#define SC_BR_REWARD_MGR_H

#include "Constantes.h"
#include "Mail.h"

class PremioMgr
{
    PremioMgr(){};
    ~PremioMgr(){};

public:
    static PremioMgr *instance()
    {
        static PremioMgr *instance = new PremioMgr();
        return instance;
    }

    void AcumularRecompensaVivos(uint32 rew, BRListaPersonajes list, BRListaDatos *data);
    void AcumularRecompensa(uint32 rew, PlayerData *data);
    void DarRecompensas(Player *player, uint32 honor);
};

#define sPremioMgr PremioMgr::instance()

#endif
