#include "PowerUpSystem.h"
#include "Utils.h"
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include <cmath>

// ============================================================
// Инициализация — DBP makesortie: создание 5 powerup
// ============================================================
void PowerUpSystem::init(AudioSystem *audioSystem, Terrain *terrain, TankSystem *tankSystem)
{
    this->audioSystem = audioSystem;
    this->terrain = terrain;
    this->tankSystem = tankSystem;

    reset();
}
void PowerUpSystem::reset()
{
    for (int i = 0; i < NUM_PUP; i++)
    {
        pups[i] = PowerUpData{};
    }
}

// ============================================================
// Загрузка моделей
// DBP: load object "object/pup1.x" / "pup2.x" / "pup3.x"
// ============================================================
void PowerUpSystem::loadAssets()
{
    const char *modelFiles[3] = {
        "data/objects/pup1.glb", // barrier
        "data/objects/pup2.glb", // superbullet
        "data/objects/pup3.glb"  // repair
    };

    const char *textureFiles[3] = {
        "data/objects/pup1.png", // barrier
        "data/objects/pup2.png", // superbullet
        "data/objects/pup3.png"  // repair
    };

    for (int i = 0; i < 3; i++)
    {
        Model m = LoadModel(modelFiles[i]);
        if (m.meshCount > 0)
        {
            models[i] = m;
            modelLoaded[i] = true;
        }
        else
        {
            TraceLog(LOG_WARNING, "PUP model %d NOT found: %s", i, modelFiles[i]);
            continue;
        }

        Texture2D tex = LoadTexture(textureFiles[i]);
        if (tex.id != 0)
        {
            textures[i] = tex;
            textureLoaded[i] = true;
            SetTextureFilter(tex, TEXTURE_FILTER_POINT);

            for (int m = 0; m < models[i].materialCount; m++)
            {
                Material *mat = &models[i].materials[m];
                mat->maps[MATERIAL_MAP_DIFFUSE].texture = tex;
                mat->maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
            }
        }
        else
        {
            TraceLog(LOG_WARNING, "PUP texture %d NOT found: %s", i, textureFiles[i]);
        }
    }
}

void PowerUpSystem::respawn()
{
    // DBP makesortie:
    //   51,52 = barrier:     tk#(o,4)=1550+rnd(200)
    //   53    = repair:      tk#(o,4)=4500+rnd(200)
    //   54,55 = superbullet: tk#(o,4)=1550+rnd(200)

    // Типы
    pups[0].type = 0; // 51 barrier
    pups[1].type = 0; // 52 barrier
    pups[2].type = 2; // 53 repair
    pups[3].type = 1; // 54 superbullet
    pups[4].type = 1; // 55 superbullet

    // DBP makesortie: сначала powup(o) — позиция + таймер 2500+rnd(2500)
    for (int i = 0; i < NUM_PUP; i++)
        respawn(i);

    // DBP makesortie: ПОТОМ перезапись начального таймера
    // (в DBP: tk#(o,4)=1550+rnd(200) после powup(o))
    pups[0].spawnTimer = 1550 + rnd(200); // barrier
    pups[1].spawnTimer = 1550 + rnd(200); // barrier
    pups[2].spawnTimer = 4500 + rnd(200); // repair
    pups[3].spawnTimer = 1550 + rnd(200); // superbullet
    pups[4].spawnTimer = 1550 + rnd(200); // superbullet
}

// ============================================================
// Размещение powerup — DBP: powup(o)
// ============================================================
void PowerUpSystem::respawn(int idx)
{
    if (idx < 0 || idx >= NUM_PUP)
        return;

    if (!terrain)
        return;

    PowerUpData &p = pups[idx];

    // DBP: coord: x=500+rnd(4000):z=500+rnd(4000)
    //      xm=x/100:zm=z/100
    //      if ter(0,xm,zm)=1 then goto coord
    float x, z;
    int xm, zm;
    int attempts = 0;
    do
    {
        x = 500.0f + (float)rnd(4000);
        z = 500.0f + (float)rnd(4000);
        xm = (int)(x / 100.0f);
        zm = (int)(z / 100.0f);
        if (++attempts > 500)
        {
            TraceLog(LOG_WARNING, "PowerUpSystem::respawn(%d): too many attempts", idx);
            break;
        }
    } while (terrain->getCell(xm, zm).objectType == 1);

    // DBP: y#=get ground height(1,x,z)
    float y = terrain->getHeight(x, z);

    p.x = x;
    p.y = y;
    p.z = z;
    p.angleY = (float)rnd(359);
    p.active = false; // DBP: hide object o, tk#(o,0)=0

    // DBP: tk#(o,4)=2500+rnd(2500)
    //      if o=53 then tk#(o,4)=tk#(o,4)+3500
    p.spawnTimer = 2500 + rnd(2500);
    if (p.type == 2) // repair
        p.spawnTimer += 3500;

    TraceLog(LOG_INFO, "PowerUp %d (type=%d) at %.0f, %.0f, %.0f  timer=%d",
             idx, p.type, x, y, z, p.spawnTimer);
}

// ============================================================
// Обновление — DBP: managing powup (после цикла n=1..50)
// ============================================================
void PowerUpSystem::update()
{
    for (int i = 0; i < NUM_PUP; i++)
    {
        PowerUpData &p = pups[i];

        // DBP: yrotate object o,wrapvalue(object angle y(o)+0.65)
        if (p.active)
            p.angleY = wrapValue(p.angleY + 0.65f);

        // DBP: if tk#(o,0)=0
        //        tk#(o,4)=tk#(o,4)-1
        //        if tk#(o,4)<=0 then show object o:tk#(o,0)=1
        if (!p.active)
        {
            p.spawnTimer--;
            if (p.spawnTimer <= 0)
                p.active = true;
        }

        // === Синхронизация с TankSystem для AI ===
        // AI ищет powerup по tk#(t,0)>0 для t=51..55
        int tankIdx = POWERUP_MIN + i;
        TankData &tk = tankSystem->getTankMut(tankIdx);
        tk.type = p.active ? 1 : 0;
        tk.x = p.x;
        tk.y = p.y;
        tk.z = p.z;
        tk.yaw = p.angleY;
    }
}

// ============================================================
// Подбор — DBP: PowUpPicker (внутри цикла n=1..obmax)
// ============================================================
void PowerUpSystem::checkPickup(int n)
{
    if (!tankSystem)
        return;

    TankData &tk = tankSystem->getTankMut(n);
    if (tk.type == 0)
        return;

    // DBP: for pup=1 to 5
    for (int i = 0; i < NUM_PUP; i++)
    {
        PowerUpData &p = pups[i];
        if (!p.active)
            continue;

        // DBP: px=tk#(n,1)-tk#(50+pup,1)
        //      pz=tk#(n,3)-tk#(50+pup,3)
        //      rp=sqrt((px*px)+(pz*pz))
        float px = tk.x - p.x;
        float pz = tk.z - p.z;
        float rp = sqrtf(px * px + pz * pz);

        // DBP: if rp<35
        if (rp >= 35.0f)
            continue;

        // === PICKUP ===
        switch (p.type)
        {
        case 0: // barrier
            // DBP: if pup<=2 then so=24:tk#(n,50)=tk#(n,50)+1500
            tk.barrierCounter += 1500;
            if (audioSystem) audioSystem->playBarrierPickup({p.x, p.y, p.z});        
            break;

        case 2: // repair
            // DBP: if pup=3
            //   tk#(n,37)=tk#(n,37)+tk#(n,49)/2
            //   if tk#(n,37)>tk#(n,49) then tk#(n,37)=tk#(n,49)
            //   tk#(n,35)=0: texture reset
            //   powup(53): so=28
            tk.energy += tk.maxEnergy / 2.0f;
            if (tk.energy > tk.maxEnergy)
                tk.energy = tk.maxEnergy;
            tk.damaged = false; // DBP: tk#(n,35)=0
            if (audioSystem) audioSystem->playRepairPickup({p.x, p.y, p.z});
            break;

        case 1: // superbullet
            // DBP: if pup>3 then so=25:tk#(n,51)=tk#(n,51)+2000
            tk.superBulletCounter += 2000;
            if (audioSystem) audioSystem->playSuperBulletPickup({p.x, p.y, p.z});
            break;
        }

        // DBP: powup(50+pup) — respawn
        respawn(i);
    }

    // DBP: if tk#(n,50)>0 then tk#(n,50)=tk#(n,50)-1
    if (tk.barrierCounter > 0)
        tk.barrierCounter--;

    // DBP: if tk#(n,51)>0 then tk#(n,51)=tk#(n,51)-1
    if (tk.superBulletCounter > 0)
        tk.superBulletCounter--;
}

// ============================================================
// Рендеринг
// ============================================================
void PowerUpSystem::render() const
{
    for (int i = 0; i < NUM_PUP; i++)
    {
        const PowerUpData &p = pups[i];
        if (!p.active)
            continue;

        // DBP: pup1=barrier, pup2=superbullet, pup3=repair
        int modelIdx = p.type; // 0,1,2
        if (!modelLoaded[modelIdx])
            continue;

        BoundingBox bbox = GetMeshBoundingBox(models[modelIdx].meshes[0]);
        float modelH = bbox.max.y - bbox.min.y;
        float hover = modelH * 0.5f + 5.0f;

        rlPushMatrix();
        rlTranslatef(p.x, p.y + hover, p.z);
        rlRotatef(180.0f + p.angleY, 0.0f, 1.0f, 0.0f);  // yaw (DBP→OpenGL)
        rlRotatef(180.0f, 1.0f, 0.0f, 0.0f);              // flip по X
        DrawModel(models[modelIdx], {0,0,0}, 1.0f, WHITE);
        rlPopMatrix();
    }
}