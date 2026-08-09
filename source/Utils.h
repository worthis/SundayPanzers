#pragma once
#include "raylib.h"

// DBP: rnd(n) → целое от 0 до n-1
int rnd(int n);

// DBP: sin/cos в градусах
float sinDeg(float ang);
float cosDeg(float ang);

float atanDeg(float v);

// DBP: wrapvalue(angle) → 0..359
float wrapValue(float angle);

// DBP: newxvalue(x, angle, dist) / newzvalue(z, angle, dist)
float newXValue(float x, float angle, float dist);
float newZValue(float z, float angle, float dist);

// Расчет центра bounding box меша
Vector3 ComputeMeshCenter(const Mesh &mesh);

// Загрузка текстуры с конверсией черного цвета в прозрачный
Texture2D LoadTextureColorKey(const char *path);