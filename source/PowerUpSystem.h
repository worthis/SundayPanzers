#pragma once

#include "raylib.h"
#include "GameConfig.h"
#include "EventSystem.h"
#include "Terrain.h"
#include "TankSystem.h"

// ============================================================
// PowerUp System — точный порт DBP powup() + PowUpPicker
//
// DBP: 5 объектов (51-55)
//   51,52 = barrier  (pup1.x) → tk#(n,50)+=1500
//   53    = repair   (pup3.x) → energy += maxEnergy/2
//   54,55 = superbullet (pup2.x) → tk#(n,51)+=2000
//
// Данные синхронизируются в TankSystem::tanks[51-55]
// для AI target designation (AI ищет pup по tk#(t,0)>0).
// ============================================================

struct PowerUpData
{
    bool active = false; // tk#(o,0): 0=waiting, 1=visible
    float x = 0.0f;      // tk#(o,1)
    float y = 0.0f;      // tk#(o,2)
    float z = 0.0f;      // tk#(o,3)
    int spawnTimer = 0;  // tk#(o,4): countdown to appear
    float angleY = 0.0f; // tk#(o,5): rotation
    int type = 0;        // 0=barrier, 1=superbullet, 2=repair
};

class PowerUpSystem
{
public:
    PowerUpSystem() = default;

    void init(EventSystem *eventSystem, Terrain *terrain, TankSystem *tankSystem);
    void loadAssets();
    void update();           // Вращение + таймер появления + синхронизация с TankSystem
    void checkPickup(int n); // Подбор для танка n + decrease counters. Вызывается для КАЖДОГО танка 1..50 каждый тик
    void render() const;
    void reset();
    void respawn();

    // Debug: доступ к данным
    const PowerUpData &getPup(int idx) const { return pups[idx]; }
    int getCount() const { return NUM_PUP; }
    bool isModelLoaded(int type) const { return modelLoaded[type]; }

private:
    EventSystem *eventSystem = nullptr;
    Terrain *terrain = nullptr;
    TankSystem *tankSystem = nullptr;

    PowerUpData pups[NUM_PUP];

    Model models[3]; // 0=pup1(barrier), 1=pup2(superbullet), 2=pup3(repair)
    Texture2D textures[3];
    bool modelLoaded[3] = {false, false, false};
    bool textureLoaded[3] = {false, false, false};

    void respawn(int idx); // Переразмещение (DBP: powup(o))
};