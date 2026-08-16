#pragma once
#include "raylib.h"
#include "GameData.h"
#include <array>
#include <functional>
#include <cstddef>

class EventSystem
{
public:
    using ListenerId = size_t;

    // Максимум подписчиков на один тип события (хватит с запасом)
    static constexpr size_t MAX_LISTENERS = 16;

    template <typename EventType>
    ListenerId subscribe(std::function<void(const EventType &)> callback)
    {
        auto &slots = getSlots<EventType>();

        for (size_t i = 0; i < MAX_LISTENERS; i++)
        {
            if (!slots[i].active)
            {
                slots[i].callback = callback;
                slots[i].active = true;
                return i;
            }
        }
        return static_cast<ListenerId>(-1); // нет свободных слотов
    }

    template <typename EventType>
    void publish(const EventType &event)
    {
        auto &slots = getSlots<EventType>();

        // Копируем индексы активных слотов на случай,
        // если колбэк отпишет кого-то во время итерации
        size_t activeCount = 0;
        size_t activeIndices[MAX_LISTENERS];

        for (size_t i = 0; i < MAX_LISTENERS; i++)
        {
            if (slots[i].active)
            {
                activeIndices[activeCount++] = i;
            }
        }

        for (size_t k = 0; k < activeCount; k++)
        {
            size_t i = activeIndices[k];
            if (slots[i].active && slots[i].callback)
            {
                slots[i].callback(event);
            }
        }
    }

    template <typename EventType>
    void unsubscribe(ListenerId id)
    {
        auto &slots = getSlots<EventType>();
        if (id < MAX_LISTENERS)
        {
            slots[id].active = false;
            slots[id].callback = nullptr;
        }
    }

private:
    template <typename EventType>
    struct ListenerSlot
    {
        std::function<void(const EventType &)> callback;
        bool active = false;
    };

    template <typename EventType>
    static std::array<ListenerSlot<EventType>, MAX_LISTENERS> &getSlots()
    {
        static std::array<ListenerSlot<EventType>, MAX_LISTENERS> slots;
        return slots;
    }
};

// ============================================================
// События боя — точное соответствие точкам вызова в DBP tanks()
// ============================================================

struct BulletFlightEvent
{
    BulletData &bullet;
};

// DBP: dead=1 (земля, дерево, граница карты)
struct BulletTerrainHitEvent
{
    Vector3 position;
    float hitScale;
    int bulletType;
};

// Попадание в танк
struct BulletTankHitEvent
{
    int tankId;
    int attackerId;
    Vector3 position;
    float bulletPower;
    bool isSuperBullet;
    float hitScale;
    int bulletType;
};

struct TankFiredEvent
{
    int tankId;
    int tankSquadId;
    bool isCommander;
    Vector3 position;
    Vector3 direction;
    int bulletType;
    int bulletLifeMax;
    float bulletPower;
    float bulletGravity;
    float bulletScale;
    float hitScale;
    bool isSuperBullet;
};

// Танк уничтожен
struct TankDestroyedEvent
{
    int tankId;
    Vector3 position;
    int squadId;
    float explosionRange;
    float yaw, pitch, roll;
};

// Столкновение двух танков
struct TankCollisionEvent
{
    Vector3 position;
};

// Столкновение танка с деревом
struct TankTreeCollisionEvent
{
    int cellX, cellZ;
    Vector3 position;
    float hitAngle;
};

// DBP: tree(ter(2,xm,zm)-1000,0)=2 (дерево умирает)
/*struct TreeDestroyedEvent
{
    int cellX, cellZ;
    float fallAngle; // DBP: tk#(n,14)+180 или anb#+180
};*/

// Подбор powerup
struct PowerUpPickedEvent
{
    int tankId;
    int powerUpType; // 1=barrier(51-52), 2=repair(53), 3=superbullet(54-55)
    Vector3 position;
};

// DBP: tk#(n,44)=tk#(n,46) (активация турбо)
struct TurboActivatedEvent
{
    int tankId;
    Vector3 position;
};

// DBP: tk#(c,37)<tk#(c,49)/3 and tk#(c,35)=0 (смена текстуры)
/*struct TankDamagedEvent
{
    int tankId;
    Vector3 position;
};*/

// DBP: powerup 53 (repair) -> tk#(n,35)=0, texture object n,100+...
/*struct TankRepairedEvent
{
    int tankId;
    Vector3 position;
};*/
