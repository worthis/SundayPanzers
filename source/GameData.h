#pragma once

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