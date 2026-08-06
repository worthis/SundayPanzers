#pragma once
#include "raylib.h"
#include "GameConfig.h"
#include "Terrain.h"
#include "TankSystem.h"
#include "TreeSystem.h"
#include "Utils.h"
#include <cmath>

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
    int bulletModelType = 1;  // 1 = bullet, 2 = bullet2
    int hitModelType = 1;     // 1 = hit, 2 = hit2
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

class BulletSystem
{
public:
    BulletSystem();
    ~BulletSystem();

    void init(Terrain *terrain, TankSystem *tankSystem, TreeSystem* treeSystem);
    void fireBullet(int n);
    void update();
    void render() const;
    void loadAssets();
    void reset();

private:
    BulletData bullets[MAX_BULLETS + 1];
    HitEffect hits[MAX_BULLETS + 1];
    ExplosionData explosions[MAX_EXPLOSIONS];

    Terrain *terrain = nullptr;
    TankSystem *tankSystem = nullptr;
    TreeSystem *treeSystem = nullptr;

    // Модели пуль
    Model bulletModel1;
    Model bulletModel2;
    bool bulletModelsLoaded = false;

    // Текстуры (загружаются отдельно, т.к. не встроены в .glb)
    Texture2D bulletTex1, bulletTex2;
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
    Texture2D loadTextureColorKey(const char* path);

    bool checkGroundCollision(const BulletData &b) const;
    bool checkTreeCollision(BulletData& b, int& treeIndex, float& hitAngle) const;
    bool checkMapBounds(const BulletData &b) const;
    int checkTankCollision(const BulletData &b) const;

    void applyDamage(int targetIdx, const BulletData &b, float collAngle);
    void spawnHitEffect(float x, float y, float z, float magnifier, int modelType);
    void spawnExplosion(float x, float y, float z, float range,
                        float yaw, float pitch, float roll);

    void getMuzzlePosition(int n, float &mx, float &my, float &mz) const;
    void getMuzzleDirection(int n, float &dx, float &dy, float &dz) const;
};