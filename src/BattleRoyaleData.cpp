#include "BattleRoyaleData.h"

BattleRoyaleData::BattleRoyaleData()
{
    start_map = 0;
    start_x = 0.0f;
    start_y = 0.0f;
    start_z = 0.0f;
    start_o = 0.0f;
    dmg_tick = 0;
    kills = 0;
}

BattleRoyaleData::~BattleRoyaleData()
{
}

void BattleRoyaleData::SetPosition(int m, float x, float y, float z, float o)
{
    start_map = m;
    start_x = x;
    start_y = y;
    start_z = z;
    start_o = o;
}

int BattleRoyaleData::GetMap()
{
    return start_map;
}

float BattleRoyaleData::GetX()
{
    return start_x;
}

float BattleRoyaleData::GetY()
{
    return start_y;
}

float BattleRoyaleData::GetZ()
{
    return start_z;
}

float BattleRoyaleData::GetO()
{
    return start_o;
}
