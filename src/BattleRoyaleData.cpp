#include "BattleRoyaleData.h"

BattleRoyaleData::BattleRoyaleData()
{
    last_z = 0.0f;
    start_x = 0.0f;
    start_y = 0.0f;
    start_z = 0.0f;
    start_o = 0.0f;
}

BattleRoyaleData::~BattleRoyaleData()
{
}

void BattleRoyaleData::SetPosition(float x, float y, float z, float o)
{
    start_x = x;
    start_y = y;
    start_z = z;
    start_o = o;
}

void BattleRoyaleData::SetLast(float last)
{
    last_z = last;
}

float BattleRoyaleData::GetLast()
{
    return last_z;
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

bool BattleRoyaleData::IsHackingZ(float z)
{
    if (last_z > z)
    {
        last_z = z;
        return false;
    }
    if (z - last_z > 50.0f) return true;
    last_z = z;
    return false;
}
