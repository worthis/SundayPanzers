#pragma once
#include "raylib.h"
#include "GameConfig.h"
#include "Terrain.h"
#include "TankSystem.h"
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

    // Параметры владельца на момент выстрела
    float bulletPower = 0;
    float bulletGravity = 0;
    int ownerSquadId = 0;
    bool superBullet = false;
    float hitScale = 1.0f;
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
    Model model;
    bool modelLoaded = false;
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
    Model ringModel;
    Model ballModel;
    bool modelsLoaded = false;
};

class BulletSystem
{
public:
    BulletSystem();
    ~BulletSystem();

    void init(Terrain *terrain, TankSystem *tankSystem);
    void fireBullet(int n);
    void update();
    void render() const;
    void loadAssets();

private:
    BulletData bullets[MAX_BULLETS + 1];
    HitEffect hits[MAX_BULLETS + 1];
    ExplosionData explosions[MAX_EXPLOSIONS];

    Terrain *terrain = nullptr;
    TankSystem *tankSystem = nullptr;

    Model bulletModel1;
    Model bulletModel2;
    bool bulletModelsLoaded = false;

    bool checkGroundCollision(const BulletData &b) const;
    bool checkTreeCollision(BulletData &b, int &treeIndex) const;
    bool checkMapBounds(const BulletData &b) const;
    int checkTankCollision(const BulletData &b) const;

    void applyDamage(int targetIdx, const BulletData &b, float collAngle);
    void spawnHitEffect(float x, float y, float z, float magnifier);
    void spawnExplosion(float x, float y, float z, float range);

    void getMuzzlePosition(int n, float &mx, float &my, float &mz) const;
    void getMuzzleDirection(int n, float &dx, float &dy, float &dz) const;
};