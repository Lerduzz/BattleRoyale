#ifndef SC_BR_SOUNDS_MGR_H
#define SC_BR_SOUNDS_MGR_H

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

};

#define sBRObjetosMgr BRObjetosMgr::instance()

#endif