#pragma once

// ============================================================
// Данные одной пули — порт bul#(50,4)
// ============================================================
struct BulletData
{
    bool active = false;
    int owner = 0;
    float x = 0, y = 0, z = 0;
    float dirX = 0, dirY = 0, dirZ = 1;
    float gravCounter = 0;
    int lifeCounter = 0;
    float bulletPower = 0;
    float bulletGravity = 0;
    int ownerSquadId = 0;
    bool superBullet = false;
    float hitScale = 1.0f;
    float bulletScale = 1.0f; // масштаб модели пули (DBP %)
    int bulletType = 1;       // 1 = hit, 2 = hit2
};

// ============================================================
// Эффект попадания — порт hit(50) + object n+300
// ============================================================
struct HitEffect
{
    bool active = false;
    float x = 0, y = 0, z = 0;
    int counter = 0;
    float angleY = 0;
    float magnifier = 1.0f;
    int modelType = 1; // ← НОВОЕ: 1=hit, 2=hit2
};

// ============================================================
// Взрыв — порт gam(4,5,6) + objects 401,402
// ============================================================
struct ExplosionData
{
    bool active = false;
    float x = 0, y = 0, z = 0;
    int counter = 0;
    int direction = -1;
    float range = 35.0f;
    float ringYaw = 0;
    float ringPitch = 0;
    float ringRoll = 0;
    float ballYaw = 0; // случайный Y для шара
};

// ============================================================
// Данные уровней — точный порт DBP DATA-секции (initialize)
//
// casenemy[level][15] — 15 вариантов выбора типа танка
// spec[level][7]      — параметры сложности
// lev[level][16]      — данные игрока (кредиты + сценарии)
// ============================================================

struct LevelSpec
{
    int ai1;             // spec(l,1) — AI type primary
    int ai2;             // spec(l,2) — AI type secondary
    int aim;             // spec(l,3) — aim ratio
    int fireRate;        // spec(l,4) — fire rate
    int energyDecreaser; // spec(l,5) — energy decreaser
    int numEnemy;        // spec(l,6) — number of enemy tanks
    int numGuest;        // spec(l,7) — number of guest tanks
};

struct LevelData
{
    int credits;      // lev(l,0)
    int scenario[15]; // lev(l,1-15)
};

// DBP: casenemy(50,15), spec(50,7), lev(50,15)
extern int casenemy[50][15];
extern LevelSpec spec[50];
extern LevelData lev[50];

void initGameData();