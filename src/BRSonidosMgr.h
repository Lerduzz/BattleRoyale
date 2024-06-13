#ifndef SC_BR_SOUNDS_MGR_H
#define SC_BR_SOUNDS_MGR_H

#include "Common.h"
#include "MiscPackets.h"
#include "Player.h"

class BRSonidosMgr
{
    BRSonidosMgr(){};
    ~BRSonidosMgr(){};

public:
    static BRSonidosMgr *instance()
    {
        static BRSonidosMgr *instance = new BRSonidosMgr();
        return instance;
    }

    void ReproducirSonidoParaTodos(uint32 soundID, BR_ListaSonido playerList)
    {
        if (playerList.size())
        {
            for (BR_ListaSonido::iterator it = playerList.begin(); it != playerList.end(); ++it)
            {
                if (it->second)
                {
                    it->second->GetSession()->SendPacket(WorldPackets::Misc::Playsound(soundID).Write());
                }
            }
        }
    };

};

#define sBRSonidosMgr BRSonidosMgr::instance()

#endif