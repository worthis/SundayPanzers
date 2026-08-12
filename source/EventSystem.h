#pragma once
#include "raylib.h"
#include "GameData.h"
#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <cstdint>
#include <algorithm>

class EventSystem
{
public:
    using ListenerId = uint64_t;

    // Подписка на событие типа EventType
    template <typename EventType>
    ListenerId subscribe(std::function<void(const EventType &)> callback)
    {
        auto typeIdx = std::type_index(typeid(EventType));
        ListenerId id = nextId_++;

        auto wrapper = [cb = std::move(callback)](const void *event)
        {
            cb(*static_cast<const EventType *>(event));
        };

        listeners_[typeIdx].push_back({id, std::move(wrapper)});
        return id;
    }

    // Публикация события (синхронный вызов всех подписчиков)
    template <typename EventType>
    void publish(const EventType &event)
    {
        auto typeIdx = std::type_index(typeid(EventType));
        auto it = listeners_.find(typeIdx);
        if (it == listeners_.end())
            return;

        // Копируем список: защита от отписки внутри колбэка
        auto snapshot = it->second;
        for (auto &entry : snapshot)
            entry.handler(&event);
    }

    // Отписка по ID
    void unsubscribe(ListenerId id)
    {
        for (auto &[typeIdx, vec] : listeners_)
        {
            vec.erase(std::remove_if(vec.begin(), vec.end(),
                                     [id](const Entry &e)
                                     { return e.id == id; }),
                      vec.end());
        }
    }

    // Очистка (вызывать при выходе из боя)
    void clear() { listeners_.clear(); }

private:
    struct Entry
    {
        ListenerId id;
        std::function<void(const void *)> handler;
    };

    std::unordered_map<std::type_index, std::vector<Entry>> listeners_;
    ListenerId nextId_ = 1;
};

// ============================================================
// События боя — точное соответствие точкам вызова в DBP tanks()
// ============================================================

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
struct TreeDestroyedEvent
{
    int cellX, cellZ;
    float fallAngle; // DBP: tk#(n,14)+180 или anb#+180
};

// DBP: rp<35 (подбор powerup)
struct PowerUpPickedEvent
{
    int tankId;
    int powerUpSlot; // 51..55
    int powerUpType; // 1=barrier(51-52), 2=repair(53), 3=superbullet(54-55)
    Vector3 position;
};

// DBP: tk#(n,44)=tk#(n,46) (активация турбо)
struct TurboActivatedEvent
{
    int tankId;
    Vector3 position;
    bool isPlayer;
};

// DBP: tk#(c,37)<tk#(c,49)/3 and tk#(c,35)=0 (смена текстуры)
struct TankDamagedEvent
{
    int tankId;
    Vector3 position;
};

// DBP: powerup 53 (repair) -> tk#(n,35)=0, texture object n,100+...
struct TankRepairedEvent
{
    int tankId;
    Vector3 position;
};
