#ifndef SC_EPDATA_H
#define SC_EPDATA_H

#include "BattleRoyaleMgr.h"

class BattleRoyaleData
{
public:
    BattleRoyaleData();
    ~BattleRoyaleData();

    void SetPosition(float x, float y, float z, float o);
    void SetLast(float last);
    float GetLast();
    float GetX();
    float GetY();
    float GetZ();
    float GetO();
    bool IsHackingZ(float z);    

private:
    float last_z;
    float start_x;
    float start_y;
    float start_z;
    float start_o;    
    
};

#endif