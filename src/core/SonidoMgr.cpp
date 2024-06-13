#include "SonidoMgr.h"

void SonidoMgr::ReproducirSonidoParaTodos(uint32 soundID, BRListaPersonajes playerList)
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
}
