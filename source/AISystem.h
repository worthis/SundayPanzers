#pragma once
#include "EventSystem.h"
#include "TankSystem.h"
#include "GameConfig.h"

// Результат работы AI для одного танка
struct AIOutput
{
    float xj = 0.0f;   // -1..1 поворот
    float yj = 0.0f;   // -1..1 газ/тормоз
    bool fire = false; // запрос выстрела
};

class AISystem
{
public:
    void init(EventSystem *eventSystem, TankSystem *tankSystem);

    // Вызывается ОДИН раз за тик ПЕРЕД циклом updateTank
    void update();

    // Вычисляет управление для AI-танка n
    AIOutput computeInput(int n);

    // gam(12) = 96 - level: порог поиска powerup
    void setPowupSearchFactor(int v) { powupSearchFactor = v; }

private:
    EventSystem *eventSystem = nullptr;
    TankSystem *tankSystem = nullptr;

    int changeTargetClock = 0;  // DBP: gam(11), 0..300
    int powupSearchFactor = 90; // DBP: gam(12)

    int findTarget(int n) const;       // Поиск ближайшей цели для танка n
    float range2D(int a, int b) const; // 2D-дистанция между танками
    float range3D(int a, int b) const; // 3D-дистанция
};