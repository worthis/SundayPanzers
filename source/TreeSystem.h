#pragma once
#include "raylib.h"
#include "GameConfig.h"
#include "Terrain.h"
#include "EventSystem.h"

// ============================================================
// DBP: tree(75,3)
//   0 > состояние: 0=idle, 1=bounce(качание), 2=fall(падение)
//   1 > угол Y — направление качания/падения
//   2 > swingSpeed — начальная скорость (35), уменьшается на 1/тик
//   3 > tilt — накопленный наклон
// ============================================================
struct TreeAnim
{
    int state = 0;           // 0=idle, 1=bounce, 2=fall
    float angleY = 0.0f;     // направление (DBP: tree(t,1))
    float swingSpeed = 0.0f; // (DBP: tree(t,2))
    float tilt = 0.0f;       // (DBP: tree(t,3))
};

struct Tree
{
    bool active = false;
    Vector3 position = {0, 0, 0};
    float rotationY = 0.0f;
    Vector3 scale = {1, 1, 1};
    int biome = 0;
    int cellX = 0; // координата в ter[][]
    int cellZ = 0;
    TreeAnim anim;
};

class TreeSystem
{
public:
    TreeSystem();
    ~TreeSystem();

    void init(EventSystem *eventSystem, Terrain *terrain);
    void placeTrees(int biome);
    void update();
    void render() const;
    void reset();

    // NEW: попадание в дерево (пуля или танк)
    // cellX, cellZ — координаты клетки в ter[][]
    // angleFromSource — угол от источника удара к дереву (DBP: anb#)
    void hitTree(int cellX, int cellZ, float angleFromSource);

    const Tree &getTree(int i) const { return trees[i]; }
    int getTreeCount() const { return treeCount; }

private:
    EventSystem *eventSystem = nullptr;

    Tree trees[MAX_TREES];
    int treeCount = 0;
    Terrain *terrain = nullptr;

    Model treeModels[7]; // индексы 1-6 для биомов
    Texture2D treeTextures[7];
    bool modelLoaded[7];
    bool textureLoaded[7];

    void loadTreeModels();
    void unloadTreeModels();

    void onBulletFlight(const BulletFlightEvent &e);
    void onTankTreeCollision(const TankTreeCollisionEvent &e);
};