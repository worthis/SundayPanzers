#pragma once

#include "TankSystem.h"
#include "GameData.h"

// ============================================================
// DBP: player(13,4)
// player(p,1) = тип танка (0 = нет)
// player(p,3) = AI level
// ============================================================
struct PlayerTankInfo
{
    int type = 0; // player(p,1)
    int ai = 0;   // player(p,3)
};

// ============================================================
// makesortie(l, se, sg) — точный порт DBP
// l  = level (gam(15))
// se = enemy squad color (gam(19))
// sg = guest squad color (gam(20))
// ============================================================
void makeSortie(TankSystem &tankSystem,
                int level,
                int playerSquad,   // DBP: gam(7)
                int enemySquad,    // DBP: gam(19)
                int guestSquad,    // DBP: gam(20)
                const PlayerTankInfo player[13],
                int &playerCommander);