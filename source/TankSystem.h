#pragma once
#include "raylib.h"
#include "GameConfig.h"
#include "Terrain.h"
#include "Utils.h"

// === Параметры типа танка (из tankloader DBP) ===
struct TankType
{
    float scale[3];        // scale object n,x,y,z (в процентах / 100)
    float steering;        // tk#(n,7) — макс. скорость поворота
    float maxSpeed;        // tk#(n,8) — макс. скорость
    int fireLimb;          // tk#(n,16) — limb для стрельбы
    int reloadTime;        // tk#(n,19) — время перезарядки
    int bulletLength;      // tk#(n,20) — длительность полёта пули
    float bulletPower;     // tk#(n,21) — урон пули
    float collisionRange;  // tk#(n,33) — радиус коллизии (1/2 танка)
    float shotAngle;       // tk#(n,36) — начальный угол выстрела
    float energy;          // tk#(n,37) — энергия
    float hitScale;        // tk#(n,39) — масштаб hitball
    float soundStart;      // tk#(n,40) — базовая скорость звука двигателя
    float collisionHeight; // tk#(n,41) — высота коллизии (1/2 высоты)
    float bulletGravity;   // tk#(n,42) — гравитация пули
    int turboTime;         // tk#(n,46) — длительность турбо
    int turboReload;       // tk#(n,47) — перезарядка турбо
};

// === Состояние одного танка (аналог tk#(n,0..52)) ===
struct TankData
{
    // 0: тип танка (>0 = активен, <0 = уничтожен, 0 = нет)
    int type;

    // 1-3: позиция
    float x, y, z;

    // 4-6: углы (pitch, yaw, roll)
    float pitch, yaw, roll;

    // 7-8: параметры из TankType (копируются при загрузке)
    float steering;
    float maxSpeed;

    // 9: текущая скорость поворота (spin)
    float spin;

    // 10: текущее ускорение
    float accel;

    // 11: сила подъёма/падения
    float fallForce;

    // 12-13: эффект "ходьбы" (покачивание)
    float walkSine;
    int walkCounter;

    // 14-15: отскок от дерева/танка
    float bounceAngle;
    float bounceForce;

    // 16: limb для стрельбы
    int fireLimb;

    // 17-18: пуля и перезарядка
    int bulletCounter;
    int reloadCounter;

    // 19-21: параметры стрельбы
    int reloadTime;
    int bulletLength;
    float bulletPower;

    // 22-23: отскок и "на земле"
    float bounce;
    bool onGround;

    // 24-25: случайный наклон при отскоке
    float bounceRoll;
    float bouncePitch;

    // 26: цель (для AI)
    int target;

    // 27-29: AI состояние
    int aiState;
    int aiCounter;
    float escapeAngle;

    // 30-32: AI параметры
    int aiType;
    int fireRatio;
    int aimRatio;

    // 33: радиус коллизии
    float collisionRange;

    // 34: ID команды (цвет)
    int squadId;

    // 35: флаг смены текстуры (повреждён)
    bool damaged;

    // 36: начальный угол выстрела
    float shotAngle;

    // 37: текущая энергия
    float energy;

    // 38: rpm для звука двигателя
    float rpm;

    // 39-40: параметры звука/hitball
    float hitScale;
    float soundStart;

    // 41: высота коллизии
    float collisionHeight;

    // 42: гравитация пули
    float bulletGravity;

    // 43: может стрелять?
    bool canFire;

    // 44-47: турбо
    int turboCounter;
    int turboCharger;
    int turboTime;
    int turboReload;

    // 48: ID танка для картинки
    int pictureId;

    // 49: оригинальная энергия
    float originalEnergy;

    // 50-52: барьер, суперпуля, "меня попали"
    int barrierCounter;
    int bulletFlag;
    int hitCounter;

    // === Интерполяция для плавной отрисовки ===
    float prevX = 0, prevY = 0, prevZ = 0;
    float prevPitch = 0, prevYaw = 0, prevRoll = 0;

    // Интерполированные значения (заполняются перед рендером)
    float interpX = 0, interpY = 0, interpZ = 0;
    float interpPitch = 0, interpYaw = 0, interpRoll = 0;
};

class TankSystem
{
public:
    TankSystem();
    ~TankSystem();

    void init(Terrain *terrain);

    // Загрузка танка (аналог tankloader(n,t,c))
    // n = индекс (1-55), t = тип (1-8), c = команда (1-10)
    void loadTank(int n, int tankType, int colorId);

    // Размещение танка на карте
    void placeTank(int n, float x, float z, float yaw);

    // Обновление физики одного танка (аналог цикла в tanks())
    // xj = -1..1 (поворот), yj = -1..1 (газ/тормоз)
    void updateTank(int n, float xj, float yj, float deltaTime);

    // Интерполяция между prev и current (вызывается перед рендером)
    void interpolate(float alpha);

    // Отрисовка всех танков
    void render() const;

    // Доступ к данным танка
    TankData &getTank(int n) { return tanks[n]; }
    const TankData &getTank(int n) const { return tanks[n]; }
    TankData &getTankMut(int n) { return tanks[n]; }

    // Количество активных танков
    int getActiveCount() const;

private:
    TankData tanks[MAX_TANKS];
    TankType tankTypes[9]; // типы 1-8

    Model tankModels[9]; // модели t1-t8
    bool modelsLoaded[9];

    // Текстуры команд (DBP: 101-110 обычные, 111-120 повреждённые, 121-130 уничтоженные)
    Texture2D squadNormal[11];    // t1.png - t10.png
    Texture2D squadDamaged[11];   // td1.png - td10.png
    Texture2D squadDestroyed[11]; // tw1.png - tw10.png
    bool squadTexLoaded[11];

    Terrain *terrain;

    void initTankTypes();
    void loadTankModels();
    void unloadTankModels();
    void loadSquadTextures();
    void unloadSquadTextures();
    void resetTank(int n);
};