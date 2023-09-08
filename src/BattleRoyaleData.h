#ifndef SC_BR_DATA_H
#define SC_BR_DATA_H

#include "BattleRoyaleMgr.h" // TODO: Creo que esto no hace falta aqui.

class BattleRoyaleData
{
public:
    BattleRoyaleData();
    ~BattleRoyaleData();

    void SetPosition(int m, float x, float y, float z, float o);
    int GetMap();
    float GetX();
    float GetY();
    float GetZ();
    float GetO();
    int dmg_tick;
    int kills;

private:
    int start_map;
    float start_x;
    float start_y;
    float start_z;
    float start_o;
    
};

#endif
