#include "TankSystem.h"
#include "Utils.h"
#include <cstdio>
#include <cmath>
#include <cfloat>
#include <cstring>
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
    unloadAllExtras();
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
    t.superBulletCounter = 0;
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

    if (superBulletPUPModelLoaded && superBulletPUPModel.meshCount > 0)
    {
        UnloadModel(superBulletPUPModel);
        superBulletPUPModelLoaded = false;
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

    Mesh ringMesh = GenMeshTorus(0.125f, 1.0f, 8, 32);

    if (ringMesh.vertexCount > 0)
    {
        superBulletPUPModel = LoadModelFromMesh(ringMesh);
        superBulletPUPModelLoaded = (superBulletPUPModel.meshCount > 0);

        if (superBulletPUPModelLoaded)
        {
            // Материал: белый (цвет задаётся через tint при DrawModelEx)
            if (superBulletPUPModel.materialCount > 0)
                superBulletPUPModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;

            TraceLog(LOG_INFO, "Ring model (torus) created: %d vertices",
                     ringMesh.vertexCount);
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

// ============================================================
// Загрузка extra-объекта в слот n (46..50)
// DBP: tankloader() cases 10..13
// ============================================================
void TankSystem::loadExtra(int n, int type)
{
    if (n < EXTRA_MIN || n > EXTRA_MAX)
        return;
    int slot = n - EXTRA_MIN;

    // выбор файлов по типу
    const char *modelFile = nullptr;
    const char *texFile = nullptr;
    switch (type)
    {
    case 10:
        modelFile = "data/extra/cow.glb";
        texFile = "data/extra/cow.png";
        break;
    case 11:
        modelFile = "data/extra/camel.glb";
        texFile = "data/extra/camel.png";
        break;
    case 12:
        modelFile = "data/extra/moose.glb";
        texFile = "data/extra/moose.png";
        break;
    case 13:
        modelFile = "data/extra/alien.glb";
        texFile = "data/extra/alien.png";
        break;
    default:
        TraceLog(LOG_WARNING, "loadExtra: unknown type %d", type);
        return;
    }

    if (!FileExists(modelFile))
    {
        TraceLog(LOG_ERROR, "loadExtra: model not found: %s", modelFile);
        return;
    }

    // освободить слот если занят
    unloadExtraSlot(slot);

    ExtraModelSlot &es = extraSlots[slot];

    // --- модель ---
    es.model = LoadModel(modelFile);

    // === ДИАГНОСТИКА ===
    TraceLog(LOG_INFO, "Model: bones=%d, meshes=%d", es.model.boneCount, es.model.meshCount);
    for (int i = 0; i < es.model.meshCount; i++)
    {
        Mesh &m = es.model.meshes[i];
        TraceLog(LOG_INFO, "  Mesh %d: verts=%d, boneIds=%s, boneWeights=%s",
                 i, m.vertexCount,
                 m.boneIds ? "YES" : "NULL",
                 m.boneWeights ? "YES" : "NULL");
    }

    // --- явная загрузка текстуры и применение к материалам ---
    if (FileExists(texFile))
    {
        es.texture = LoadTexture(texFile);
        for (int j = 0; j < es.model.materialCount; j++)
        {
            es.model.materials[j].maps[MATERIAL_MAP_DIFFUSE].texture = es.texture;
            es.model.materials[j].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
        }
        TraceLog(LOG_INFO, "Extra %d: texture %s loaded", n, texFile);
    }
    else
    {
        TraceLog(LOG_WARNING, "loadExtra: texture not found: %s", texFile);
    }

    // --- скелетная анимация ---
    es.anims = LoadModelAnimations(modelFile, &es.animCount);
    if (es.animCount > 0)
    {
        TraceLog(LOG_INFO, "Extra %d: %d animations, %d frames, %d bones",
                 n, es.animCount, es.anims[0].frameCount, es.anims[0].boneCount);
    }
    else
    {
        TraceLog(LOG_WARNING, "loadExtra: no animation in %s", modelFile);
    }

    es.loaded = true;

    // ============================================================
    // Параметры TankData — точно по DBP tankloader cases 10-13
    // ============================================================
    TankData &tk = tanks[n];
    tk.type = type;
    tk.baseType = type;
    tk.squadId = 0;        // extra без squad
    tk.y = -1000.0f;       // DBP: tk#(n,2)=-1000 (падает на землю)
    tk.animFrame = 1.0f;   // DBP: tk#(n,16)=1
    tk.aimRatio = 20;      // DBP: tk#(n,32)=20
    tk.bulletCounter = 75; // DBP: tk#(n,20)=75
    tk.canFire = 0;        // DBP: tk#(n,43)=0

    switch (type)
    {
    case 10: // cow
        tk.steering = 0.5f;
        tk.maxSpeed = 0.25f;
        tk.animSpeed = 0.4f;
        tk.collisionRange = 17.0f;
        tk.energy = 1.0f;
        tk.collisionHeight = 13.0f;
        tk.scaleX = tk.scaleY = tk.scaleZ = 2.2f; // 220%
        break;
    case 11: // camel
        tk.steering = 0.4f;
        tk.maxSpeed = 0.22f;
        tk.animSpeed = 0.37f;
        tk.collisionRange = 17.0f;
        tk.energy = 1.5f;
        tk.collisionHeight = 14.0f;
        tk.scaleX = tk.scaleY = tk.scaleZ = 2.2f; // 220%
        break;
    case 12: // moose (sheep)
        tk.steering = 0.4f;
        tk.maxSpeed = 0.22f;
        tk.animSpeed = 0.37f;
        tk.collisionRange = 17.0f;
        tk.energy = 1.5f;
        tk.collisionHeight = 14.0f;
        tk.scaleX = 2.25f;
        tk.scaleY = 2.30f;
        tk.scaleZ = 2.25f; // 225,230,225
        break;
    case 13: // alien
        tk.steering = 0.2f;
        tk.maxSpeed = 0.20f;
        tk.animSpeed = 0.37f;
        tk.collisionRange = 12.0f;
        tk.energy = 1.5f;
        tk.collisionHeight = 12.0f;
        tk.scaleX = tk.scaleY = tk.scaleZ = 2.25f; // 225%
        break;
    }

    tk.originalEnergy = tk.energy;
}

void TankSystem::unloadExtraSlot(int slot)
{
    if (slot < 0 || slot > (EXTRA_MAX - EXTRA_MIN))
        return;
    ExtraModelSlot &es = extraSlots[slot];
    if (!es.loaded)
        return;

    UnloadModel(es.model);
    if (es.texture.id != 0)
        UnloadTexture(es.texture);
    if (es.anims)
    {
        UnloadModelAnimations(es.anims, es.animCount);
        es.anims = nullptr;
    }
    es.model = {};
    es.texture = {};
    es.animCount = 0;
    es.loaded = false;
}

void TankSystem::unloadAllExtras()
{
    for (int i = 0; i <= (EXTRA_MAX - EXTRA_MIN); i++)
        unloadExtraSlot(i);
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

// ============================================================
// Спавн extra по биому — DBP maketerrain()
// ============================================================
void TankSystem::spawnExtrasForBiome(int biome)
{
    int type;
    switch (biome)
    {
    case 1:
    case 2:
        type = 10;
        break; // grass, mountains > коровы
    case 3:
        type = 11;
        break; // desert > верблюды
    case 4:
        type = 12;
        break; // frozed > лоси
    case 5:
        type = 10;
        break; // tundra > коровы
    case 6:
        type = 13;
        break; // moon > инопланетяне
    default:
        return;
    }

    // DBP: nc=1+rnd(3): for c=46 to 46+nc
    int nc = 1 + rnd(3);
    for (int c = EXTRA_MIN; c <= EXTRA_MIN + nc; c++)
    {
        if (c > EXTRA_MAX)
            break;
        loadExtra(c, type);

        TankData &tk = tanks[c];
        // DBP: tk#(c,1)=rnd(3000)+1000 : tk#(c,3)=rnd(3000)+1000 : tk#(c,5)=rnd(359)
        tk.x = 1000.0f + (float)rnd(3000);
        tk.z = 1000.0f + (float)rnd(3000);
        tk.yaw = (float)rnd(359);
        // y остаётся -1000 > упадёт на землю через гравитацию в updateTank()

        TraceLog(LOG_INFO, "Extra %d spawned: type=%d at (%.0f, %.0f)", c, type, tk.x, tk.z);
    }
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
    if (n < 1 || n > COMBAT_MAX)
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

    // === DBP: Turbo setting ===
    // if tk#(n,45)>0 then tk#(n,45)=tk#(n,45)-1
    if (tk.turboCharger > 0)
        tk.turboCharger--;

    // DBP: if tk#(n,44)>0 then tk#(n,44)=tk#(n,44)-1
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
        tk.aiState = 0;
        tk.escapeAngle = 0.0f;
        tk.target = 51;
    }
    if (tk.z < TANK_LIMIT_MIN)
    {
        tk.z = TANK_LIMIT_MIN;
        tk.accel /= 1.015f;
        tk.aiState = 0;
        tk.escapeAngle = 0.0f;
        tk.target = 51;
    }
    if (tk.x > TANK_LIMIT_MAX)
    {
        tk.x = TANK_LIMIT_MAX;
        tk.accel /= 1.015f;
        tk.aiState = 0;
        tk.escapeAngle = 0.0f;
        tk.target = 51;
    }
    if (tk.z > TANK_LIMIT_MAX)
    {
        tk.z = TANK_LIMIT_MAX;
        tk.accel /= 1.015f;
        tk.aiState = 0;
        tk.escapeAngle = 0.0f;
        tk.target = 51;
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

    // ============================================================
    // Анимация extra — DBP:
    //   if n>45 and tk#(n,0)>0
    //     tk#(n,16)=tk#(n,16)+tk#(n,19)
    //     if tk#(n,16)>50 then tk#(n,16)=tk#(n,16)-50
    //     set object frame n,tk#(n,16)
    // ============================================================
    if (n >= EXTRA_MIN && n <= EXTRA_MAX && tk.type > 0)
        updateExtraAnimation(n);
}

void TankSystem::updateExtraAnimation(int n)
{
    if (n < EXTRA_MIN || n > EXTRA_MAX)
        return;

    int slot = n - EXTRA_MIN;
    ExtraModelSlot &es = extraSlots[slot];
    if (!es.loaded || es.animCount == 0)
        return;

    TankData &tk = tanks[n];

    if (tk.type <= 0)
        return;

    ModelAnimation &anim = es.anims[0];
    if (anim.frameCount <= 0)
        return;

    tk.animFrame += tk.animSpeed;
    float fc = (float)anim.frameCount;
    while (tk.animFrame >= fc)
        tk.animFrame -= fc;
    if (tk.animFrame < 0)
        tk.animFrame = 0;

    int frame = (int)tk.animFrame;
    if (frame >= anim.frameCount)
        frame = anim.frameCount - 1;

    if (frame != tk.lastAnimFrame)
    {
        UpdateModelAnimation(es.model, anim, frame);
        tk.lastAnimFrame = frame;
    }
}

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
        if (n >= EXTRA_MIN && n <= EXTRA_MAX)
        {
            renderExtra(n);
            continue;
        }

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

        // ============================================
        // ТЕНЬ — используем ТЕ ЖЕ интерполированные координаты
        // DBP: position object 200+n, tk#(n,1), hg#+3.5, tk#(n,3)
        // turn/pitch/roll такие же как у танка
        // ============================================
        float shadowY = hg + 3.5f;

        // Уменьшение тени с высотой (для летающих танков)
        float height = tk.interpY - hg;
        float shScale = 1.0f;
        if (height > 1.0f)
        {
            shScale = 1.0f / (1.0f + height / 200.0f);
            if (shScale < 0.2f)
                shScale = 0.2f;
        }
        float shadowRadius = tk.collisionRange * shScale * 1.25f;
        if (shadowRadius < 10.0f)
            shadowRadius = 10.0f;

        //rlDisableDepthMask();
        //rlDisableDepthTest();
        BeginBlendMode(BLEND_MULTIPLIED);
        rlPushMatrix();
        rlTranslatef(tk.interpX, shadowY, tk.interpZ);
        rlRotatef(finalYaw + 180.0f, 0.0f, 1.0f, 0.0f);
        rlRotatef(tk.interpPitch, 1.0f, 0.0f, 0.0f);
        rlRotatef(tk.interpRoll, 0.0f, 0.0f, 1.0f);
        DrawCylinder({0, 0, 0}, shadowRadius, shadowRadius, 0.1f, 24, {200, 200, 200, 255});

        // ============================================
        // СУПЕРПУЛЯ — красное кольцо
        // DBP: show limb 200+n,1 if tk#(n,51)>0
        // ============================================
        if (tk.superBulletCounter > 0 && superBulletPUPModelLoaded)
        {
            //BeginBlendMode(BLEND_ADDITIVE);
            // Внешний радиус кольца (чуть больше тени)
            float outerR = shadowRadius * 2.6f;
            DrawModelEx(superBulletPUPModel, {0, 0, 0}, {1, 0, 0}, -90.0f, {outerR, outerR, 0.1f}, {220, 20, 20, 255});
        }

        rlPopMatrix();
        EndBlendMode();
        //rlEnableDepthTest();
        //rlEnableDepthMask();

        // танк
        rlPushMatrix();
        rlTranslatef(posX, posY, posZ);
        rlRotatef(finalYaw + 180.0, 0.0f, 1.0f, 0.0f);
        rlRotatef(finalPitch, 1.0f, 0.0f, 0.0f);
        rlRotatef(finalRoll, 0.0f, 0.0f, 1.0f);
        rlScalef(tk.scaleX, tk.scaleY, tk.scaleZ);

        for (int i = 0; i < mdl.meshCount; i++)
        {
            // DBP: hide limb n, fireLimb — меш дула невидим
            if (i == mdl.meshCount - tk.fireLimb)
                continue;

            int matIdx = mdl.meshMaterial[i];
            DrawMesh(mdl.meshes[i], mdl.materials[matIdx], MatrixIdentity());
        }

        rlPopMatrix();
    }
}

// ============================================
// BARRIER — полупрозрачная сфера вокруг танка
// DBP: show limb 200+n,2 if tk#(n,50)>0
// Сфера симметрична, рисуется в мировых координатах
// (pitch/roll тени не применяются — сфера вокруг корпуса танка)
// ============================================
void TankSystem::renderShields() const
{
    for (int n = 1; n <= TANKS_MAX; n++)
    {
        const TankData &tk = tanks[n];
        if (tk.type == 0)
            continue;

        if (tk.barrierCounter > 0)
        {
            // Радиус: чуть больше collisionRange танка
            float sphereR = tk.collisionRange * 2.0f;
            if (sphereR < 20.0f)
                sphereR = 20.0f;

            // Центр сферы — центр корпуса танка
            Vector3 center = {
                tk.interpX,
                tk.interpY + tk.collisionHeight * 0.7f,
                tk.interpZ};

            BeginBlendMode(BLEND_ALPHA);
            rlDisableDepthMask();
            DrawSphere(center, sphereR, {100, 200, 255, 60});
            DrawSphereWires(center, sphereR + 0.125f, 12, 12, {150, 220, 255, 150});
            rlEnableDepthMask();
            EndBlendMode();
        }
    }
}

// ============================================================
// Рендер extra: тень + модель с анимацией + переворот мёртвых
// Вызывать в main.cpp ПОСЛЕ tankSystem.render()
// ============================================================
void TankSystem::renderExtra(int n) const
{
    if (n < EXTRA_MIN || n > EXTRA_MAX)
        return;

    const TankData &tk = tanks[n];
    if (tk.type == 0)
        return;

    int slot = n - EXTRA_MIN;
    const ExtraModelSlot &es = extraSlots[slot];
    if (!es.loaded)
        return;

    float groundH = terrain ? terrain->getHeight(tk.interpX, tk.interpZ) : tk.interpY;

    // DBP: flipcow#=0:slop#=0
    //      if n>45 and tk#(n,0)<0 then flipcow#=180:slop#=20
    bool dead = (tk.type < 0);
    float flipRoll = dead ? 180.0f : 0.0f;
    float slop = dead ? 24.0f : 0.0f;

    if (!dead)
    {
        // DBP: position object 200+n, tk#(n,1), hg#+3.5-slop#*10, tk#(n,3)
        float shadowRadius = tk.collisionRange * 1.0f;
        if (shadowRadius < 10.0f)
            shadowRadius = 10.0f;

        //rlDisableDepthMask();
        BeginBlendMode(BLEND_MULTIPLIED);
        rlPushMatrix();
        rlTranslatef(tk.interpX, groundH + 1.0f, tk.interpZ);
        rlRotatef(tk.interpYaw + 180.0f, 0.0f, 1.0f, 0.0f);
        rlRotatef(tk.interpPitch, 1.0f, 0.0f, 0.0f);
        rlRotatef(tk.interpRoll, 0.0f, 0.0f, 1.0f);
        DrawCylinder({0, 0, 0}, shadowRadius, shadowRadius, 0.1f, 24, {200, 200, 200, 255});
        rlPopMatrix();
        EndBlendMode();
        //rlEnableDepthMask();
    }

    // === МОДЕЛЬ EXTRA ===
    // DBP: position object n, tk#(n,1), slop#+tk#(n,2)+3.3, tk#(n,3)
    rlPushMatrix();
    rlTranslatef(tk.interpX, tk.interpY + 1.0f + slop, tk.interpZ);
    rlRotatef(tk.interpYaw + 180.0f, 0.0f, 1.0f, 0.0f);
    rlRotatef(tk.interpPitch, 1.0f, 0.0f, 0.0f);
    rlRotatef(tk.interpRoll + flipRoll, 0.0f, 0.0f, 1.0f);
    rlScalef(tk.scaleX, tk.scaleY, tk.scaleZ);
    DrawModel(es.model, {0, 0, 0}, 1.0f, WHITE);
    rlPopMatrix();
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