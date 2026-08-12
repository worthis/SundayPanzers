#pragma once
#include "raylib.h"
#include "GameConfig.h"
#include "GameData.h"
#include "EventSystem.h"
#include "Utils.h"
#include <cmath>

class BulletSystem
{
public:
    BulletSystem();
    ~BulletSystem();

    void init(EventSystem *eventSystem);
    void update();
    void render() const;
    void loadAssets();
    void reset();

private:
    EventSystem *eventSystem = nullptr;

    BulletData bullets[MAX_BULLETS + 1];
    HitEffect hits[MAX_BULLETS + 1];
    ExplosionData explosions[MAX_EXPLOSIONS];

    // Модели пуль
    Model bulletModel1; // general bullet
    Model bulletModel2; // heavy bullet
    Model bulletModel3; // super bullet
    bool bulletModelsLoaded = false;

    // Текстуры (загружаются отдельно, т.к. не встроены в .glb)
    Texture2D bulletTex1, bulletTex2, bulletTex3;
    Texture2D hitTex1, hitTex2;
    Texture2D ringTex;
    Texture2D exploTex;
    bool texturesLoaded = false;

    // Модели эффектов попадания
    Model hitModel1; // hit.glb
    Model hitModel2; // hit2.glb
    bool hitModelsLoaded = false;

    // Модели взрыва (одна пара, переиспользуется)
    Model ringModel;
    Model ballModel;
    bool explosionModelsLoaded = false;

    void bindTexture(Model &m, const Texture2D &tex);

    void spawnHitEffect(float x, float y, float z, float magnifier, int modelType);
    void spawnExplosion(float x, float y, float z, float range,
                        float yaw, float pitch, float roll);

    void onTankFired(const TankFiredEvent &e);
    void onBulletTerrainHit(const BulletTerrainHitEvent &e);
    void onBulletTankHit(const BulletTankHitEvent &e);
    void onTankDestroyed(const TankDestroyedEvent &e);
};