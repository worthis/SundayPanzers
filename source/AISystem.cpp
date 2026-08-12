#include "AISystem.h"
#include "Utils.h"
#include <cmath>

void AISystem::init(AudioSystem *audioSystem, TankSystem *tankSystem)
{
    this->audioSystem = audioSystem;
    this->tankSystem = tankSystem;
}

// ============================================================
// Глобальный тик AI (вызывается 1 раз за фиксированный тик)
// DBP: gam(11)=gam(11)+1 : if gam(11)>300 then gam(11)=0+rnd(10)
// ============================================================
void AISystem::update()
{
    changeTargetClock++;
    if (changeTargetClock > 300)
        changeTargetClock = rnd(10);

    // Уменьшение hitCounter для всех танков
    // DBP: if tk#(n,52)>0 then tk#(n,52)=tk#(n,52)-1
    for (int n = PLAYER_MIN; n < COMBAT_MAX; n++)
    {
        TankData &tk = tankSystem->getTankMut(n);
        if (tk.type == 0)
            continue;
        if (tk.hitCounter > 0)
            tk.hitCounter--;
    }
}

// ============================================================
// Дистанции
// ============================================================
float AISystem::range2D(int a, int b) const
{
    const TankData &ta = tankSystem->getTank(a);
    const TankData &tb = tankSystem->getTank(b);
    float dx = ta.x - tb.x;
    float dz = ta.z - tb.z;
    return sqrtf(dx * dx + dz * dz);
}

float AISystem::range3D(int a, int b) const
{
    const TankData &ta = tankSystem->getTank(a);
    const TankData &tb = tankSystem->getTank(b);
    float dx = ta.x - tb.x;
    float dz = ta.z - tb.z;
    float dy = ta.y - tb.y;
    float r2 = dx * dx + dz * dz;
    return sqrtf(r2 + dy * dy);
}

// ============================================================
// Поиск цели
// DBP: cta-логика + поиск ближайшего врага + powerup
// ============================================================
int AISystem::findTarget(int n) const
{
    const TankData &tk = tankSystem->getTank(n);

    // Нужно ли менять цель?
    bool cta = false;
    int vt = tk.target;

    // DBP: if tk#(n,26)=0 or tk#(vt,0)<=0 then cta=1
    if (vt == 0 || tankSystem->getTank(vt).type <= 0)
        cta = true;

    // DBP: if cta=0 and rnd(1000)>(999-(tk#(n,30)/1.5)) then cta=1
    if (!cta && rnd(1000) > (int)(999.0f - tk.aiType / 1.5f))
        cta = true;

    // DBP: if gam(11)=0 and rnd(100)>50 then cta=1
    if (!cta && changeTargetClock == 0 && rnd(100) > 50)
        cta = true;

    if (!cta)
        return vt; // цель остаётся

    // === Поиск новой цели ===
    int tggot = 0;
    float tgmin = 15500.0f;

    // DBP: четыре ветки по принадлежности танка
    if (n < 13) // player squad → ищет enemy + guest
    {
        for (int t = 13; t <= 45; t++)
        {
            const TankData &tt = tankSystem->getTank(t);
            if (tt.type > 0 && tt.barrierCounter < 1000)
            {
                float r = range2D(n, t);
                if (r < tgmin)
                {
                    tgmin = r;
                    tggot = t;
                }
            }
        }
    }
    else if (n > 12 && n < 41) // enemy squad → ищет player + guest
    {
        for (int t = 1; t <= 12; t++)
        {
            const TankData &tt = tankSystem->getTank(t);
            if (tt.type > 0 && tt.barrierCounter < 1000)
            {
                float r = range2D(n, t);
                if (r < tgmin)
                {
                    tgmin = r;
                    tggot = t;
                }
            }
        }
        for (int t = 41; t <= 45; t++)
        {
            const TankData &tt = tankSystem->getTank(t);
            if (tt.type > 0 && tt.barrierCounter < 5)
            {
                float r = range2D(n, t);
                if (r < tgmin)
                {
                    tgmin = r;
                    tggot = t;
                }
            }
        }
    }
    else if (n > 40 && n < 46) // guest squad → ищет всех
    {
        for (int t = 1; t <= 40; t++)
        {
            const TankData &tt = tankSystem->getTank(t);
            if (tt.type > 0 && tt.barrierCounter < 100)
            {
                float r = range2D(n, t);
                if (r < tgmin)
                {
                    tgmin = r;
                    tggot = t;
                }
            }
        }
    }
    else if (n > 45) // extra → ищет всех
    {
        for (int t = 1; t <= 40; t++)
        {
            if (tankSystem->getTank(t).type > 0)
            {
                float r = range2D(n, t);
                if (r < tgmin)
                {
                    tgmin = r;
                    tggot = t;
                }
            }
        }
    }

    // DBP: powerup search
    // if tk#(n,50)<=0
    //   if rnd(100)>gam(12) and tgmin>400 then tggot=0:tgmin=15000
    //   for t=51 to 55 ...
    if (tk.barrierCounter <= 0)
    {
        float threshold = (n > 12 && n < 41) ? 500.0f : 400.0f;
        if (rnd(100) > powupSearchFactor && tgmin > threshold)
        {
            tggot = 0;
            tgmin = 15000.0f;
        }
        for (int t = 51; t <= 55; t++)
        {
            if (tankSystem->getTank(t).type > 0)
            {
                float r = range2D(n, t);
                if (r < tgmin)
                {
                    tgmin = r;
                    tggot = t;
                }
            }
        }
    }

    return tggot;
}

// ============================================================
// Вычисление управления для AI-танка
// Полный порт секции "computer control" из DBP tanks()
// ============================================================
AIOutput AISystem::computeInput(int n)
{
    AIOutput out;

    if (!tankSystem)
        return out;

    TankData &tk = tankSystem->getTankMut(n);
    if (tk.type <= 0)
        return out;

    // ============================================================
    // EXTRA (n 46..50) — DBP:
    //   target = ближайший танк 1-40
    //   if n>45 and tk#(n,0)>0 then yj#=-1  (всегда вперёд)
    // ============================================================
    if (n >= EXTRA_MIN && n <= EXTRA_MAX)
    {
        // target designation: ближайший танк
        if (tk.target == 0 || tankSystem->getTank(tk.target).type <= 0)
        {
            float tgmin = MAP_SIZE * MAP_SIZE;
            int tggot = 0;
            for (int t = PLAYER_MIN; t <= TANKS_MAX; t++)
            {
                const TankData &extraTarget = tankSystem->getTank(t);
                if (extraTarget.type > 0)
                {
                    float dx = tk.x - extraTarget.x;
                    float dz = tk.z - extraTarget.z;
                    float r = sqrtf(dx * dx + dz * dz);
                    if (r < tgmin)
                    {
                        tgmin = r;
                        tggot = t;
                    }
                }
            }
            if (tggot > 0)
                tk.target = tggot;
        }

        if (tk.target > 0)
        {
            const TankData &tg = tankSystem->getTank(tk.target);
            if (tg.type > 0)
            {
                float dx = tg.x - tk.x;
                float dz = tg.z - tk.z;
                float r = sqrtf(dx * dx + dz * dz);

                // Вычисляем угол к цели (аналог point object в DBP)
                float ry = atan2f(dx, dz) * RAD2DEG;
                ry = wrapValue(ry);

                float tanAngle = wrapValue(tk.yaw - ry);
                float ta = fabsf(tanAngle);

                // aim = 20 для животных (tk#(n,32)=20 из tankloader)
                float aim = 20.0f;

                if (ta >= (360.0f - aim) || ta <= aim)
                {
                    out.xj = 0.0f; // уже направлены на цель
                }
                else
                {
                    int flag = 1;
                    if (ta > 180.0f)
                        flag = -flag;
                    if (tanAngle > 0.0f)
                        out.xj = -(float)flag;
                    if (tanAngle < 0.0f)
                        out.xj = (float)flag;
                }
            }
        }

        // out.xj   = 0.0f;      // heading (поворот к цели) обрабатывается общей логикой
        out.yj = -1.0f; // всегда вперёд
        out.fire = 0;   // extra не стреляют
        return out;
    }

    // ============================================================
    // 1. TARGET DESIGNATION
    // ============================================================
    int tg = findTarget(n);
    tk.target = tg;

    if (tg == 0 || tankSystem->getTank(tg).type == 0)
        return out; // нет цели — стоим

    // ============================================================
    // 2. RANGE TO TARGET
    // DBP: r = 3D distance
    // ============================================================
    float r = range3D(n, tg);

    // ============================================================
    // 3. HEADING
    // DBP: point object 65000 → ry# = angle y
    //      if tk#(n,27)=3 then ry#=tk#(n,29)
    // ============================================================
    const TankData &tgt = tankSystem->getTank(tg);
    float ry = atan2f(tgt.x - tk.x, tgt.z - tk.z) * RAD2DEG;
    ry = wrapValue(ry);

    if (tk.aiState == 3)
        ry = tk.escapeAngle;

    // DBP: tan# = wrapvalue(tk#(n,5) - ry#)
    float tan_ = wrapValue(tk.yaw - ry);
    float ta = fabsf(tan_);

    // DBP: gd=0 : aim=tk#(n,32)
    //      if tk#(n,26)>50 then aim=5
    bool gd = false;
    float aim = (float)tk.aimRatio;
    if (tk.target > 50)
        aim = 5.0f;

    // DBP: if ta>=(360-aim) or ta<=aim → xj#=0, gd=1
    //      else → xj# = ±flag
    float xj = 0.0f;
    if (ta >= (360.0f - aim) || ta <= aim)
    {
        xj = 0.0f;
        if (tk.aiState != 3)
            gd = true;
    }
    else
    {
        float flag = 1.0f;
        if (ta > 180.0f)
            flag = -flag;
        if (tan_ > 0.0f)
            xj = -flag;
        if (tan_ < 0.0f)
            xj = flag;
    }

    // ============================================================
    // 4. FIRE CONTROL
    // DBP: if gd=1 and r<550 and rnd(1000)>tk#(n,31)
    //      and tk#(n,43)=1 and tk#(n,26)<46 then fire=1
    // ============================================================
    bool fire = false;
    if (gd && r < 550.0f &&
        rnd(1000) > tk.fireRatio &&
        tk.canFire &&
        tk.target < 46)
    {
        fire = true;
    }

    // ============================================================
    // 5. AI SELECTION (10 типов поведения)
    // DBP: if tk#(n,27)=0 → select ai
    // ============================================================
    bool tb = false; // запрос турбо

    if (tk.aiState == 0)
    {
        int ai = tk.aiType;

        switch (ai)
        {
        // ── Case 1: very stupid ──
        case 1:
            if (!gd && rnd(1000) > 945 && r < 650)
            {
                tk.aiState = 1;
                tk.aiCounter = 240 + rnd(100);
            }
            if (!gd && rnd(1000) > 945 && r < 350)
            {
                tk.aiState = 2;
                tk.aiCounter = 240 + rnd(100);
            }
            if (rnd(1000) > 955 && r < 250)
            {
                tk.aiState = 3;
                tk.aiCounter = 155 + rnd(80);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (!gd && rnd(1000) > 995 && r > 2500)
            {
                tk.aiState = 3;
                tk.aiCounter = 140 + rnd(120);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (!gd && rnd(1000) > 985 && r > 650 && r < 1750)
            {
                tk.aiState = 3;
                tk.aiCounter = 140 + rnd(120);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (gd && rnd(1000) > 955 && r < 110)
            {
                tk.aiState = 3;
                tk.aiCounter = 125 + rnd(100);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (!gd && rnd(1000) > 995 && r < 500 && tk.turboCharger <= 0)
                tb = true;
            break;

        // ── Case 2: stupid ──
        case 2:
            if (!gd && rnd(1000) > 935 && r < 650)
            {
                tk.aiState = 1;
                tk.aiCounter = 220 + rnd(100);
            }
            if (!gd && rnd(1000) > 935 && r < 350)
            {
                tk.aiState = 2;
                tk.aiCounter = 220 + rnd(100);
            }
            if (rnd(1000) > 965 && r < 250)
            {
                tk.aiState = 3;
                tk.aiCounter = 155 + rnd(80);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (!gd && rnd(1000) > 996 && r > 2500)
            {
                tk.aiState = 3;
                tk.aiCounter = 100 + rnd(120);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (!gd && rnd(1000) > 990 && r > 650 && r < 1750)
            {
                tk.aiState = 3;
                tk.aiCounter = 100 + rnd(120);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (gd && rnd(1000) > 965 && r < 110)
            {
                tk.aiState = 3;
                tk.aiCounter = 125 + rnd(100);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (!gd && rnd(1000) > 993 && r < 650 && r > 300 && tk.turboCharger <= 0)
                tb = true;
            break;

        // ── Case 3: less stupid ──
        case 3:
            if (!gd && rnd(1000) > 940 && r < 650)
            {
                tk.aiState = 1;
                tk.aiCounter = 180 + rnd(90);
            }
            if (!gd && rnd(1000) > 940 && r < 350)
            {
                tk.aiState = 2;
                tk.aiCounter = 200 + rnd(90);
            }
            if (rnd(1000) > 975 && r < 250)
            {
                tk.aiState = 3;
                tk.aiCounter = 105 + rnd(20);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (!gd && rnd(1000) > 995 && r > 2500)
            {
                tk.aiState = 3;
                tk.aiCounter = 200 + rnd(100);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (!gd && rnd(1000) > 993 && r > 650 && r < 1750)
            {
                tk.aiState = 3;
                tk.aiCounter = 100 + rnd(100);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (gd && rnd(1000) > 970 && r < 110)
            {
                tk.aiState = 3;
                tk.aiCounter = 105 + rnd(90);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (!gd && rnd(1000) > 992 && r < 1650 && r > 300 && tk.turboCharger <= 0)
                tb = true;
            break;

        // ── Case 4: good ──
        case 4:
            if (!gd && rnd(1000) > 920 && r < 550)
            {
                tk.aiState = 1;
                tk.aiCounter = 180 + rnd(90);
            }
            if (!gd && rnd(1000) > 920 && r < 200)
            {
                tk.aiState = 2;
                tk.aiCounter = 200 + rnd(90);
            }
            if (gd && rnd(1000) > 955 && r < 230)
            {
                tk.aiState = 2;
                tk.aiCounter = 50 + rnd(50);
            }
            if (rnd(1000) > 955 && r < 250)
            {
                tk.aiState = 3;
                tk.aiCounter = 55 + rnd(20);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (rnd(1000) > 995 && r > 2500)
            {
                tk.aiState = 3;
                tk.aiCounter = 200 + rnd(100);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (rnd(1000) > 990 && r > 650 && r < 1750)
            {
                tk.aiState = 3;
                tk.aiCounter = 100 + rnd(100);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (!gd && rnd(1000) > 990 && r < 1650 && r > 250 && tk.turboCharger <= 0)
                tb = true;
            break;

        // ── Case 5: damned good ──
        case 5:
            if (!gd && rnd(1000) > 935 && r < 350)
            {
                tk.aiState = 1;
                tk.aiCounter = 185 + rnd(105);
            }
            if (!gd && rnd(1000) > 920 && r < 195)
            {
                tk.aiState = 2;
                tk.aiCounter = 200 + rnd(90);
            }
            if (gd && rnd(1000) > 935 && r < 230)
            {
                tk.aiState = 2;
                tk.aiCounter = 50 + rnd(50);
            }
            if (rnd(1000) > 965 && r < 350)
            {
                tk.aiState = 3;
                tk.aiCounter = 55 + rnd(20);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (rnd(1000) > 991 && r > 750 && r < 1950)
            {
                tk.aiState = 3;
                tk.aiCounter = 100 + rnd(100);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (tk.hitCounter > 0 && rnd(1000) > 997)
            {
                tk.aiState = 3;
                tk.aiCounter = 80 + rnd(100);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (!gd && rnd(1000) > 989 && r < 1650 && r > 220 && tk.turboCharger <= 0)
                tb = true;
            break;

        // ── Case 6: skilled ──
        case 6:
            if (!gd && rnd(1000) > 935 && r < 370)
            {
                tk.aiState = 1;
                tk.aiCounter = 185 + rnd(105);
            }
            if (!gd && rnd(1000) > 920 && r < 195)
            {
                tk.aiState = 2;
                tk.aiCounter = 200 + rnd(90);
            }
            if (gd && rnd(1000) > 935 && r < 230)
            {
                tk.aiState = 2;
                tk.aiCounter = 50 + rnd(50);
            }
            if (rnd(1000) > 970 && r < 150)
            {
                tk.aiState = 3;
                tk.aiCounter = 55 + rnd(30);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (rnd(1000) > 997 && r > 750 && r < 1950)
            {
                tk.aiState = 3;
                tk.aiCounter = 100 + rnd(100);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (tk.hitCounter > 0 && rnd(1000) > 995)
            {
                tk.aiState = 3;
                tk.aiCounter = 80 + rnd(100);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (!gd && rnd(1000) > 989 && r < 1650 && r > 220 && tk.turboCharger <= 0)
                tb = true;
            break;

        // ── Case 7: very skilled ──
        case 7:
            if (!gd && rnd(1000) > 930 && r < 350)
            {
                tk.aiState = 1;
                tk.aiCounter = 185 + rnd(105);
            }
            if (!gd && rnd(1000) > 930 && r < 225)
            {
                tk.aiState = 2;
                tk.aiCounter = 200 + rnd(90);
            }
            if (gd && rnd(1000) > 935 && r < 250)
            {
                tk.aiState = 2;
                tk.aiCounter = 50 + rnd(50);
            }
            if (rnd(1000) > 972 && r < 150)
            {
                tk.aiState = 3;
                tk.aiCounter = 75 + rnd(30);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (rnd(1000) > 997 && r > 750 && r < 2050)
            {
                tk.aiState = 3;
                tk.aiCounter = 100 + rnd(100);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (tk.hitCounter > 0 && !gd && rnd(1000) > 980)
            {
                tk.aiState = 3;
                tk.aiCounter = 80 + rnd(100);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (!gd && rnd(1000) > 989 && r < 1650 && r > 220 && tk.turboCharger <= 0)
                tb = true;
            break;

        // ── Case 8: veteran ──
        case 8:
            if (!gd && rnd(1000) > 915 && r < 340)
            {
                tk.aiState = 1;
                tk.aiCounter = 185 + rnd(105);
            }
            if (rnd(1000) > 920 && r < 225)
            {
                tk.aiState = 2;
                tk.aiCounter = 210 + rnd(80);
            }
            if (rnd(1000) > 970 && r < 160)
            {
                tk.aiState = 3;
                tk.aiCounter = 80 + rnd(20);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (rnd(1000) > 997 && r > 750 && r < 2050)
            {
                tk.aiState = 3;
                tk.aiCounter = 100 + rnd(100);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (tk.hitCounter > 0 && !gd && rnd(1000) > 975)
            {
                tk.aiState = 3;
                tk.aiCounter = 80 + rnd(100);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (!gd && rnd(1000) > 989 && r < 1650 && r > 320 && tk.turboCharger <= 0)
                tb = true;
            break;

        // ── Case 9: super veteran ──
        case 9:
            if (!gd && rnd(1000) > 910 && r < 330)
            {
                tk.aiState = 1;
                tk.aiCounter = 185 + rnd(105);
            }
            if (rnd(1000) > 920 && r < 260)
            {
                tk.aiState = 2;
                tk.aiCounter = 200 + rnd(70);
            }
            if (rnd(1000) > 970 && r < 170)
            {
                tk.aiState = 3;
                tk.aiCounter = 90 + rnd(20);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (rnd(1000) > 997 && r > 700 && r < 1150)
            {
                tk.aiState = 3;
                tk.aiCounter = 100 + rnd(100);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (tk.hitCounter > 0 && !gd && rnd(1000) > 975)
            {
                tk.aiState = 3;
                tk.aiCounter = 80 + rnd(100);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (!gd && rnd(1000) > 989 && r < 1650 && r > 320 && tk.turboCharger <= 0)
                tb = true;
            break;

        // ── Case 10: the ultimate ──
        case 10:
            if (!gd && rnd(1000) > 910 && r < 310)
            {
                tk.aiState = 1;
                tk.aiCounter = 185 + rnd(105);
            }
            if (rnd(1000) > 920 && r < 260)
            {
                tk.aiState = 2;
                tk.aiCounter = 180 + rnd(20);
            }
            if (rnd(1000) > 980 && r < 180)
            {
                tk.aiState = 3;
                tk.aiCounter = 90 + rnd(20);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (rnd(1000) > 998 && r > 750 && r < 1150)
            {
                tk.aiState = 3;
                tk.aiCounter = 100 + rnd(100);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (tk.hitCounter > 0 && rnd(1000) > 978)
            {
                tk.aiState = 3;
                tk.aiCounter = 80 + rnd(100);
                tk.escapeAngle = ry + 90.0f + rnd(180);
            }
            if (!gd && rnd(1000) > 989 && r < 1650 && r > 320 && tk.turboCharger <= 0)
                tb = true;
            break;
        }
    }

    // ============================================================
    // 6. AI COUNTER DECREASE
    // DBP: if tk#(n,28)>0 then tk#(n,28)--
    //      if tk#(n,28)<=0 then tk#(n,27)=0 : tk#(n,29)=0
    // ============================================================
    if (tk.aiCounter > 0)
    {
        tk.aiCounter--;
        if (tk.aiCounter <= 0)
        {
            tk.aiState = 0;
            tk.escapeAngle = 0.0f;
        }
    }

    // ============================================================
    // 7. AI EFFECT → yj
    // DBP:
    //   if tk#(n,27)=0 and r>(220+tk#(n,33)/2) then yj#=-1
    //   if tk#(n,27)=1 then yj#=0
    //   if tk#(n,27)=2 then yj#=1
    //   if tk#(n,27)=3 then yj#=-1
    // ============================================================
    float yj = 0.0f;

    if (tk.aiState == 0 && r > (220.0f + tk.collisionRange / 2.0f))
        yj = -1.0f;
    else if (tk.aiState == 1)
        yj = 0.0f;
    else if (tk.aiState == 2)
        yj = 1.0f;
    else if (tk.aiState == 3)
        yj = -1.0f;

    // ============================================================
    // 8. TURBO
    // DBP: if tb=1 then tk#(n,44)=tk#(n,46) : tk#(n,45)=tk#(n,47)
    // ============================================================
    if (tb)
    {
        tk.turboCounter = tk.turboTime;
        tk.turboCharger = tk.turboReload;

        if (audioSystem)
            audioSystem->playTurbo({tk.x, tk.y, tk.z});
    }

    // ============================================================
    // 9. POWERUP SEEKING
    // DBP: if tk#(n,26)>50
    //        yj#=-1 : tk#(n,27)=0
    //        if r<200 and gd=0 then yj#=0
    //        if gd=1 then yj#=-1
    // ============================================================
    if (tk.target >= POWERUP_MIN)
    {
        yj = -1.0f;
        tk.aiState = 0;
        if (r < 200.0f && !gd)
            yj = 0.0f;
        if (gd)
            yj = -1.0f;
    }

    out.xj = xj;
    out.yj = yj;
    out.fire = fire;
    return out;
}