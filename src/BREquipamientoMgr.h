#ifndef SC_BR_ITEM_MGR_H
#define SC_BR_ITEM_MGR_H

#include "Player.h"

enum BR_Equipamiento
{
    EQUIPAMIENTO_CAMISA_ALAS = 500000,
    EQUIPAMIENTO_PIERNAS_BASE = 500001,
    EQUIPAMIENTO_BR_MAXIMO = 500210,
};

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
        DarEquipamiento(player, EQUIPAMIENTO_PIERNAS_BASE);
        return DarEquipamiento(player, EQUIPAMIENTO_CAMISA_ALAS);
    };

    void QuitarTodosLosObjetos(Player *player)
    {
        for (uint32 i = EQUIPAMIENTO_CAMISA_ALAS; i <= EQUIPAMIENTO_BR_MAXIMO; ++i)
        {
            QuitarEquipamiento(player, i);
        }
        player->UpdateTitansGrip();
    };

    bool EsEquipamientoDeBR(uint32 item)
    {
        return item >= EQUIPAMIENTO_CAMISA_ALAS && item <= EQUIPAMIENTO_BR_MAXIMO;
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
