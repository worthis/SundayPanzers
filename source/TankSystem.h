#pragma once
#include "raylib.h"
#include "GameData.h"
#include "GameConfig.h"
#include "EventSystem.h"
#include "Terrain.h"
#include "TankCamera.h"
#include "Utils.h"

class TankSystem
{
public:
    TankSystem();
    ~TankSystem();

    void init(EventSystem *eventSystem, Terrain *terrain);
    void reset();

    void spawnExtrasForBiome(int biome);

    // Загрузка танка (аналог tankloader(n,t,c))
    // n = индекс (1-55), t = тип (1-8), c = команда (1-10)
    void loadTank(int n, int tankType, int colorId);

    // Размещение танка на карте
    void placeTank(int n, float x, float z, float yaw);

    // Обновление физики одного танка (аналог цикла в tanks())
    // xj = -1..1 (поворот), yj = -1..1 (газ/тормоз)
    void updateTank(int n, float xj, float yj);

    void fireBullet(int n);        // Сделать выстрел
    void updateCollisions();       // Вызывается ОДИН раз за тик ПОСЛЕ updateTank для всех танков
    void interpolate(float alpha); // Интерполяция между prev и current (вызывается перед рендером)

    void render(const TankCamera &camera) const;        // Отрисовка всех танков
    void renderShields(const TankCamera &camera) const; // Отрисовка щитов танков (рендер в последнюю очередь)

    // Доступ к данным танка
    const TankData &getTank(int n) const { return tanks[n]; }
    TankData &getTankMut(int n) { return tanks[n]; }
    Vector3 getMuzzleLocal(int type) const { return muzzleLocal[type]; } // Локальный центр меша-дула (в координатах модели, до масштаба)
    int getActiveCount() const;                                          // Количество активных танков
    Model getTankModel(int type) const { return tankModels[type]; }
    bool isTankModelLoaded(int type) const { return modelsLoaded[type]; }
    Texture2D getSquadTexture(int squadId) const
    {
        if (squadId >= 1 && squadId <= 10)
            return squadTexNormal[squadId];
        return {};
    }

private:
    EventSystem *eventSystem = nullptr;
    Terrain *terrain = nullptr;

    TankData tanks[OBJECTS_MAX + 1];
    TankType tankTypes[MAX_TANK_TYPES + 1]; // типы 1-8

    Model tankModels[MAX_TANK_TYPES + 1]; // модели t1-t8
    bool modelsLoaded[MAX_TANK_TYPES + 1];

    Model superBulletPUPModel;
    bool superBulletPUPModelLoaded = false;

    // Текстуры команд (DBP: 101-110 обычные, 111-120 повреждённые, 121-130 уничтоженные)
    Texture2D squadTexNormal[11];    // t1.png - t10.png
    Texture2D squadTexDamaged[11];   // td1.png - td10.png
    Texture2D squadTexDestroyed[11]; // tw1.png - tw10.png
    bool squadTexLoaded[11];

    // Локальный центр меша fireLimb для каждого типа танка
    // Вычисляется один раз при загрузке модели
    Vector3 muzzleLocal[MAX_TANK_TYPES + 1] = {};

    ExtraModelSlot extraSlots[EXTRA_MAX - EXTRA_MIN + 1]; // слоты 46..50 → индексы 0..4

    void initTankTypes();
    void loadTankModels();
    void unloadTankModels();
    void loadSquadTextures();
    void unloadSquadTextures();
    void loadExtra(int n, int type);
    void unloadExtraSlot(int slot);
    void unloadAllExtras();
    void resetTank(int n);
    void hitTank(int attackerId, int targetId, Vector3 bulletPos, float bulletPower, bool isSuperBullet);
    void applyBounce(int n);
    void updateExtraAnimation(int n);
    void renderTank(int n, const TankCamera &camera) const;
    void renderExtra(int n, const TankCamera &camera) const;
    void getMuzzlePosition(int n, float &mx, float &my, float &mz) const;
    void getMuzzleDirection(int n, float &dx, float &dy, float &dz) const;

    void onBulletFlight(const BulletFlightEvent &e);
    void onTurboActivated(const TurboActivatedEvent &e);
    void onPowerUpPicked(const PowerUpPickedEvent &e);
};