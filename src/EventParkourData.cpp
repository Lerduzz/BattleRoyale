#include "EventParkourData.h"

EventParkourData::EventParkourData()
{
    last_z = 0.0f;
    start_x = 0.0f;
    start_y = 0.0f;
    start_z = 0.0f;
    start_o = 0.0f;
}

EventParkourData::~EventParkourData()
{
}

void EventParkourData::SetPosition(float x, float y, float z, float o)
{
    start_x = x;
    start_y = y;
    start_z = z;
    start_o = o;
}

void EventParkourData::SetLast(float last)
{
    last_z = last;
}

float EventParkourData::GetLast()
{
    return last_z;
}

float EventParkourData::GetX()
{
    return start_x;
}

float EventParkourData::GetY()
{
    return start_y;
}

float EventParkourData::GetZ()
{
    return start_z;
}

float EventParkourData::GetO()
{
    return start_o;
}

bool EventParkourData::IsHackingZ(float z)
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
