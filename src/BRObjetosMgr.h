#ifndef SC_BR_SOUNDS_MGR_H
#define SC_BR_SOUNDS_MGR_H

#include "GameObject.h"
#include "MapMgr.h"
#include "Transport.h"

enum BR_ObjetosMundo
{
    OBJETO_NAVE                             = 194675,
    OBJETO_COFRE                            = 499999,
    OBJETO_CENTRO_DEL_MAPA                  = 500000,
    OBJETO_ZONA_SEGURA_INICIAL              = 500001,
};

class BRObjetosMgr
{
    BRObjetosMgr(){};
    ~BRObjetosMgr(){};

public:
    static BRObjetosMgr *instance()
    {
        static BRObjetosMgr *instance = new BRObjetosMgr();
        return instance;
    }

    bool InvocarNave(Position pos)
    {
        int mapID = (*mapaActual).second->idMapa;
        Map* map = sMapMgr->FindBaseNonInstanceMap(mapID);
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
                return true;
            }
            else
            {
                delete obj_Nave;
                obj_Nave = nullptr;
            }
        }
        return false;
    };
    bool BattleRoyaleMgr::InvocarCentroDelMapa()
    {
        if (HayJugadores())
        {
            int mapID = (*mapaActual).second->idMapa;
            Map* map = sMapMgr->FindBaseNonInstanceMap(mapID);
            if (map)
            {
                DesaparecerCentro();
                float x = (*mapaActual).second->centroMapa.GetPositionX();
                float y = (*mapaActual).second->centroMapa.GetPositionY();
                float z = (*mapaActual).second->centroMapa.GetPositionZ();
                float o = (*mapaActual).second->centroMapa.GetOrientation();
                map->LoadGrid(x, y);
                obj_Centro = new GameObject();
                if (obj_Centro->Create(map->GenerateLowGuid<HighGuid::GameObject>(), OBJETO_CENTRO_DEL_MAPA, map, DIMENSION_EVENTO, x, y, z, o, G3D::Quat(), 100, GO_STATE_READY))
                {
                    obj_Centro->SetVisibilityDistanceOverride(VisibilityDistanceType::Infinite);
                    map->AddToMap(obj_Centro);
                    return true;
                }
                else
                {
                    LOG_ERROR("br.nave", "BattleRoyaleMgr::InvocarCentroDelMapa: No se ha podido invocar el centro (OBJETO = {})!", OBJETO_CENTRO_DEL_MAPA);
                    delete obj_Centro;
                    obj_Centro = nullptr;
                }
            }
            else
            {
                LOG_ERROR("br.nave", "BattleRoyaleMgr::InvocarCentroDelMapa: No se ha podido obtener el mapa para el centro (MAPA: {})!", mapID);
            }
        }
        else
        {
            LOG_ERROR("br.nave", "BattleRoyaleMgr::InvocarCentroDelMapa: No se ha invocado el centro (OBJETO = {}) porque no hay jugadores!", OBJETO_CENTRO_DEL_MAPA);
        }
        return false;
    };
    bool InvocarZonaSegura()
    {
        if (HayJugadores())
        {
            int mapID = (*mapaActual).second->idMapa;
            Map* map = sMapMgr->FindBaseNonInstanceMap(mapID);
            if (map)
            {
                AlReducirseLaZona();
                DesaparecerZona();
                if (indiceDeZona < CANTIDAD_DE_ZONAS)
                {
                    float x = (*mapaActual).second->centroMapa.GetPositionX();
                    float y = (*mapaActual).second->centroMapa.GetPositionY();
                    float z = (*mapaActual).second->centroMapa.GetPositionZ() + BR_EscalasDeZonaSegura[indiceDeZona] * 66.0f;
                    float o = (*mapaActual).second->centroMapa.GetOrientation();
                    map->LoadGrid(x, y);
                    obj_Zona = new GameObject();
                    if (obj_Zona->Create(map->GenerateLowGuid<HighGuid::GameObject>(), OBJETO_ZONA_SEGURA_INICIAL + indiceDeZona, map, DIMENSION_EVENTO, x, y, z, o, G3D::Quat(), 100, GO_STATE_READY))
                    {
                        obj_Zona->SetVisibilityDistanceOverride(VisibilityDistanceType::Infinite);
                        map->AddToMap(obj_Zona);
                        indiceDeZona++;
                        estaLaZonaActiva = true;
                        return true;
                    }
                    else
                    {
                        LOG_ERROR("br.nave", "BattleRoyaleMgr::InvocarZonaSegura: No se ha podido invocar la zona (OBJETO = {})!", OBJETO_ZONA_SEGURA_INICIAL + indiceDeZona);
                        delete obj_Zona;
                        obj_Zona = nullptr;
                    }
                }
                else
                {
                    estaLaZonaActiva = false;
                    return true;
                }
            }
            else
            {
                LOG_ERROR("br.nave", "BattleRoyaleMgr::InvocarZonaSegura: No se ha podido obtener el mapa para la zona (MAPA: {})!", mapID);
            }
        }
        else
        {
            LOG_ERROR("br.nave", "BattleRoyaleMgr::InvocarZonaSegura: No se ha invocado la zona (OBJETO = {}) porque no hay jugadores!", OBJETO_ZONA_SEGURA_INICIAL + indiceDeZona);
        }
        return false;
    };
    void DesaparecerTodosLosObjetos()
    {
        DesaparecerZona();
        DesaparecerCentro();
        DesaparecerNave();
    };
    bool DesaparecerZona()
    {
        if (obj_Zona) {
            obj_Zona->CleanupsBeforeDelete();
            delete obj_Zona;
            obj_Zona = nullptr;
            return true;
        }
        return false;
    };
    bool DesaparecerCentro()
    {
        if (obj_Centro) {
            obj_Centro->CleanupsBeforeDelete();
            delete obj_Centro;
            obj_Centro = nullptr;
            return true;
        }
        return false;
    };
    bool DesaparecerNave()
    {
        if (obj_Nave) {
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
    };


private:
    GameObject* obj_Zona;
    GameObject* obj_Centro;
    GameObject* obj_Nave;

};

#define sBRObjetosMgr BRObjetosMgr::instance()

#endif