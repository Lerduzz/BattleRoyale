#ifndef SC_BR_BL_MGR_H
#define SC_BR_BL_MGR_H

#include "Player.h"

struct BR_Bloqueado
{
    bool estaBloqueado;
    std::string motivo;
};

typedef std::map<uint32, std::string> BR_ListaNegra;

class BRListaNegraMgr
{
    BRListaNegraMgr()
    {
        RecargarLista();
    };
    ~BRListaNegraMgr()
    {
        list_Negra.clear();
    };

public:
    static BRListaNegraMgr *instance()
    {
        static BRListaNegraMgr *instance = new BRListaNegraMgr();
        return instance;
    }

    BR_Bloqueado* EstaBloqueado(uint32 guid)
    {
        BR_Bloqueado* resultado = new BR_Bloqueado();
        if (list_Negra.find(guid) != list_Negra.end())
        {
            resultado->estaBloqueado = true;
            resultado->motivo = list_Negra[guid];
        }
        else
        {
            resultado->estaBloqueado = false;
            resultado->motivo = "";
        }
        return resultado;
    };
    void RecargarLista()
    {
        list_Negra.clear();
        QueryResult result = WorldDatabase.Query("SELECT `guid`, `reason` FROM `battleroyale_blacklist`;");
        if (result)
        {
            do
            {
                Field* fields      = result->Fetch();
                uint32 guid        = fields[0].Get<uint32>();
                std::string reason = fields[1].Get<std::string>();
                list_Negra[guid]   = reason;
            } while (result->NextRow());
        }
    };

private:
    BR_ListaNegra list_Negra;

};

#define sBRListaNegraMgr BRListaNegraMgr::instance()

#endif
