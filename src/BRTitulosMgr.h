#ifndef SC_BR_TITLES_MGR_H
#define SC_BR_TITLES_MGR_H

#include "DBCStores.h"
#include "Player.h"
#include "SharedDefines.h"

class BRTitulosMgr
{
    BRTitulosMgr(){};
    ~BRTitulosMgr(){};

public:
    static BRTitulosMgr *instance()
    {
        static BRTitulosMgr *instance = new BRTitulosMgr();
        return instance;
    }

    void Ascender(Player* player)
    {
        if (!player) return;
        TeamId pteam = TEAM_ALLIANCE;
        if (pteam != TEAM_ALLIANCE && pteam != TEAM_HORDE) return;
        uint32 first = player->GetTeamId() == TEAM_ALLIANCE ? 1 : 15;
        uint32 last = first + 13;
        for (uint32 titleId = last; titleId >= first; --titleId)
        {
            CharTitlesEntry const* titleInfo = sCharTitlesStore.LookupEntry(titleId);
            if (titleInfo)
            {
                if (player->HasTitle(titleInfo))
                {
                    if (titleId == last)
                    {
                        player->SetUInt32Value(PLAYER_CHOSEN_TITLE, titleInfo->bit_index);
                        return;
                    }
                    player->SetTitle(titleInfo, true);
                    CharTitlesEntry const* newTitleInfo = sCharTitlesStore.LookupEntry(titleId + 1);
                    if (newTitleInfo)
                    {
                        player->SetTitle(newTitleInfo);
                        player->SetUInt32Value(PLAYER_CHOSEN_TITLE, newTitleInfo->bit_index);
                    }
                    if (!player->HasTitle(player->GetInt32Value(PLAYER_CHOSEN_TITLE))) player->SetUInt32Value(PLAYER_CHOSEN_TITLE, 0);
                    break;
                }
                else
                {
                    if (titleId == first)
                    {
                        player->SetTitle(titleInfo);
                        player->SetUInt32Value(PLAYER_CHOSEN_TITLE, titleInfo->bit_index);
                    }
                }
            }
        }
    };
};

#define sBRTitulosMgr BRTitulosMgr::instance()

#endif