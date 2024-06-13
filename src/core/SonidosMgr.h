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

    void ReproducirSonidoParaTodos(uint32 soundID, BRListaPersonajes playerList);
};

#define sSonidosMgr SonidosMgr::instance()

#endif