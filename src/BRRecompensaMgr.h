#ifndef SC_BR_REWARD_MGR_H
#define SC_BR_REWARD_MGR_H

#include "PlayerData.h"
#include "Player.h"
#include "Mail.h"

typedef std::map<uint32, Player*> BR_ListaRecompensa;
typedef std::map<uint32, PlayerData> BR_ListaDatos;

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

    void AcumularRecompensa(uint32 rew, PlayerData* data)
    {
        if (data)
        {
            data->reward += rew;
        }
    }

    void DarRecompensas(Player* player, uint32 honor)
    {
        if (!player || honor <= 0)
        {
            return;
        }
        // TODO: Variable de configuracion para definir el % de las diferentes recompensas.
        player->RewardHonor(nullptr, 1, honor);
        uint32 money = honor * 100;
        uint32 countTriunph = uint32(honor / 500);
        if (countTriunph <= 0)
        {
            countTriunph = 1;
        }
        uint32 countFrost = uint32(honor / 2500);
        if (countFrost < 0)
        {
            countFrost = 0;
        }
        std::string subject = "Recompensa de Battle Royale";
        std::ostringstream body;
        body << "Tu recompensa de honor acumulada en la ronda ha sido de " << honor << ", el mismo se te ha entregado directamente a tus monedas. En este correo te enviamos la recompensa adicional acumulada en esa partida de Battle Royale. ¡Sigue así!";
        MailDraft draft(subject, body.str().c_str());
        MailSender sender(MAIL_NORMAL, player->GetGUID().GetCounter(), MAIL_STATIONERY_GM);
        draft.AddMoney(money);
        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
        if (Item* triunph = Item::CreateItem(47241, countTriunph, player))
        {
            triunph->SaveToDB(trans);
            draft.AddItem(triunph);
        }
        if (countFrost > 0)
        {
            if (Item* frost = Item::CreateItem(49426, countFrost, player))
            {
                frost->SaveToDB(trans);
                draft.AddItem(frost);
            }
        }
        draft.SendMailTo(trans, MailReceiver(player, player->GetGUID().GetCounter()), sender);
        CharacterDatabase.CommitTransaction(trans);
    }
};

#define sBRRecompensaMgr BRRecompensaMgr::instance()

#endif
