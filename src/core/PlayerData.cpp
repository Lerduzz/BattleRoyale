#include "PlayerData.h"

PlayerData::PlayerData()
{
    start_map = 0;
    start_x = 0.0f;
    start_y = 0.0f;
    start_z = 0.0f;
    start_o = 0.0f;
    dmg_tick = 0;
    kills = 0;
    spect = 0;
    reward = 0;
}

PlayerData::~PlayerData()
{
}

void PlayerData::SetPosition(int m, float x, float y, float z, float o)
{
    start_map = m;
    start_x = x;
    start_y = y;
    start_z = z;
    start_o = o;
}

int PlayerData::GetMap()
{
    return start_map;
}

float PlayerData::GetX()
{
    return start_x;
}

float PlayerData::GetY()
{
    return start_y;
}

float PlayerData::GetZ()
{
    return start_z;
}

float PlayerData::GetO()
{
    return start_o;
}
