#ifndef SC_BR_SOUNDS_MGR_H
#define SC_BR_SOUNDS_MGR_H

#include "Constantes.h"
#include "MiscPackets.h"

class SonidoMgr
{
    SonidoMgr(){};
    ~SonidoMgr(){};

public:
    static SonidoMgr *instance()
    {
        static SonidoMgr *instance = new SonidoMgr();
        return instance;
    }

    void ReproducirSonidoParaTodos(uint32 soundID, BRListaPersonajes playerList);
};

#define sSonidoMgr SonidoMgr::instance()

#endif
