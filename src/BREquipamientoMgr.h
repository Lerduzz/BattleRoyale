#ifndef SC_BR_ITEM_MGR_H
#define SC_BR_ITEM_MGR_H

#include "Player.h"

enum BR_Equipamiento
{
    EQUIPAMIENTO_CAMISA_ALAS                  = 17,
    EQUIPAMIENTO_PIERNAS_BASE                 = 20902,
};

const uint32 CANTIDAD_ARMAS = 30;
const uint32 BR_Armas[CANTIDAD_ARMAS] = { 34985, 34987, 34988, 34989, 34995, 34996, 34997, 35014, 35015, 35017, 35018, 35037, 35038, 35047, 35058, 35064, 35065, 35071, 35072, 35075, 35076, 35082, 35093, 35095, 35101, 35102, 35103, 35107, 35108, 35109 };

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

    bool DarObjetosIniciales(Player* player)
    {
        DarEquipamiento(player, EQUIPAMIENTO_PIERNAS_BASE);
        return DarEquipamiento(player, EQUIPAMIENTO_CAMISA_ALAS);
    };

    void QuitarTodosLosObjetos(Player* player)
    {
        QuitarEquipamiento(player, EQUIPAMIENTO_PIERNAS_BASE);
        QuitarEquipamiento(player, EQUIPAMIENTO_CAMISA_ALAS);
        player->UpdateTitansGrip();
    };

    bool EsEquipamientoDeBR(uint32 item)
    {
        if (item == EQUIPAMIENTO_CAMISA_ALAS || item == EQUIPAMIENTO_PIERNAS_BASE) return true;
        for (uint32 i = 0; i < CANTIDAD_ARMAS; ++i)
        {
            if (item == BR_Armas[i]) return true;
        }
        return false;
    };

    void Desnudar(Player* player)
    {
        for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
        {
            if (Item* pItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
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
    bool QuitarEquipamiento(Player* player, BR_Equipamiento item)
    {
        uint32 count = player->GetItemCount(item, true);
        if (count > 0)
        {
            player->DestroyItemCount(item, count, true);
            return true;
        }
        return false;
    };

    bool DarEquipamiento(Player* player, BR_Equipamiento item)
    {
        QuitarEquipamiento(player, item);
        return player->StoreNewItemInBestSlots(item, 1);
    };

};

#define sBREquipamientoMgr BREquipamientoMgr::instance()

#endif
