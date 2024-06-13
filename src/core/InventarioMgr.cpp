#include "InventarioMgr.h"

bool InventarioMgr::DarObjetosIniciales(Player *player)
{
    QuitarTodosLosObjetos(player);
    DarEquipamiento(player, BR_EQUIPO_INICIAL_PANTALONES);
    return DarEquipamiento(player, BR_EQUIPO_INICIAL_CAMISA);
}

void InventarioMgr::QuitarTodosLosObjetos(Player *player)
{
    QuitarEquipamiento(player, BR_EQUIPO_INICIAL_CAMISA);
    QuitarEquipamiento(player, BR_EQUIPO_INICIAL_PANTALONES);
    for (uint32 i = 0; i < BR_EQUIPO_MAXIMO; ++i)
    {
        QuitarEquipamiento(player, BR_EQUIPO_LISTA[i]);
    }
    player->UpdateTitansGrip();
}

bool InventarioMgr::EsEquipamientoDeBR(uint32 item)
{
    if (item == BR_EQUIPO_INICIAL_CAMISA || item == BR_EQUIPO_INICIAL_PANTALONES)
        return true;
    for (uint32 i = 0; i < BR_EQUIPO_MAXIMO; ++i)
    {
        if (item == BR_EQUIPO_LISTA[i])
            return true;
    }
    return false;
}

void InventarioMgr::Desnudar(Player *player)
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
}

bool InventarioMgr::QuitarEquipamiento(Player *player, uint32 item)
{
    uint32 count = player->GetItemCount(item, true);
    if (count > 0)
    {
        player->DestroyItemCount(item, count, true);
        return true;
    }
    return false;
}

bool InventarioMgr::DarEquipamiento(Player *player, uint32 item)
{
    QuitarEquipamiento(player, item);
    return player->StoreNewItemInBestSlots(item, 1);
}
