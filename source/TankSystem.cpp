#include "TankSystem.h"
#include "Utils.h"
#include <cstdio>
#include <cmath>
#include <cfloat>
#include "rlgl.h"
#include "raymath.h"

// Центр bounding box меша — локальная позиция дула
Vector3 TankSystem::computeMeshCenter(const Mesh &mesh)
{
    if (mesh.vertexCount == 0)
        return {0, 0, 0};

    float minX = FLT_MAX, minY = FLT_MAX, minZ = FLT_MAX;
    float maxX = -FLT_MAX, maxY = -FLT_MAX, maxZ = -FLT_MAX;

    for (int i = 0; i < mesh.vertexCount; i++)
    {
        float x = mesh.vertices[i * 3 + 0];
        float y = mesh.vertices[i * 3 + 1];
        float z = mesh.vertices[i * 3 + 2];

        if (x < minX)
            minX = x;
        if (x > maxX)
            maxX = x;
        if (y < minY)
            minY = y;
        if (y > maxY)
            maxY = y;
        if (z < minZ)
            minZ = z;
        if (z > maxZ)
            maxZ = z;
    }

    return {(minX + maxX) * 0.5f, (minY + maxY) * 0.5f, (minZ + maxZ) * 0.5f};
}

TankSystem::TankSystem() : terrain(nullptr), treeSystem(nullptr)
{
    for (int i = 0; i < 9; i++)
    {
        tankModels[i] = {0};
        modelsLoaded[i] = false;
    }

    for (int i = 0; i < 11; i++)
    {
        squadTexNormal[i] = {0};
        squadTexDamaged[i] = {0};
        squadTexDestroyed[i] = {0};
        squadTexLoaded[i] = false;
    }

    for (int i = 0; i < MAX_TANKS; i++)
    {
        resetTank(i);
    }
}

TankSystem::~TankSystem()
{
    unloadTankModels();
    unloadSquadTextures();
}

void TankSystem::resetTank(int n)
{
    TankData &t = tanks[n];
    t.type = 0;
    t.baseType = 0;
    t.x = 0;
    t.y = -1000;
    t.z = 0;
    t.pitch = 0;
    t.yaw = 0;
    t.roll = 0;
    t.scaleX = 1;
    t.scaleY = 1;
    t.scaleZ = 1;
    t.steering = 0;
    t.maxSpeed = 0;
    t.spin = 0;
    t.accel = 0;
    t.fallForce = 0;
    t.walkSine = 0;
    t.walkCounter = 0;
    t.bounceAngle = 0;
    t.bounceForce = 0;
    t.fireLimb = 0;
    t.bulletCounter = 0;
    t.reloadCounter = 0;
    t.reloadTime = 0;
    t.bulletLength = 0;
    t.bulletPower = 0;
    t.bounce = 0;
    t.onGround = false;
    t.bounceRoll = 0;
    t.bouncePitch = 0;
    t.target = 0;
    t.aiState = 0;
    t.aiCounter = 0;
    t.escapeAngle = 0;
    t.aiType = 0;
    t.fireRatio = 0;
    t.aimRatio = 0;
    t.collisionRange = 0;
    t.squadId = 0;
    t.damaged = false;
    t.shotAngle = 0;
    t.energy = 0;
    t.rpm = 0;
    t.hitScale = 0;
    t.soundStart = 0;
    t.collisionHeight = 0;
    t.bulletGravity = 0;
    t.canFire = false;
    t.turboCounter = 0;
    t.turboCharger = 0;
    t.turboTime = 0;
    t.turboReload = 0;
    t.pictureId = 0;
    t.originalEnergy = 0;
    t.barrierCounter = 0;
    t.bulletFlag = 0;
    t.hitCounter = 0;
}

void TankSystem::initTankTypes()
{
    // === Тип 1 (из DBP tankloader case 1) ===
    tankTypes[1] = {
        1.0f, // scale 100,110,100
        1.1f,
        1.0f,
        0.99f,    // steering
        4.0f,     // maxSpeed
        1,        // fireLimb
        80,       // reloadTime
        75,       // bulletLength
        6.0f,     // bulletPower
        15.0f,    // collisionRange
        0.0f,     // shotAngle
        35.0f,    // energy
        0.7f,     // hitScale
        22500.0f, // soundStart
        17.0f,    // collisionHeight
        0.025f,   // bulletGravity
        350,      // turboTime
        800,      // turboReload
        1.0f,
        1};

    // === Тип 2 ===
    tankTypes[2] = {
        1.02f,
        1.05f,
        1.00f,
        0.90f,
        3.75f,
        3,
        95,
        90,
        8.0f,
        20.0f,
        -0.5f,
        50.0f,
        0.95f,
        14000.0f,
        17.5f,
        0.033f,
        300,
        600,
        1.3f,
        1};

    // === Тип 3 ===
    tankTypes[3] = {
        1.02f,
        1.05f,
        1.00f,
        0.92f,
        3.65f,
        4,
        100,
        95,
        9.0f,
        20.3f,
        -0.1f,
        60.0f,
        0.96f,
        14500.0f,
        19.5f,
        0.030f,
        300,
        700,
        1.3f,
        1};

    // === Тип 4 ===
    tankTypes[4] = {
        1.00f,
        0.98f,
        1.00f,
        0.83f,
        3.0f,
        3,
        125,
        120,
        11.0f,
        22.0f,
        0.0f,
        75.0f,
        1.0f,
        11000.0f,
        20.0f,
        0.03f,
        450,
        1500,
        1.4f,
        1};

    // === Тип 5 ===
    tankTypes[5] = {
        1.00f,
        0.95f,
        1.00f,
        0.85f,
        3.4f,
        1,
        135,
        130,
        13.0f,
        22.0f,
        -0.1f,
        90.0f,
        1.0f,
        10000.0f,
        20.0f,
        0.033f,
        450,
        1000,
        1.5f,
        1};

    // === Тип 6 ===
    tankTypes[6] = {
        1.00f,
        0.99f,
        1.01f,
        0.75f,
        2.7f,
        3,
        145,
        110,
        15.0f,
        22.0f,
        -0.15f,
        105.0f,
        1.35f,
        7000.0f,
        20.0f,
        0.033f,
        310,
        1100,
        1.8f,
        1};

    // === Тип 7 ===
    tankTypes[7] = {
        1.00f,
        1.01f,
        1.01f,
        0.80f,
        3.9f,
        1,
        115,
        100,
        20.0f,
        25.0f,
        -1.8f,
        125.0f,
        1.65f,
        5000.0f,
        21.0f,
        0.01f,
        600,
        3000,
        2.0f,
        2};

    // === Тип 8 ===
    tankTypes[8] = {
        1.05f,
        1.01f,
        1.01f,
        0.70f,
        3.1f,
        1,
        150,
        150,
        24.0f,
        29.0f,
        -1.0f,
        140.0f,
        1.75f,
        5500.0f,
        21.0f,
        0.008f,
        2000,
        5000,
        2.1f,
        2};
}

void TankSystem::init(Terrain *terrain, TreeSystem *treeSystem)
{
    this->terrain = terrain;
    this->treeSystem = treeSystem;
    initTankTypes();
    loadTankModels();
    loadSquadTextures();
}

void TankSystem::unloadTankModels()
{
    for (int i = 1; i <= 8; i++)
    {
        if (modelsLoaded[i] && tankModels[i].meshCount > 0)
        {
            UnloadModel(tankModels[i]);
            tankModels[i] = {0};
            modelsLoaded[i] = false;
        }
    }
}

void TankSystem::loadTankModels()
{
    unloadTankModels();

    for (int i = 1; i <= MAX_TANK_TYPES; i++)
    {
        char path[64];
        snprintf(path, sizeof(path), "data/tanks/t%d.glb", i);

        Model m = LoadModel(path);
        if (m.meshCount > 0)
        {
            tankModels[i] = m;
            modelsLoaded[i] = true;

            // Вычисляем локальный центр меша-дула (fireLimb)
            // нумерация мешей в DarkBasic начинается с последнего
            int fl = tankTypes[i].fireLimb;
            if (fl >= 0 && fl < m.meshCount)
            {
                muzzleLocal[i] = computeMeshCenter(m.meshes[m.meshCount - fl]);
                TraceLog(LOG_INFO, "Tank %d: fireLimb mesh %d center = (%.1f, %.1f, %.1f)",
                         i, fl, muzzleLocal[i].x, muzzleLocal[i].y, muzzleLocal[i].z);
            }
            else
            {
                // Fallback если fireLimb вне диапазона
                muzzleLocal[i] = {0, 14, 22};
                TraceLog(LOG_WARNING, "Tank %d: fireLimb %d out of range (meshCount=%d), using fallback",
                         i, fl, m.meshCount);
            }

            // DBP: set object specular n,0 : set object n,1,0,1,0,0,0,0
            // Отключаем освещение, ставим цвет WHITE
            for (int j = 0; j < m.materialCount; j++)
            {
                m.materials[j].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
                if (m.materials[j].maps[MATERIAL_MAP_DIFFUSE].texture.id != 0)
                {
                    SetTextureFilter(m.materials[j].maps[MATERIAL_MAP_DIFFUSE].texture,
                                     TEXTURE_FILTER_POINT);
                }
            }
            TraceLog(LOG_INFO, "Tank model %d loaded: %s", i, path);
        }
        else
        {
            TraceLog(LOG_WARNING, "Tank model %d NOT found: %s", i, path);
        }
    }
}

void TankSystem::loadTank(int n, int t, int c)
{
    if (n < 1 || n >= MAX_TANKS)
        return;
    if (t < 1 || t > 8)
        return;

    resetTank(n);

    TankData &tk = tanks[n];
    TankType &tt = tankTypes[t];

    // DBP: tk#(n,0)=t : tk#(n,48)=t : tk#(n,34)=c
    tk.type = t;
    tk.baseType = t;
    tk.pictureId = t;
    tk.squadId = c;

    // Копируем параметры типа
    tk.scaleX = tt.scaleX;
    tk.scaleY = tt.scaleY;
    tk.scaleZ = tt.scaleZ;
    tk.steering = tt.steering;
    tk.maxSpeed = tt.maxSpeed;
    tk.fireLimb = tt.fireLimb;
    tk.reloadTime = tt.reloadTime;
    tk.bulletLength = tt.bulletLength;
    tk.bulletPower = tt.bulletPower;
    tk.collisionRange = tt.collisionRange;
    tk.shotAngle = tt.shotAngle;
    tk.energy = tt.energy;
    tk.originalEnergy = tt.energy;
    tk.hitScale = tt.hitScale;
    tk.soundStart = tt.soundStart;
    tk.collisionHeight = tt.collisionHeight;
    tk.bulletGravity = tt.bulletGravity;
    tk.canFire = true;
    tk.turboTime = tt.turboTime;
    tk.turboReload = tt.turboReload;
    tk.bulletScale = tt.bulletScale;
    tk.hitModelType = tt.hitModelType;

    // DBP: модификаторы команды (select c)
    // Применяем бонусы/штрафы команды
    float turnMod = 1.0f, speedMod = 1.0f, powerMod = 1.0f, energyMod = 1.0f;

    switch (c)
    {
    case 1: // The Slug
        turnMod = 1.0f / 1.05f;
        speedMod = 1.0f / 1.05f;
        powerMod = 1.05f;
        energyMod = 1.05f;
        break;
    case 2: // The Braves
        turnMod = 1.05f;
        speedMod = 1.05f;
        powerMod = 1.0f / 1.05f;
        energyMod = 1.0f;
        break;
    case 3: // The Smashers
        turnMod = 1.0f / 1.07f;
        speedMod = 1.0f / 1.07f;
        powerMod = 1.10f;
        energyMod = 1.10f;
        break;
    case 4:    // The Yanks
        break; // без модификаторов
    case 5:    // The Hooks
        turnMod = 1.1f;
        speedMod = 1.1f;
        powerMod = 1.0f / 1.15f;
        energyMod = 1.1f;
        break;
    case 6: // The Boars
        turnMod = 1.1f;
        speedMod = 1.1f;
        powerMod = 1.135f;
        energyMod = 1.0f / 1.05f;
        break;
    case 7: // The Bullets
        turnMod = 1.1f;
        speedMod = 1.1f;
        powerMod = 1.135f;
        energyMod = 1.105f;
        break;
    case 8: // The Kisses
        turnMod = 1.12f;
        speedMod = 1.12f;
        powerMod = 1.25f;
        energyMod = 1.25f;
        break;
    case 9: // The Skulls
        turnMod = 1.2f;
        speedMod = 1.2f;
        powerMod = 1.35f;
        energyMod = 1.35f;
        break;
    case 10: // The Butchers
        turnMod = 1.25f;
        speedMod = 1.25f;
        powerMod = 1.55f;
        energyMod = 1.75f;
        break;
    }

    tk.steering *= turnMod;
    tk.maxSpeed *= speedMod;
    tk.bulletPower *= powerMod;
    tk.energy *= energyMod;
    tk.originalEnergy = tk.energy;

    TraceLog(LOG_INFO, "Tank %d loaded: type %d, squad %d", n, t, c);
}

void TankSystem::placeTank(int n, float x, float z, float yaw)
{
    if (n < 1 || n >= MAX_TANKS)
        return;
    TankData &tk = tanks[n];
    if (tk.type == 0)
        return;

    tk.x = x;
    tk.z = z;
    tk.yaw = yaw;
    tk.y = terrain ? terrain->getHeight(x, z) : 0.0f;
    tk.bounce = 0;
    tk.onGround = true;
}

void TankSystem::applyBounce(int n)
{
    if (n < 1 || n >= MAX_TANKS)
        return;
    TankData &tk = tanks[n];

    if (tk.bounceForce > 0.0f)
    {
        tk.x = newXValue(tk.x, tk.bounceAngle, tk.bounceForce);
        tk.z = newZValue(tk.z, tk.bounceAngle, tk.bounceForce);
        tk.bounceForce /= 1.05f;
        if (tk.bounceForce < 0.3f)
            tk.bounceForce = -1.0f; // DBP: -1 = сброс
    }
}

void TankSystem::updateTank(int n, float xj, float yj, float deltaTime)
{
    if (n < 1 || n >= MAX_TANKS)
        return;

    TankData &tk = tanks[n];

    if (tk.type == 0)
        return;
    if (!terrain)
        return;

    // === Сохраняем предыдущее состояние ДЛЯ ИНТЕРПОЛЯЦИИ ===
    tk.prevX = tk.x;
    tk.prevY = tk.y;
    tk.prevZ = tk.z;
    tk.prevPitch = tk.pitch;
    tk.prevYaw = tk.yaw;
    tk.prevRoll = tk.roll;

    // ================================================================
    // 1. ПОВОРОТ (DBP: turning)
    // ================================================================
    float lim = tk.steering;
    if (tk.damaged)
        lim *= 0.8f;

    if (xj != 0.0f)
    {
        tk.spin += xj * 0.02f;
        if (tk.spin > lim)
            tk.spin = lim;
        if (tk.spin < -lim)
            tk.spin = -lim;
    }

    if (xj == 0.0f && fabsf(tk.spin) >= 0.01f)
    {
        tk.spin /= 1.05f;
        if (fabsf(tk.spin) <= 0.05f)
            tk.spin = 0.0f;
    }

    tk.yaw = wrapValue(tk.yaw + tk.spin);

    // ================================================================
    // 2. УСКОРЕНИЕ (DBP: accelleration, only if on ground)
    // ================================================================
    if (tk.onGround)
    {
        float lspd = tk.maxSpeed * (20.0f + (float)tk.aiType) / 20.0f;
        if (tk.damaged)
            lspd *= 0.75f;

        if (yj != 0.0f)
        {
            tk.accel -= yj * 0.028f;
            if (tk.accel < -(lspd / 4.3f))
                tk.accel = -(lspd / 4.3f);
            if (tk.accel > lspd)
                tk.accel = lspd;
        }
    }

    // Инерционная остановка
    if ((yj == 0.0f && fabsf(tk.accel) >= 0.05f) || !tk.onGround)
    {
        tk.accel /= 1.035f;
        if (fabsf(tk.accel) < 0.05f)
            tk.accel = 0.0f;
    }

    // ================================================================
    // 3. НАКЛОН (DBP: inclination engine)
    // ================================================================
    float hg = terrain->getHeight(tk.x, tk.z);

    // Pitch
    float xax = newXValue(tk.x, tk.yaw, 10.0f);
    float zax = newZValue(tk.z, tk.yaw, 10.0f);
    float hx = terrain->getHeight(xax, zax) - hg;
    float ax = atanDeg(hx / 10.0f);
    tk.pitch += (ax - tk.pitch) / 20.0f;

    // Roll
    float xaz = newXValue(tk.x, tk.yaw - 90.0f, 10.0f);
    float zaz = newZValue(tk.z, tk.yaw - 90.0f, 10.0f);
    float hz = terrain->getHeight(xaz, zaz) - hg;
    float az = atanDeg(hz / 10.0f);
    tk.roll += (az - tk.roll) / 20.0f;

    // ================================================================
    // 4. ГРАВИТАЦИЯ / ХОДЬБА (DBP: gravity / walk)
    // ================================================================
    float v = tk.pitch;
    if (tk.accel < 0.0f)
        v = -v;
    float k = 1.0f - v / 75.0f;
    if (tk.turboCounter > 0)
        k *= 2.0f;
    tk.fallForce += (k - tk.fallForce) / 10.0f;
    float f = tk.fallForce * tk.accel;

    if (tk.turboCounter > 0)
        tk.turboCounter--;

    // Эффект ходьбы
    if (tk.walkCounter > 0)
    {
        tk.walkSine = wrapValue(tk.walkSine + f * 3.75f);
        tk.walkCounter--;
    }
    if (tk.walkCounter <= 0 && rnd(1000) > 970)
    {
        tk.walkCounter = rnd(150) + 150;
    }

    // ================================================================
    // 5. ПЕРЕМЕЩЕНИЕ (DBP: moving tank on the map)
    // ================================================================
    tk.x = newXValue(tk.x, tk.yaw, f);
    tk.z = newZValue(tk.z, tk.yaw, f);
    tk.rpm = f;

    // Лимиты карты
    if (tk.x < TANK_LIMIT_MIN)
    {
        tk.x = TANK_LIMIT_MIN;
        tk.accel /= 1.015f;
        tk.aiState     = 0;
        tk.escapeAngle = 0.0f;
        tk.target      = 51;
    }
    if (tk.z < TANK_LIMIT_MIN)
    {
        tk.z = TANK_LIMIT_MIN;
        tk.accel /= 1.015f;
        tk.aiState     = 0;
        tk.escapeAngle = 0.0f;
        tk.target      = 51;
    }
    if (tk.x > TANK_LIMIT_MAX)
    {
        tk.x = TANK_LIMIT_MAX;
        tk.accel /= 1.015f;
        tk.aiState     = 0;
        tk.escapeAngle = 0.0f;
        tk.target      = 51;
    }
    if (tk.z > TANK_LIMIT_MAX)
    {
        tk.z = TANK_LIMIT_MAX;
        tk.accel /= 1.015f;
        tk.aiState     = 0;
        tk.escapeAngle = 0.0f;
        tk.target      = 51;
    }

    // ================================================================
    // 6. ВЫСОТА / ЗЕМЛЯ (DBP: height control and gravity)
    // ================================================================
    hg = terrain->getHeight(tk.x, tk.z);

    tk.bounce -= 0.115f;
    if (tk.bounce < -8.0f)
        tk.bounce = -8.0f;

    tk.y += tk.bounce;
    tk.onGround = false;

    if (tk.y <= hg)
    {
        tk.y = hg;
        tk.onGround = true;
        // DBP не сбрасывает bounce явно, но при onGround
        // танк больше не падает. Оставляем как в оригинале.
    }

    // ================================================================
    // 7. BOUNCE (DBP: bounce tank vs tree / vs tank collision routine)
    //    if tk#(n,15)>0
    //      tk#(n,1)=newxvalue(tk#(n,1),tk#(n,14),tk#(n,15))
    //      tk#(n,3)=newzvalue(tk#(n,3),tk#(n,14),tk#(n,15))
    //      tk#(n,15)=tk#(n,15)/1.05
    //      if tk#(n,15)<0.3 then tk#(n,15)=-1
    // ================================================================
    applyBounce(n);
}

#include "TreeSystem.h" // в начало TankSystem.cpp

void TankSystem::updateCollisions()
{
    if (!terrain)
        return;

    // ================================================================
    // A. TANK ↔ TREE  (DBP: ter(0,xm,zm)=1 check)
    // ================================================================
    for (int n = 1; n < MAX_TANKS; n++)
    {
        TankData &tk = tanks[n];
        if (tk.type == 0)
            continue;

        int xm = (int)(tk.x / 100.0f);
        int zm = (int)(tk.z / 100.0f);

        if (xm < 0 || xm >= HEIGHTMAP_SIZE ||
            zm < 0 || zm >= HEIGHTMAP_SIZE)
            continue;

        if (terrain->getCell(xm, zm).objectType != 1)
            continue;

        float cex = xm * 100.0f + 50.0f;
        float cez = zm * 100.0f + 50.0f;

        // DBP: if tk#(n,1)<cex+tk#(n,33) and ...
        if (tk.x >= cex - tk.collisionRange &&
            tk.x <= cex + tk.collisionRange &&
            tk.z >= cez - tk.collisionRange &&
            tk.z <= cez + tk.collisionRange)
        {
            // Угол от дерева к танку (DBP: point object 65000 → angle y)
            float angle = atan2f(tk.x - cex, tk.z - cez) * RAD2DEG;
            if (angle < 0.0f)
                angle += 360.0f;

            // DBP: tk#(n,14) = angle - 10 + rnd(20)  — irregular bounce
            tk.bounceAngle = angle - 10.0f + (float)rnd(20);
            // DBP: tk#(n,15) = 0.8 + abs(f#)*1.5
            tk.bounceForce = 0.8f + fabsf(tk.rpm) * 1.5f;

            // DBP: tk#(n,22) = abs(tk#(n,10))/1.425 : tk#(n,23)=0
            tk.bounce = fabsf(tk.accel) / 1.425f;
            tk.onGround = false;
            // DBP: tk#(n,24)=rnd(20)-10 : tk#(n,25)=rnd(20)-10
            tk.bounceRoll = (float)(rnd(20) - 10);
            tk.bouncePitch = (float)(rnd(20) - 10);

            // Урон дереву
            if (treeSystem)
                treeSystem->hitTree(xm, zm, angle);
        }
    }

    // ================================================================
    // B. TANK ↔ TANK  (DBP: for c=n+1 to obmax)
    // ================================================================
    for (int n = 1; n < MAX_TANKS - 1; n++)
    {
        TankData &tkN = tanks[n];
        if (tkN.type == 0)
            continue;

        for (int c = n + 1; c < MAX_TANKS; c++)
        {
            TankData &tkC = tanks[c];
            if (tkC.type == 0)
                continue;

            float dx = tkN.x - tkC.x;
            float dz = tkN.z - tkC.z;
            float dy = tkN.y - tkC.y;
            float r = sqrtf(dx * dx + dz * dz);
            float rcol = tkN.collisionRange + tkC.collisionRange;

            // DBP: if r<rcol and abs(dy#)<rcol
            if (r < rcol && fabsf(dy) < rcol)
            {
                // Угол от N к C
                // DBP: point object 65000, tk#(c,1), 0, tk#(c,3)
                float angleNtoC = atan2f(tkC.x - tkN.x,
                                         tkC.z - tkN.z) *
                                  RAD2DEG;
                if (angleNtoC < 0.0f)
                    angleNtoC += 360.0f;

                // DBP: fce# = (abs(tk#(n,10))+abs(tk#(c,10)))/4 + 2.5
                float fce = (fabsf(tkN.accel) + fabsf(tkC.accel)) / 4.0f + 2.5f;

                // DBP: tk#(n,14) = wrapvalue(angle - 180)
                tkN.bounceAngle = wrapValue(angleNtoC + 180.0f);
                tkN.bounceForce = fce;
                tkN.accel /= 2.0f;

                // DBP: tk#(c,14) = angle
                tkC.bounceAngle = angleNtoC;
                tkC.bounceForce = fce;
                tkC.accel /= 2.0f;

                // DBP: если wreck → половина силы
                if (tkC.type < 0)
                    tkC.bounceForce /= 2.0f;
                if (tkN.type < 0)
                    tkN.bounceForce /= 2.0f;
            }
        }
    }
}

void TankSystem::interpolate(float alpha)
{
    for (int n = 1; n < MAX_TANKS; n++)
    {
        TankData &tk = tanks[n];
        if (tk.type == 0)
            continue;

        tk.interpX = tk.prevX + (tk.x - tk.prevX) * alpha;
        tk.interpY = tk.prevY + (tk.y - tk.prevY) * alpha;
        tk.interpZ = tk.prevZ + (tk.z - tk.prevZ) * alpha;

        // Для углов нужна осторожность с wraparound (359° → 0°)
        // Используем кратчайшую дугу
        float dYaw = tk.yaw - tk.prevYaw;
        if (dYaw > 180.0f)
            dYaw -= 360.0f;
        if (dYaw < -180.0f)
            dYaw += 360.0f;
        tk.interpYaw = wrapValue(tk.prevYaw + dYaw * alpha);

        tk.interpPitch = tk.prevPitch + (tk.pitch - tk.prevPitch) * alpha;
        tk.interpRoll = tk.prevRoll + (tk.roll - tk.prevRoll) * alpha;
    }
}

void TankSystem::render() const
{
    for (int n = 1; n < MAX_TANKS; n++)
    {
        const TankData &tk = tanks[n];
        if (tk.type == 0)
            continue;
        if (!modelsLoaded[tk.baseType])
            continue;

        // Выбираем текстуру по состоянию
        Texture2D tex = {0};
        int sq = tk.squadId;
        if (sq >= 1 && sq <= 10 && squadTexLoaded[sq])
        {
            if (tk.type < 0)
            {
                tex = squadTexDestroyed[sq];
            }
            else if (tk.damaged)
            {
                tex = squadTexDamaged[sq];
            }
            else
            {
                tex = squadTexNormal[sq];
            }
        }

        // Подменяем текстуру
        Model &mdl = const_cast<Model &>(tankModels[tk.baseType]);
        if (tex.id != 0)
        {
            for (int j = 0; j < mdl.materialCount; j++)
            {
                mdl.materials[j].maps[MATERIAL_MAP_DIFFUSE].texture = tex;
                mdl.materials[j].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
            }
        }

        // DBP: position object n, tk#(n,1), tk#(n,2)+3.3, tk#(n,3)
        float posX = tk.interpX;
        float posY = tk.interpY + 3.3f;
        float posZ = tk.interpZ;

        // DBP: walk effect
        float walkPitch = sinDeg(tk.walkSine) * 2.0f;
        float spinRoll = tk.spin * 3.0f;

        // DBP: bounce effects
        float hg = terrain ? terrain->getHeight(posX, posZ) : 0.0f;
        float rbounce = (hg - posY) * tk.bounceRoll / 1.5f;
        float hbounce = (hg - posY) * tk.bouncePitch;

        float finalPitch = tk.interpPitch + walkPitch - hbounce / 10.0f;
        float finalRoll = tk.interpRoll + rbounce / 10.0f + spinRoll;
        float finalYaw = tk.interpYaw;

        rlPushMatrix();

        rlTranslatef(posX, posY, posZ);

        // Модель .glb: нос вдоль −Z → добавляем 180°
        rlRotatef(finalYaw + 180.0, 0.0f, 1.0f, 0.0f);
        // Pitch: инвертирован для OpenGL, но 180° yaw компенсирует → без инверсии
        rlRotatef(finalPitch, 1.0f, 0.0f, 0.0f);
        // Roll: аналогично
        rlRotatef(finalRoll, 0.0f, 0.0f, 1.0f);
        rlScalef(tk.scaleX, tk.scaleY, tk.scaleZ);

        // DrawModel(mdl, {0.0f, 0.0f, 0.0f}, 1.0f, WHITE);

        // DBP: hide limb n, fireLimb — меш дула невидим
        for (int i = 0; i < mdl.meshCount; i++)
        {
            if (i == mdl.meshCount - tk.fireLimb)
                continue; // пропускаем меш дула

            int matIdx = mdl.meshMaterial[i];
            DrawMesh(mdl.meshes[i], mdl.materials[matIdx], MatrixIdentity());
        }

        rlPopMatrix();
    }
}

int TankSystem::getActiveCount() const
{
    int count = 0;
    for (int n = 1; n < MAX_TANKS; n++)
    {
        if (tanks[n].type != 0)
            count++;
    }
    return count;
}

void TankSystem::unloadSquadTextures()
{
    for (int i = 1; i <= 10; i++)
    {
        if (squadTexLoaded[i])
        {
            if (squadTexNormal[i].id != 0)
                UnloadTexture(squadTexNormal[i]);
            if (squadTexDamaged[i].id != 0)
                UnloadTexture(squadTexDamaged[i]);
            if (squadTexDestroyed[i].id != 0)
                UnloadTexture(squadTexDestroyed[i]);
            squadTexNormal[i] = {0};
            squadTexDamaged[i] = {0};
            squadTexDestroyed[i] = {0};
            squadTexLoaded[i] = false;
        }
    }
}

void TankSystem::loadSquadTextures()
{
    unloadSquadTextures();

    for (int i = 1; i <= 10; i++)
    {
        char path[128];

        // Обычная текстура: data/tanks/t1.png ... t10.png
        snprintf(path, sizeof(path), "data/tanks/t%d.png", i);
        squadTexNormal[i] = LoadTexture(path);

        // Повреждённая: data/tanks/td1.png ... td10.png
        snprintf(path, sizeof(path), "data/tanks/td%d.png", i);
        squadTexDamaged[i] = LoadTexture(path);

        // Уничтоженная: data/tanks/tw1.png ... tw10.png
        snprintf(path, sizeof(path), "data/tanks/tw%d.png", i);
        squadTexDestroyed[i] = LoadTexture(path);

        if (squadTexNormal[i].id != 0)
        {
            squadTexLoaded[i] = true;
            // DBP: set object texture n,0,0 — отключаем mipmapping
            SetTextureFilter(squadTexNormal[i], TEXTURE_FILTER_POINT);
            SetTextureFilter(squadTexDamaged[i], TEXTURE_FILTER_POINT);
            SetTextureFilter(squadTexDestroyed[i], TEXTURE_FILTER_POINT);
            TraceLog(LOG_INFO, "Squad textures %d loaded (normal/damaged/destroyed)", i);
        }
        else
        {
            TraceLog(LOG_WARNING, "Squad texture %d NOT found", i);
        }
    }
}