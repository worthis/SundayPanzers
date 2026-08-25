#pragma once
#include "raylib.h"

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
    int bulletType = 1;       // 1 = hit, 2 = hit2
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

// ============================================================
// Данные уровней — точный порт DBP DATA-секции (initialize)
//
// casenemy[level][15] — 15 вариантов выбора типа танка
// spec[level][7]      — параметры сложности
// lev[level][16]      — данные игрока (кредиты + сценарии)
// ============================================================

struct LevelSpec
{
    int ai1;             // spec(l,1) — AI type primary
    int ai2;             // spec(l,2) — AI type secondary
    int aim;             // spec(l,3) — aim ratio
    int fireRate;        // spec(l,4) — fire rate
    int energyDecreaser; // spec(l,5) — energy decreaser
    int numEnemy;        // spec(l,6) — number of enemy tanks
    int numGuest;        // spec(l,7) — number of guest tanks
};

struct LevelData
{
    int credits;      // lev(l,0)
    int scenario[15]; // lev(l,1-15)
};

// DBP: casenemy(50,15), spec(50,7), lev(50,15)
extern int casenemy[50][15];
extern LevelSpec spec[50];
extern LevelData lev[50];

void initGameData();