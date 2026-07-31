#include "Utils.h"
#include "raylib.h"
#include <cmath>

int rnd(int n)
{
    if (n <= 0)
        return 0;
    return GetRandomValue(0, n - 1);
}

float sinDeg(float ang)
{
    return sinf(ang * DEG2RAD);
}

float cosDeg(float ang)
{
    return cosf(ang * DEG2RAD);
}

float atanDeg(float v)
{
    return atanf(v) * RAD2DEG;
}

float wrapValue(float angle)
{
    angle = fmodf(angle, 360.0f);
    if (angle < 0.0f)
        angle += 360.0f;
    return angle;
}

float newXValue(float x, float angle, float dist)
{
    return x + sinDeg(angle) * dist;
}

float newZValue(float z, float angle, float dist)
{
    return z + cosDeg(angle) * dist;
}