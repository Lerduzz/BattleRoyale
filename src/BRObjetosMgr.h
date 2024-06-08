#ifndef SC_BR_OBJ_MGR_H
#define SC_BR_OBJ_MGR_H

#include "BRConstantes.h"
#include "GameObject.h"
#include "MapMgr.h"
#include "SpellAuras.h"
#include "Transport.h"

enum BR_Dimensiones
{
    DIMENSION_NORMAL                        = 0x00000001,
    DIMENSION_EVENTO                        = 0x00000002,
};

enum BR_ObjetosMundo
{
    OBJETO_NAVE                             = 194675,
    OBJETO_COFRE                            = 499999,
    OBJETO_CENTRO_DEL_MAPA                  = 500000,
    OBJETO_ZONA_SEGURA_INICIAL              = 500001,
};

enum BR_Criaturas
{
    CRIATURA_VENDEDOR_ARMAS                 = 200001,
    CRIATURA_DRAGON_GUARDIAN                = 199999,
};

class BRObjetosMgr
{
    BRObjetosMgr()
    {
        npc_Centro = nullptr;
        obj_Nave = nullptr;
        npc_Vendedor = nullptr;
        npc_Guardian = nullptr;
        zonaActiva = false;    
    };
    ~BRObjetosMgr(){};

public:
    static BRObjetosMgr *instance()
    {
        static BRObjetosMgr *instance = new BRObjetosMgr();
        return instance;
    }

    void DesaparecerTodosLosObjetos()
    {
        DesaparecerZona();
        DesaparecerCentro();
        DesaparecerNave();
    }

    bool DesaparecerCentro()
    {
        if (HayGuardian())
        {
            npc_Guardian->CleanupsBeforeDelete();
            delete npc_Guardian;
            npc_Guardian = nullptr;
        }
        if (HayCentro()) {
            npc_Centro->CleanupsBeforeDelete();
            delete npc_Centro;
            npc_Centro = nullptr;
            return true;
        }
        return false;
    }

    bool DesaparecerNave()
    {
        if (npc_Vendedor)
        {
            npc_Vendedor->CleanupsBeforeDelete();
            delete npc_Vendedor;
            npc_Vendedor = nullptr;
        }
        if (HayNave()) {
            if(Transport* tp = obj_Nave->ToTransport())
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

    bool InvocarNave(uint32 mapID, Position pos)
    {
        Map* map = sMapMgr->CreateBaseMap(mapID);
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
            if (obj_Nave->Create(map->GenerateLowGuid<HighGuid::GameObject>(), OBJETO_NAVE, map, DIMENSION_EVENTO, x, y, z, o, G3D::Quat(0, 0, rot2, rot3), 100, GO_STATE_READY))
            {
                obj_Nave->SetVisibilityDistanceOverride(VisibilityDistanceType::Infinite);
                map->AddToMap(obj_Nave);
                if (Transport* transport = obj_Nave->ToTransport())
                {
                    float vX = 0.0f;
                    float vY = 23.5f;
                    float vZ = 0.0f;
                    float vO = - M_PI_2;
                    transport->CalculatePassengerPosition(*(&vX), *(&vY), *(&vZ), &vO);
                    if ((npc_Vendedor = transport->SummonCreature(CRIATURA_VENDEDOR_ARMAS, vX, vY, vZ, vO, TEMPSUMMON_MANUAL_DESPAWN)))
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

    bool EncenderNave()
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

    bool EstaEnLaNave(Player* player)
    {
        if (player && HayNave())
        {
            if (Transport* tp = obj_Nave->ToTransport())
            {
                if (Transport* playertp = player->GetTransport())
                {
                    if (tp == playertp) return true;
                }
            }
        }
        return false;
    }

    bool InvocarCentroDelMapa(uint32 mapID, Position pos)
    {
        Map* map = sMapMgr->CreateBaseMap(mapID);
        if (map)
        {
            DesaparecerCentro();
            float x = pos.GetPositionX();
            float y = pos.GetPositionY();
            float z = pos.GetPositionZ();
            float o = pos.GetOrientation();
            CreatureTemplate const* cinfo = sObjectMgr->GetCreatureTemplate(12999);
            if (!cinfo)
                return false;
            npc_Centro = new Creature(true);
            if (!npc_Centro->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, DIMENSION_EVENTO, 12999, 0, x, y, z, o))
            {
                delete npc_Centro;
                npc_Centro = nullptr;
                return false;
            }
            npc_Centro->SetVisibilityDistanceOverride(VisibilityDistanceType::Infinite);
            map->AddToMap(npc_Centro);
            Aura::TryRefreshStackOrCreate(63894, MAX_EFFECT_MASK, npc_Centro, npc_Centro);
            npc_Centro->SetObjectScale(15.0f);
            if (!(npc_Guardian = npc_Centro->SummonCreature(CRIATURA_DRAGON_GUARDIAN, x, y, z + 330.0f, o, TEMPSUMMON_MANUAL_DESPAWN)))
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
        if (HayCentro())
        {
            if (npc_Centro->SummonGameObject(OBJETO_COFRE, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation(), 0, 0, 0, 0, 60))
            {
                return true;
            }
        }
        return false;
    }

    bool ActualizarZonaSegura(float& scale)
    {
        if (scale < 0.2f)
        {
            if (npc_Centro)
            {
                npc_Centro->CleanupsBeforeDelete();
                delete npc_Centro;
                npc_Centro = nullptr;
            }
        }
        else
        {
            npc_Centro->SetObjectScale(scale);
            scale -= 0.01f;
        }
        return true;
    }

    float DistanciaDelCentro(Player* player) { return HayCentro() ? player->GetExactDist(npc_Centro): 0.0f; }; // TODO: GetDist
    bool EstaLaZonaActiva() { return HayCentro(); }; // TODO: Esto esta repetido.

    bool HayCentro() { return npc_Centro ? true : false; };
    bool HayNave() { return obj_Nave ? true : false; };
    bool HayGuardian() { return npc_Guardian ? true : false; };

    bool HechizoGuardian(uint32 spell, Player* player)
    {
        if (HayGuardian() && player && spell > 0)
        {
            return npc_Guardian->CastSpell(player, spell, true) == SPELL_CAST_OK;
        }
        return false;
    }


private:
    GameObject* obj_Nave;

    Creature* npc_Vendedor;
    Creature* npc_Centro;
    Creature* npc_Guardian;

    bool zonaActiva;

};

#define sBRObjetosMgr BRObjetosMgr::instance()

#endif
