#include "PremioMgr.h"

void PremioMgr::AcumularRecompensaVivos(uint32 rew, BRListaPersonajes list, BRListaDatos *data)
{
    if (list.size())
    {
        for (BRListaPersonajes::iterator it = list.begin(); it != list.end(); ++it)
        {
            BRListaDatos::iterator dt = data->find(it->first);
            if (it->second && it->second->IsAlive() && dt != data->end())
            {
                dt->second.reward += rew;
            }
        }
    }
}

void PremioMgr::AcumularRecompensa(uint32 rew, PlayerData *data)
{
    if (data)
        data->reward += rew;
}

void PremioMgr::DarRecompensas(Player *player, uint32 honor)
{
    if (!player || honor <= 0)
        return;
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
    if (Item *triunph = Item::CreateItem(47241, countTriunph, player))
    {
        triunph->SaveToDB(trans);
        draft.AddItem(triunph);
    }
    if (countFrost > 0)
    {
        if (Item *frost = Item::CreateItem(49426, countFrost, player))
        {
            frost->SaveToDB(trans);
            draft.AddItem(frost);
        }
    }
    draft.SendMailTo(trans, MailReceiver(player, player->GetGUID().GetCounter()), sender);
    CharacterDatabase.CommitTransaction(trans);
}
