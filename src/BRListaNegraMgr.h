#ifndef SC_BR_BL_MGR_H
#define SC_BR_BL_MGR_H

#include "Player.h"

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

    std::string EstaBloqueado(uint32 guid)
    {
        if (list_Negra.find(guid) != list_Negra.end())
        {
            return list_Negra[guid];
        }
        return ""; // TODO: Devolver un struct que tenga la veracidad y la causa.
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
