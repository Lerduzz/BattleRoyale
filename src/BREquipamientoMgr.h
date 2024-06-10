#ifndef SC_BR_ITEM_MGR_H
#define SC_BR_ITEM_MGR_H

#include "Player.h"

enum BREquipo
{
    EQUIPAMIENTO_INICIAL_CAMISA = 17,
    EQUIPAMIENTO_INICIAL_PANTALONES = 20902,
};

const uint32 EQUIPAMIENTO_BR_MAXIMO = 209;
const uint32 EQUIPAMIENTO_BR[EQUIPAMIENTO_BR_MAXIMO] = {41896, 41901, 41907, 42055, 42022, 42056, 42023, 42057, 42024, 42110, 41880, 41884, 41892, 41330, 41331, 41332, 41628, 41633, 41638, 42058, 42026, 42059, 42025, 41233, 41228, 41223, 42027, 42060, 42020, 42061, 42021, 42112, 40966, 40973, 40972, 40877, 40878, 40887, 41877, 41879, 41878, 41827, 41828, 41830, 41050, 41049, 41047, 41068, 41073, 41063, 42316, 28298, 42321, 28299, 42331, 24550, 42489, 34529, 42524, 42345, 28297, 32053, 44417, 24557, 44418, 33716, 42359, 33766, 33763, 28346, 42536, 42530, 42501, 42494, 42446, 40781, 40803, 40820, 40841, 42619, 40860, 41308, 41284, 41319, 41296, 42577, 41273, 41659, 41771, 41676, 41665, 42587, 41713, 42389, 34540, 41314, 41291, 41325, 41302, 42582, 41279, 42226, 28309, 42247, 28310, 42269, 28314, 42279, 28302, 42289, 28307, 42484, 34530, 20335, 42563, 42351, 32450, 42569, 42383, 32055, 41085, 41141, 41155, 41203, 41215, 41950, 41969, 41944, 41957, 41963, 40904, 40925, 40931, 40937, 42613, 40961, 40782, 40802, 40821, 40842, 42851, 40861, 42326, 28300, 41857, 41872, 41852, 41862, 41867, 41919, 41938, 41913, 41925, 41931, 41648, 41765, 41670, 41653, 41681, 40988, 40999, 41011, 41025, 42596, 41036, 41079, 41135, 41149, 41162, 42606, 41209, 40989, 41005, 41017, 41031, 42601, 41042, 42207, 42236, 42241, 28312, 42274, 28305, 42284, 28295, 42259, 28313, 42231, 31985, 42254, 33801, 32003, 42264, 42518, 42512, 42558, 42449, 42001, 42015, 41991, 42003, 42009, 40783, 40801, 40819, 40840, 40859, 18854, 18834};

class BREquipamientoMgr
{
    BREquipamientoMgr(){};
    ~BREquipamientoMgr(){};

public:
    static BREquipamientoMgr *instance()
    {
        static BREquipamientoMgr *instance = new BREquipamientoMgr();
        return instance;
    }

    bool DarObjetosIniciales(Player *player)
    {
        QuitarTodosLosObjetos(player);
        DarEquipamiento(player, EQUIPAMIENTO_INICIAL_PANTALONES);
        return DarEquipamiento(player, EQUIPAMIENTO_INICIAL_CAMISA);
    };

    void QuitarTodosLosObjetos(Player *player)
    {
        QuitarEquipamiento(player, EQUIPAMIENTO_INICIAL_CAMISA);
        QuitarEquipamiento(player, EQUIPAMIENTO_INICIAL_PANTALONES);
        for (uint32 i = 0; i < EQUIPAMIENTO_BR_MAXIMO; ++i)
        {
            QuitarEquipamiento(player, EQUIPAMIENTO_BR[i]);
        }
        player->UpdateTitansGrip();
    };

    bool EsEquipamientoDeBR(uint32 item)
    {
        if (item == EQUIPAMIENTO_INICIAL_CAMISA || item == EQUIPAMIENTO_INICIAL_PANTALONES)
            return true;
        for (uint32 i = 0; i < EQUIPAMIENTO_BR_MAXIMO; ++i)
        {
            if (item == EQUIPAMIENTO_BR[i])
                return true;
        }
        return false;
    };

    void Desnudar(Player *player)
    {
        for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
        {
            if (Item *pItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            {
                ItemPosCountVec dest;
                uint8 msg = player->CanStoreItem(NULL_BAG, NULL_SLOT, dest, pItem, false);
                if (msg == EQUIP_ERR_OK)
                {
                    player->RemoveItem(INVENTORY_SLOT_BAG_0, i, true);
                    player->StoreItem(dest, pItem, true);
                }
                else
                {
                    player->MoveItemFromInventory(INVENTORY_SLOT_BAG_0, i, true);
                    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
                    pItem->DeleteFromInventoryDB(trans);
                    pItem->SaveToDB(trans);
                    std::string subject = "Battle Royale: Guardian de Equipamiento";
                    std::string body = "¡Hubo un problema al desequipar uno de tus objetos! En el Battle Royale solo se permite utilizar el equipamiento preparado para este modo de juego.";
                    MailDraft(subject, body).AddItem(pItem).SendMailTo(trans, player, MailSender(player, MAIL_STATIONERY_GM), MAIL_CHECK_MASK_COPIED);
                    CharacterDatabase.CommitTransaction(trans);
                }
            }
        }
        player->UpdateTitansGrip();
    };

private:
    bool QuitarEquipamiento(Player *player, uint32 item)
    {
        uint32 count = player->GetItemCount(item, true);
        if (count > 0)
        {
            player->DestroyItemCount(item, count, true);
            return true;
        }
        return false;
    };

    bool DarEquipamiento(Player *player, uint32 item)
    {
        QuitarEquipamiento(player, item);
        return player->StoreNewItemInBestSlots(item, 1);
    };
};

#define sBREquipamientoMgr BREquipamientoMgr::instance()

#endif
