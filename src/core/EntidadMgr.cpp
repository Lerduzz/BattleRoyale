#include "EntidadMgr.h"

EntidadMgr::EntidadMgr()
{
    npc_Centro = nullptr;
    obj_Nave = nullptr;
    npc_Vendedor = nullptr;
    npc_Guardian = nullptr;
    zonaActiva = false;
}

void EntidadMgr::DesaparecerTodosLosObjetos()
{
    DesaparecerZona();
    DesaparecerNave();
}

bool EntidadMgr::DesaparecerZona()
{
    if (HayGuardian())
    {
        npc_Guardian->CleanupsBeforeDelete();
        delete npc_Guardian;
        npc_Guardian = nullptr;
    }
    if (EstaLaZonaActiva())
    {
        npc_Centro->CleanupsBeforeDelete();
        delete npc_Centro;
        npc_Centro = nullptr;
        return true;
    }
    return false;
}

bool EntidadMgr::DesaparecerNave()
{
    if (npc_Vendedor)
    {
        npc_Vendedor->CleanupsBeforeDelete();
        delete npc_Vendedor;
        npc_Vendedor = nullptr;
    }
    if (HayNave())
    {
        if (Transport *tp = obj_Nave->ToTransport())
        {
            tp->CleanupsBeforeDelete();
        }
        else
        {
            obj_Nave->CleanupsBeforeDelete();
        }
        delete obj_Nave;
        obj_Nave = nullptr;
        return true;
    }
    return false;
}

bool EntidadMgr::InvocarNave(uint32 mapID, Position pos)
{
    Map *map = sMapMgr->CreateBaseMap(mapID);
    if (map)
    {
        DesaparecerNave();
        float x = pos.GetPositionX();
        float y = pos.GetPositionY();
        float z = pos.GetPositionZ();
        float o = pos.GetOrientation();
        float rot2 = std::sin(o / 2);
        float rot3 = cos(o / 2);
        map->LoadGrid(x, y);
        obj_Nave = new StaticTransport();
        if (obj_Nave->Create(map->GenerateLowGuid<HighGuid::GameObject>(), BR_OBJETO_NAVE, map, BR_VISIBILIDAD_EVENTO, x, y, z, o, G3D::Quat(0, 0, rot2, rot3), 100, GO_STATE_READY))
        {
            obj_Nave->SetVisibilityDistanceOverride(VisibilityDistanceType::Infinite);
            map->AddToMap(obj_Nave);
            if (Transport *transport = obj_Nave->ToTransport())
            {
                float vX = 0.0f;
                float vY = 23.5f;
                float vZ = 0.0f;
                float vO = -M_PI_2;
                transport->CalculatePassengerPosition(*(&vX), *(&vY), *(&vZ), &vO);
                if ((npc_Vendedor = transport->SummonCreature(BR_CRIATURA_VENDEDOR_ARMAS, vX, vY, vZ, vO, TEMPSUMMON_MANUAL_DESPAWN)))
                {
                    transport->AddPassenger(npc_Vendedor, true);
                }
                else
                {
                    delete npc_Vendedor;
                    npc_Vendedor = nullptr;
                }
            }
            return true;
        }
        else
        {
            delete obj_Nave;
            obj_Nave = nullptr;
        }
    }
    return false;
}

bool EntidadMgr::EncenderNave()
{
    if (HayNave())
    {
        uint32_t const autoCloseTime = obj_Nave->GetGOInfo()->GetAutoCloseTime() ? 10000u : 0u;
        obj_Nave->SetLootState(GO_READY);
        obj_Nave->UseDoorOrButton(autoCloseTime, false, nullptr);
        return true;
    }
    return false;
}

bool EntidadMgr::EstaEnLaNave(Player *player)
{
    if (player && HayNave())
    {
        if (Transport *tp = obj_Nave->ToTransport())
        {
            if (Transport *playertp = player->GetTransport())
            {
                if (tp == playertp)
                    return true;
            }
        }
    }
    return false;
}

bool EntidadMgr::InvocarZona(uint32 mapID, Position pos)
{
    Map *map = sMapMgr->CreateBaseMap(mapID);
    if (map)
    {
        DesaparecerZona();
        float x = pos.GetPositionX();
        float y = pos.GetPositionY();
        float z = pos.GetPositionZ();
        float o = pos.GetOrientation();
        CreatureTemplate const *cinfo = sObjectMgr->GetCreatureTemplate(12999);
        if (!cinfo)
            return false;
        npc_Centro = new Creature(true);
        if (!npc_Centro->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, BR_VISIBILIDAD_EVENTO, 12999, 0, x, y, z, o))
        {
            delete npc_Centro;
            npc_Centro = nullptr;
            return false;
        }
        npc_Centro->SetVisibilityDistanceOverride(VisibilityDistanceType::Infinite);
        map->AddToMap(npc_Centro);
        npc_Centro->AddAura(63894, npc_Centro);
        npc_Centro->SetObjectScale(15.2f);
        if (!(npc_Guardian = npc_Centro->SummonCreature(BR_CRIATURA_DRAGON_GUARDIAN, x, y, z + 330.0f, o, TEMPSUMMON_MANUAL_DESPAWN)))
        {
            delete npc_Guardian;
            npc_Guardian = nullptr;
        }
        return true;
    }
    return false;
}

bool EntidadMgr::InvocarCofre(Position pos)
{
    if (EstaLaZonaActiva())
    {
        if (npc_Centro->SummonGameObject(BR_OBJETO_COFRE, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation(), 0, 0, 0, 0, 60))
        {
            return true;
        }
    }
    return false;
}

bool EntidadMgr::ActualizarZona(float &scale)
{
    if (EstaLaZonaActiva())
    {
        if (scale < 0.2f)
        {
            npc_Centro->CleanupsBeforeDelete();
            delete npc_Centro;
            npc_Centro = nullptr;
        }
        else
        {
            npc_Centro->SetObjectScale(scale);
            scale -= 0.01f;
        }
    }
    return true;
}

bool EntidadMgr::HechizoGuardian(uint32 spell, Player *player)
{
    if (HayGuardian() && player && spell > 0)
    {
        return npc_Guardian->CastSpell(player, spell, true) == SPELL_CAST_OK;
    }
    return false;
}
