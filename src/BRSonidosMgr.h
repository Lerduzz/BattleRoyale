#ifndef SC_BR_SOUNDS_MGR_H
#define SC_BR_SOUNDS_MGR_H

#include "Common.h"
#include "MiscPackets.h"
#include "Player.h"

enum BR_Sonidos
{
    SONIDO_NAVE_EN_MOVIMIENTO               = 8232,
    SONIDO_NAVE_RETIRADA                    = 8213,
    SONIDO_RONDA_INICIADA                   = 8173,
    SONIDO_ZONA_TIEMPO                      = 8192,
    SONIDO_ZONA_REDUCIDA                    = 8212,
    SONIDO_ALGUIEN_MUERE                    = 8174,
    SONIDO_GANADOR_HORDA                    = 8454,
    SONIDO_GANADOR_ALIANZA                  = 8455,
};

typedef std::map<uint32, Player*> BR_ListaSonido;

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
        for (BR_ListaSonido::iterator it = playerList.begin(); it != playerList.end(); ++it)
        {
            if (it->second) it->second->GetSession()->SendPacket(WorldPackets::Misc::Playsound(soundID).Write());
        }
    };

};

#define sBRSonidosMgr BRSonidosMgr::instance()

#endif