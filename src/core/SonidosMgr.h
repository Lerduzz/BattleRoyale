#ifndef SC_BR_SOUNDS_MGR_H
#define SC_BR_SOUNDS_MGR_H

#include "Common.h"
#include "MiscPackets.h"
#include "Player.h"

class SonidosMgr
{
    SonidosMgr(){};
    ~SonidosMgr(){};

public:
    static SonidosMgr *instance()
    {
        static SonidosMgr *instance = new SonidosMgr();
        return instance;
    }

    void ReproducirSonidoParaTodos(uint32 soundID, BRListaPersonajes playerList)
    {
        if (playerList.size())
        {
            for (BRListaPersonajes::iterator it = playerList.begin(); it != playerList.end(); ++it)
            {
                if (it->second)
                {
                    it->second->GetSession()->SendPacket(WorldPackets::Misc::Playsound(soundID).Write());
                }
            }
        }
    };

};

#define sSonidosMgr SonidosMgr::instance()

#endif