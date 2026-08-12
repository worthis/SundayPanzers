#pragma once
#include "raylib.h"
#include "GameConfig.h"
#include "EventSystem.h"
#include "Terrain.h"
#include "Utils.h"

// === Параметры типа танка (из tankloader DBP) ===
struct TankType
{
    float scaleX, scaleY, scaleZ; // scale object n,x,y,z (в процентах / 100)
    float steering;               // tk#(n,7) — макс. скорость поворота
    float maxSpeed;               // tk#(n,8) — макс. скорость
    int fireLimb;                 // tk#(n,16) — limb для стрельбы
    int reloadTime;               // tk#(n,19) — время перезарядки
    int bulletLifeMax;             // tk#(n,20) — длительность полёта пули
    float bulletPower;            // tk#(n,21) — урон пули
    float collisionRange;         // tk#(n,33) — радиус коллизии (1/2 танка)
    float shotAngle;              // tk#(n,36) — начальный угол выстрела
    float energy;                 // tk#(n,37) — энергия
    float hitScale;               // tk#(n,39) — масштаб hitball
    float soundStart;             // tk#(n,40) — базовая скорость звука двигателя
    float collisionHeight;        // tk#(n,41) — высота коллизии (1/2 высоты)
    float bulletGravity;          // tk#(n,42) — гравитация пули
    int turboTime;                // tk#(n,46) — длительность турбо
    int turboReload;              // tk#(n,47) — перезарядка турбо
    float bulletScale;            // масштаб модели пули (DBP: scale object n+100,...)
    int hitModelType;             // 1 = hit.glb, 2 = hit2.glb (DBP: типы 7-8 → hit2.x)
};

// === Состояние одного танка (аналог tk#(n,0..52)) ===
struct TankData
{
    int type;                     // 0: тип танка (>0 = активен, <0 = уничтожен, 0 = нет)
    int baseType;                 // 48: ID танка (сохранен)
    float x, y, z;                // 1-3: позиция
    float pitch, yaw, roll;       // 4-6: углы (pitch, yaw, roll)
    float scaleX, scaleY, scaleZ; // scale танка
    float steering;               // 7
    float maxSpeed;               // 8
    float spin;                   // 9: текущая скорость поворота (spin)
    float accel;                  // 10: текущее ускорение
    float fallForce;              // 11: сила подъёма/падения
    float walkSine;               // 12: эффект "ходьбы" (покачивание)
    int walkCounter;              // 13: эффект "ходьбы" (покачивание)
    float bounceAngle;            // 14: отскок от дерева/танка
    float bounceForce;            // 15: отскок от дерева/танка
    int fireLimb;                 // 16: меш для стрельбы
    int reloadCounter;            // 18: перезарядка
    int reloadTime;               // 19-21: параметры стрельбы
    int bulletLifeMax;             // 19-21: параметры стрельбы
    float bulletPower;            // 19-21: параметры стрельбы
    float bulletScale;            // масштаб модели пули (DBP: scale object n+100,...)
    int hitModelType;             // 1 = hit.glb, 2 = hit2.glb (DBP: типы 7-8 → hit2.x)
    float bounce;                 // 22: отскок
    bool onGround;                // 23: на земле
    float bounceRoll;             // 24-25: случайный наклон при отскоке
    float bouncePitch;            // 24-25: случайный наклон при отскоке
    int target;                   // 26: цель (для AI)
    int aiState;                  // 27-29: AI состояние
    int aiCounter;                // 27-29: AI состояние
    float escapeAngle;            // 27-29: AI состояние
    int aiType;                   // 30-32: AI параметры
    int fireRatio;                // 30-32: AI параметры
    int aimRatio;                 // 30-32: AI параметры
    float collisionRange;         // 33: радиус коллизии
    int squadId;                  // 34: ID команды (цвет)
    bool damaged;                 // 35: флаг смены текстуры (повреждён)
    float shotAngle;              // 36: начальный угол выстрела
    float energy;                 // 37: текущая энергия (здоровье)
    float rpm;                    // 38: rpm для звука двигателя
    float hitScale;               // 39: размер hitball
    float soundStart;             // 40: параметры звука (базовый pitch двигателя)
    float collisionHeight;        // 41: высота коллизии
    float bulletGravity;          // 42: гравитация пули
    bool canFire;                 // 43: может стрелять?
    int turboCounter;             // 44-47: турбо
    int turboCharger;             // 44-47: турбо
    int turboTime;                // 44-47: турбо
    int turboReload;              // 44-47: турбо
    float maxEnergy;              // 49: максимальная энергия
    int barrierCounter;           // 50: барьер
    int superBulletCounter;       // 51: суперпуля
    int hitCounter;               // 52: "в меня попали"
    float animFrame = 0.0f;       // DBP: tk#(n,16) — текущий кадр
    float animSpeed = 0.0f;       // DBP: tk#(n,19) — скорость анимации
    int lastAnimFrame = -1;       // оптимизация: обновлять кости только при смене кадра

    // === Интерполяция для плавной отрисовки ===
    float prevX = 0, prevY = 0, prevZ = 0;
    float prevPitch = 0, prevYaw = 0, prevRoll = 0;

    // Интерполированные значения (заполняются перед рендером)
    float interpX = 0, interpY = 0, interpZ = 0;
    float interpPitch = 0, interpYaw = 0, interpRoll = 0;
};

// === Модель extra на каждый слот (скелетная анимация требует отдельную копию) ===
struct ExtraModelSlot
{
    Model model = {};
    ModelAnimation *anims = nullptr;
    int animCount = 0;
    Texture2D texture = {};
    bool loaded = false;
};

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

    // Сделать выстрел
	void fireBullet(int n);

    // Вызывается ОДИН раз за тик ПОСЛЕ updateTank для всех танков
    void updateCollisions();

    // Интерполяция между prev и current (вызывается перед рендером)
    void interpolate(float alpha);

    // Отрисовка всех танков
    void render() const;
    // Отрисовка щитов танков (рендер в последнюю очередь)
    void renderShields() const;

    // Доступ к данным танка
    const TankData &getTank(int n) const { return tanks[n]; }
    TankData &getTankMut(int n) { return tanks[n]; }
    // Локальный центр меша-дула (в координатах модели, до масштаба)
    Vector3 getMuzzleLocal(int type) const { return muzzleLocal[type]; }

    // Количество активных танков
    int getActiveCount() const;

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
    void renderExtra(int n) const;
    void getMuzzlePosition(int n, float &mx, float &my, float &mz) const;
    void getMuzzleDirection(int n, float &dx, float &dy, float &dz) const;

    void onBulletFlight(const BulletFlightEvent &e);
};