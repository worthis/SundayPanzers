#include "BulletSystem.h"
#include "Utils.h"
#include "rlgl.h"
#include <cstdio>

// ============================================================
// Конструктор — value-initialization вместо memset
// ============================================================
BulletSystem::BulletSystem()
{
    reset();
}

BulletSystem::~BulletSystem()
{
    if (bulletModelsLoaded)
    {
        UnloadModel(bulletModel1);
        UnloadModel(bulletModel2);
        UnloadModel(bulletModel3);
    }
    if (hitModelsLoaded)
    {
        UnloadModel(hitModel1);
        UnloadModel(hitModel2);
    }
    if (explosionModelsLoaded)
    {
        UnloadModel(ringModel);
        UnloadModel(ballModel);
    }
    if (texturesLoaded)
    {
        UnloadTexture(bulletTex1);
        UnloadTexture(bulletTex2);
        UnloadTexture(bulletTex3);
        UnloadTexture(hitTex1);
        UnloadTexture(hitTex2);
        UnloadTexture(ringTex);
        UnloadTexture(exploTex);
    }
}

void BulletSystem::init(EventSystem *eventSystem)
{
    this->eventSystem = eventSystem;

    eventSystem->subscribe<TankFiredEvent>(
        [this](const TankFiredEvent &e)
        { onTankFired(e); });

    eventSystem->subscribe<BulletTerrainHitEvent>(
        [this](const BulletTerrainHitEvent &e)
        { onBulletTerrainHit(e); });

    eventSystem->subscribe<BulletTankHitEvent>(
        [this](const BulletTankHitEvent &e)
        { onBulletTankHit(e); });

    eventSystem->subscribe<TankDestroyedEvent>(
        [this](const TankDestroyedEvent &e)
        { onTankDestroyed(e); });
}

void BulletSystem::reset()
{
    for (int i = 0; i <= MAX_BULLETS; i++)
        bullets[i] = BulletData{};
    for (int i = 0; i <= MAX_HIT_EFFECTS; i++)
        hits[i] = HitEffect{};
    for (int i = 0; i < MAX_EXPLOSIONS; i++)
        explosions[i] = ExplosionData{};
}

// ============================================================
// Привязка текстуры к модели + unlit-настройка
// DBP: set object specular n,0
//      set object n,1,0,0,0,0,0,0   (unlit, no fog)
//      set object texture n,0,0      (no mipmapping)
// ============================================================
void BulletSystem::bindTexture(Model &m, const Texture2D &tex)
{
    for (int j = 0; j < m.materialCount; j++)
    {
        m.materials[j].maps[MATERIAL_MAP_DIFFUSE].texture = tex;
        m.materials[j].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
        m.materials[j].maps[MATERIAL_MAP_SPECULAR].color = BLACK;
        SetTextureFilter(tex, TEXTURE_FILTER_POINT);
    }
}

void BulletSystem::loadAssets()
{
    bulletModel1 = LoadModel("data/bullets/bullet.glb");
    bulletModel2 = LoadModel("data/bullets/bullet.glb");
    bulletModel3 = LoadModel("data/bullets/bullet.glb");
    bulletModelsLoaded = true;

    hitModel1 = LoadModel("data/bullets/hit.glb");
    hitModel2 = LoadModel("data/bullets/hit2.glb");
    hitModelsLoaded = true;

    ringModel = LoadModel("data/bullets/ring.glb");
    ballModel = LoadModel("data/bullets/ball.glb");
    explosionModelsLoaded = true;

    bulletTex1 = LoadTexture("data/bullets/bullet.png");
    bulletTex2 = LoadTexture("data/bullets/bullet2.png");
    bulletTex3 = LoadTexture("data/bullets/bullet3.png");
    hitTex1 = LoadTexture("data/bullets/hit.png");
    hitTex2 = LoadTexture("data/bullets/hit2.png");
    ringTex = LoadTextureColorKey("data/bullets/ring.png");
    exploTex = LoadTexture("data/bullets/expo.png");
    texturesLoaded = true;

    bindTexture(bulletModel1, bulletTex1);
    bindTexture(bulletModel2, bulletTex2);
    bindTexture(bulletModel3, bulletTex3);
    bindTexture(hitModel1, hitTex1);
    bindTexture(hitModel2, hitTex2);

    // DBP для ring/ball:
    //   set object emissive 401,rgb(255,255,20)
    //   set object diffuse 401,rgb(255,255,20)
    //   set object 401,1,1,0,0,1,0,0  (ghost)
    // Текстура ring.png привязывается, но тонирована жёлтым
    auto setupExplosionMaterial = [](Model &m)
    {
        for (int j = 0; j < m.materialCount; j++)
        {
            m.materials[j].maps[MATERIAL_MAP_DIFFUSE].color = Color{255, 255, 20, 255};
            m.materials[j].maps[MATERIAL_MAP_SPECULAR].color = BLACK;
        }
    };

    bindTexture(ringModel, ringTex);
    setupExplosionMaterial(ringModel);

    bindTexture(ballModel, exploTex);
    setupExplosionMaterial(ballModel);
}

void BulletSystem::update()
{
    // === Движение пуль ===
    for (int n = 1; n <= MAX_BULLETS; n++)
    {
        BulletData &b = bullets[n];

        if (!b.active)
            continue;

        b.lifeCounter--;

        // DBP: move object b,8.5
        b.x += b.dirX * 8.5f;
        b.y += b.dirY * 8.5f;
        b.z += b.dirZ * 8.5f;

        // DBP: гравитация
        b.gravCounter += b.bulletGravity;
        b.y -= b.gravCounter;

        if (b.lifeCounter > 0)
        {
            // Проверка коллизий
            eventSystem->publish(BulletFlightEvent{
                .bullet = b});
        }
        else
        {
            b.active = false;
        }
    }

    // === Эффекты попадания ===
    for (int i = 1; i <= MAX_HIT_EFFECTS; i++)
    {
        HitEffect &h = hits[i];
        if (!h.active)
            continue;

        h.counter -= 3;
        h.angleY = wrapValue(h.angleY + 3.0f);

        if (h.counter < 4)
        {
            h.counter = 0;
            h.active = false;
        }
    }

    // === Взрывы ===
    for (int i = 0; i < MAX_EXPLOSIONS; i++)
    {
        ExplosionData &e = explosions[i];
        if (!e.active)
            continue;

        e.counter += e.direction;

        if (e.counter < 1)
            e.direction = 2;

        if (e.counter >= 80)
            e.active = false;
    }
}

// ============================================================
// Эффекты
// ============================================================

void BulletSystem::spawnHitEffect(float x, float y, float z,
                                  float magnifier, int modelType)
{
    for (int i = 1; i <= MAX_HIT_EFFECTS; i++)
    {
        if (!hits[i].active)
        {
            hits[i].active = true;
            hits[i].x = x;
            hits[i].y = y;
            hits[i].z = z;
            hits[i].counter = 100;
            hits[i].angleY = (float)rnd(360);
            hits[i].magnifier = magnifier;
            hits[i].modelType = modelType;
            return;
        }
    }
}

void BulletSystem::spawnExplosion(float x, float y, float z, float range,
                                  float yaw, float pitch, float roll)
{
    for (int i = 0; i < MAX_EXPLOSIONS; i++)
    {
        if (!explosions[i].active)
        {
            explosions[i].active = true;
            explosions[i].x = x;
            explosions[i].y = y;
            explosions[i].z = z;
            explosions[i].counter = 55;
            explosions[i].direction = -1;
            explosions[i].range = range;

            // DBP: ring ориентируется по танку
            explosions[i].ringYaw = yaw;
            explosions[i].ringPitch = pitch;
            explosions[i].ringRoll = roll + 4.0f - (float)rnd(8);

            // DBP: rotate object 402,0,rnd(359),0
            explosions[i].ballYaw = (float)rnd(360);
            return;
        }
    }
}

// ============================================================
// Отрисовка
// ============================================================

void BulletSystem::render() const
{
    // === Пули ===
    for (int n = 1; n <= MAX_BULLETS; n++)
    {
        const BulletData &b = bullets[n];
        if (!b.active)
            continue;

        // Выбор модели: типы 7-8 → bullet2
        const Model &mdl = (b.bulletType == 2) ? bulletModel2 : bulletModel1;

        // Направление → углы для рендера
        float yaw = atan2f(b.dirX, b.dirZ) * RAD2DEG;
        float pitch = asinf(b.dirY) * RAD2DEG;
        float sc = b.bulletScale;

        rlPushMatrix();
        rlTranslatef(b.x, b.y, b.z);
        rlRotatef(yaw + 180.0f, 0, 1, 0); // DBP→OpenGL
        rlRotatef(pitch, 1, 0, 0);        // +pitch = нос вверх
        rlScalef(sc, sc, sc);
        DrawModel(b.superBullet ? bulletModel3 : mdl, {0, 0, 0}, 1.0f, WHITE);
        rlPopMatrix();
    }

    // === Эффекты попадания ===
    for (int i = 1; i <= MAX_HIT_EFFECTS; i++)
    {
        const HitEffect &h = hits[i];
        if (!h.active)
            continue;

        // DBP: sc=(20+(100-hit(n))/1.25)*tk#(n,39)
        float sc = (20.0f + (100.0f - h.counter) / 1.25f) * h.magnifier / 100.0f;

        const Model &mdl = (h.modelType == 2) ? hitModel2 : hitModel1;

        // tint: hit1 — белая вспышка, hit2 — цветное свечение
        // (их текстура hit2 синяя → белый tint выбеливает, цветной tint
        //  ограничивает добавку по каналам и возвращает оттенок)
        Color tint = (h.modelType == 2)
                         ? Color{100, 80, 230, 255}   // ← hit2: сине-фиолетовый, стартовый
                         : Color{255, 255, 255, 255}; // ← hit1: как в оригинале

        BeginBlendMode(BLEND_ADDITIVE);
        rlPushMatrix();
        rlTranslatef(h.x, h.y, h.z);
        rlRotatef(h.angleY, 0, 1, 0);
        rlScalef(sc, sc, sc);
        DrawModel(mdl, {0, 0, 0}, 1.0f, tint);
        rlPopMatrix();
        EndBlendMode();
    }

    // === Взрывы ===
    for (int i = 0; i < MAX_EXPLOSIONS; i++)
    {
        const ExplosionData &e = explosions[i];
        if (!e.active)
            continue;

        float r1 = (1.8f * (95.0f - e.counter * 1.1f) * e.range) / 100.0f;
        float r2 = (1.9f * (95.0f - e.counter / 1.1f) * e.range) / 100.0f;

        if (r1 < 0.1f)
            r1 = 0.1f;
        if (r2 < 0.1f)
            r2 = 0.1f;

        // ★ DBP scale object = проценты → делим на 100
        r1 /= 100.0f;
        r2 /= 100.0f;

        // Включаем аддитивное смешивание (DBP: ghost object on)
        BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);

        // Кольцо — ориентация по уничтоженному танку
        rlPushMatrix();
        rlTranslatef(e.x, e.y, e.z);
        rlRotatef(e.ringYaw + 180.0f, 0, 1, 0);
        rlRotatef(e.ringPitch, 1, 0, 0);
        rlRotatef(e.ringRoll, 0, 0, 1);
        rlScalef(r1, r1, r1);
        DrawModel(ringModel, {0, 0, 0}, 1.0f, Color{255, 255, 20, 180});
        rlPopMatrix();

        // Шар — случайный Y-поворот
        rlPushMatrix();
        rlTranslatef(e.x, e.y, e.z);
        rlRotatef(e.ballYaw, 0, 1, 0);
        rlScalef(r2, r2, r2);
        DrawModel(ballModel, {0, 0, 0}, 1.0f, Color{255, 255, 20, 180});
        rlPopMatrix();

        // Возвращаем стандартный alpha blending
        EndBlendMode();
    }
}

void BulletSystem::onTankFired(const TankFiredEvent &e)
{
    BulletData &b = bullets[e.tankId];
    b.active = true;
    b.owner = e.tankId;
    b.x = e.position.x;
    b.y = e.position.y;
    b.z = e.position.z;
    b.dirX = e.direction.x;
    b.dirY = e.direction.y;
    b.dirZ = e.direction.z;
    b.gravCounter = 0;
    b.lifeCounter = e.bulletLifeMax;
    b.bulletPower = e.bulletPower;
    b.bulletGravity = e.bulletGravity;
    b.ownerSquadId = e.tankSquadId;
    b.superBullet = e.isSuperBullet;
    b.hitScale = e.hitScale;
    b.bulletScale = e.bulletScale;
    b.bulletType = e.bulletType;
}

void BulletSystem::onBulletTerrainHit(const BulletTerrainHitEvent &e)
{
    spawnHitEffect(e.position.x, e.position.y, e.position.z, e.hitScale, e.bulletType);
}

void BulletSystem::onBulletTankHit(const BulletTankHitEvent &e)
{
    spawnHitEffect(e.position.x, e.position.y, e.position.z, e.hitScale, e.bulletType);
}

void BulletSystem::onTankDestroyed(const TankDestroyedEvent &e)
{
    // DBP: gam(4)=55, gam(5)=range, gam(6)=-1
    // Кольцо ориентируется по танку, шар — случайный Y
    spawnExplosion(e.position.x, e.position.y, e.position.z,
                   e.explosionRange, e.yaw, e.pitch, e.roll);
}