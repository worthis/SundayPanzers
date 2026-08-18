#include "SortieSystem.h"
#include "TankSystem.h"
#include "Utils.h"

// ============================================================
// makesortie — точный порт DBP makesortie(l, se, sg)
// ============================================================
void makeSortie(TankSystem &tankSystem,
                int level,
                int playerSquad, // DBP: gam(7)
                int enemySquad,  // DBP: gam(19)
                int guestSquad,  // DBP: gam(20)
                const PlayerTankInfo player[13],
                int &playerCommander)
{
    int l = level - 1; // DBP: 1-based → 0-based

    // ========================================================
    // 1. ENEMY SQUAD (13-40)
    // DBP: ne=spec(l,6)
    // ========================================================
    int ne = spec[l].numEnemy;

    for (int e = 0; e <= ne; e++)
    {
        int idx = ENEMY_MIN + e; // 13 + e

        // DBP: tk#(e,30)=spec(l,1)
        //      if rnd(100)>65 then tk#(e,30)=spec(l,2)
        int aiType = spec[l].ai1;
        if (rnd(100) > 65)
            aiType = spec[l].ai2;

        // DBP: tk#(e,26)=0:tk#(e,5)=180
        // DBP: tk#(e,32)=spec(l,3)
        // DBP: tk#(e,31)=spec(l,4)-rnd(4)
        int aimRatio = spec[l].aim;
        int fireRatio = spec[l].fireRate - rnd(4);

        // DBP: what=1+rnd(15):if what>15 then what=15
        //      tipe=casenemy(l,what)
        int what = 1 + rnd(15);
        if (what > 15)
            what = 15;
        int tipe = casenemy[l][what - 1]; // 1-based → 0-based

        // DBP: tankloader(e, tipe, se)
        tankSystem.loadTank(idx, tipe, enemySquad);

        // DBP: energy decreaser
        // tk#(e,37)=(tk#(e,37)*100)/spec(l,5)
        TankData &tk = tankSystem.getTankMut(idx);
        tk.energy = (tk.energy * 100.0f) / spec[l].energyDecreaser;
        tk.maxEnergy = tk.energy;

        // AI параметры
        tk.aiType = aiType;
        tk.aimRatio = aimRatio;
        tk.fireRatio = fireRatio;
        tk.target = 0;
        tk.yaw = 180.0f; // DBP: head at bottom of the map

        // DBP: positioning
        // degx=4000/(2+ne):v=e-12
        // tk#(e,1)=500+(degx*v)
        // tk#(e,2)=-1000
        // tk#(e,3)=5000
        float degx = 4000.0f / (2.0f + ne);
        float v = (float)(e + 1); // DBP: v=e-12, e=13 → v=1
        tk.x = 500.0f + degx * v;
        tk.y = -1000.0f; // падает на землю через гравитацию
        tk.z = 5000.0f;
    }

    // ========================================================
    // 2. GUEST SQUAD (41-45)
    // DBP: ne=0
    //      if spec(l,7)>0 then ne=1+rnd(spec(l,7)):if ne>4 then ne=4
    // ========================================================
    int ng = 0;
    if (spec[l].numGuest > 0)
    {
        ng = 1 + rnd(spec[l].numGuest);
        if (ng > 4)
            ng = 4;
    }

    if (ng > 0)
    {
        ng = ng - 1; // DBP: ne=ne-1

        for (int e = 0; e <= ng; e++)
        {
            int idx = GUEST_MIN + e; // 41 + e

            // DBP: tk#(e,30)=spec(l,1)
            int aiType = spec[l].ai1;

            // DBP: tk#(e,26)=0:tk#(e,5)=rnd(359)
            // DBP: tk#(e,32)=spec(l,3)+3 (more idiot than enemy)
            // DBP: tk#(e,31)=spec(l,4)
            int aimRatio = spec[l].aim + 3;
            int fireRatio = spec[l].fireRate;

            // DBP: tipe selection
            int what = 1 + rnd(15);
            if (what > 15)
                what = 15;
            int tipe = casenemy[l][what - 1];

            // DBP: tankloader(e, tipe, sg)
            tankSystem.loadTank(idx, tipe, guestSquad);

            // DBP: energy decreaser
            TankData &tk = tankSystem.getTankMut(idx);
            tk.energy = (tk.energy * 100.0f) / spec[l].energyDecreaser;
            tk.maxEnergy = tk.energy;

            // AI параметры
            tk.aiType = aiType;
            tk.aimRatio = aimRatio;
            tk.fireRatio = fireRatio;
            tk.target = 0;
            tk.yaw = (float)rnd(359); // DBP: placed at random

            // DBP: positioning
            // degx=4000/(2+ne):v=e-40
            // tk#(e,1)=500+(degx*v)
            // tk#(e,2)=-1000
            // tk#(e,3)=2200+rnd(600)
            float degx = 4000.0f / (2.0f + ng);
            float v = (float)(e + 1); // DBP: v=e-40, e=41 → v=1
            tk.x = 500.0f + degx * v;
            tk.y = -1000.0f;
            tk.z = 2200.0f + (float)rnd(600);
        }
    }

    // ========================================================
    // 3. PLAYER SQUAD (1-12)
    // DBP: npa=0:for np=1 to 12:if player(np,1)>0 then inc npa
    // ========================================================
    int npa = 0;
    for (int np = 1; np <= 12; np++)
    {
        if (player[np].type > 0)
            npa++;
    }

    int entry = 0;
    for (int p = 1; p <= 12; p++)
    {
        if (player[p].type <= 0)
            continue;

        // DBP: if gam(1)=0 then gam(1)=p
        if (playerCommander == 0)
            playerCommander = p;

        entry++;

        // DBP: tk#(p,30)=player(p,3)
        int aiType = player[p].ai;

        // DBP: tk#(p,26)=0:tk#(p,5)=0 (head at top of the map)
        // DBP: tk#(p,32)=22-player(p,3)*1.4-rnd(1)
        // DBP: tk#(p,31)=980-player(p,3)*1.4-rnd(15)
        int aimRatio = (int)(22.0f - player[p].ai * 1.4f - rnd(1));
        int fireRatio = (int)(980.0f - player[p].ai * 1.4f - rnd(15));

        // DBP: tankloader(p, tip, sp)
        int tip = player[p].type;
        int sp = playerSquad; // DBP: sp=gam(7) — player squad color

        tankSystem.loadTank(p, tip, sp);

        // AI параметры
        TankData &tk = tankSystem.getTankMut(p);
        tk.aiType = aiType;
        tk.aimRatio = aimRatio;
        tk.fireRatio = fireRatio;
        tk.target = 0;
        tk.yaw = 0.0f; // DBP: head at top of the map

        // DBP: positioning
        // degx=4000/(1+npa)
        // tk#(p,1)=500+(degx*entry)
        // tk#(p,2)=-1000
        // tk#(p,3)=100
        float degx = 4000.0f / (1.0f + npa);
        tk.x = 500.0f + degx * entry;
        tk.y = -1000.0f;
        tk.z = 100.0f;
    }
}