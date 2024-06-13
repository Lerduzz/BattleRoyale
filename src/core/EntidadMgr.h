#ifndef SC_BR_OBJ_MGR_H
#define SC_BR_OBJ_MGR_H

#include "Constantes.h"
#include "GameObject.h"
#include "MapMgr.h"
#include "SpellAuras.h"
#include "Transport.h"

class EntidadMgr
{
    EntidadMgr();
    ~EntidadMgr(){};

public:
    static EntidadMgr *instance()
    {
        static EntidadMgr *instance = new EntidadMgr();
        return instance;
    }

    void DesaparecerTodosLosObjetos();
    bool DesaparecerZona();
    bool DesaparecerNave();
    bool InvocarNave(uint32 mapID, Position pos);
    bool EncenderNave();
    bool EstaEnLaNave(Player *player);
    bool InvocarZona(uint32 mapID, Position pos)
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

    bool InvocarCofre(Position pos)
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

    bool ActualizarZona(float &scale)
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

    float DistanciaDelCentro(Player *player) { return EstaLaZonaActiva() ? player->GetExactDist(npc_Centro) : 0.0f; }; // TODO: GetDist
    bool EstaLaZonaActiva() { return npc_Centro ? true : false; };

    bool HayNave() { return obj_Nave ? true : false; };
    bool HayGuardian() { return npc_Guardian ? true : false; };

    bool HechizoGuardian(uint32 spell, Player *player)
    {
        if (HayGuardian() && player && spell > 0)
        {
            return npc_Guardian->CastSpell(player, spell, true) == SPELL_CAST_OK;
        }
        return false;
    }

private:
    GameObject *obj_Nave;

    Creature *npc_Vendedor;
    Creature *npc_Centro;
    Creature *npc_Guardian;

    bool zonaActiva;
};

#define sEntidadMgr EntidadMgr::instance()

#endif
