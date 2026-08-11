#include "HUDSystem.h"
#include <cmath>
#include <algorithm>

namespace HUDConstants
{
    constexpr float SQUAD_PANEL_X = 5.0f;
    constexpr float SQUAD_PANEL_Y_OFFSET = 2.0f;
    constexpr float SQUAD_PANEL_ITEM_HEIGHT = 40.0f;

    constexpr float MINIMAP_X = SCREEN_WIDTH - 102.0f - 2.0f;
    constexpr float MINIMAP_Y = 1.0f;
    constexpr float MINIMAP_SCALE = 50.0f;

    constexpr float BOTTOM_PANEL_Y = 429.0f;
    constexpr float BOTTOM_PANEL_STATUS_X = 195.0f;
    constexpr float BOTTOM_PANEL_BULLET_X = 375.0f;

    constexpr float PLAYER_ID_MAX_DISTANCE = 2500.0f;
    constexpr float ENEMY_ID_MAX_DISTANCE = 2000.0f;
    constexpr float ID_SCALE_MIN_DISTANCE = 500.0f;
}

HUDSystem::HUDSystem()
    : texMapBounds{},
      texRedCircle{},
      texSelCircle{},
      texCharging{},
      texReloading{},
      texReady{},
      texGo{},
      texBullet{},
      texTurbo{},
      texBarrier{},
      texSuperBullet{},
      texDestroyed{},
      initialized(false)
{
}

HUDSystem::~HUDSystem()
{
    if (initialized)
        shutdown();
}

void HUDSystem::init()
{
    if (initialized)
        return;

    texMapBounds = LoadTexture("data/hud/map.png");
    texRedCircle = LoadTexture("data/hud/sq0.png");
    texSelCircle = LoadTextureColorKey("data/hud/sel.png");

    texPlayerMiniNumbers.resize(12);
    for (int i = 0; i < 12; i++)
    {
        std::string path = "data/hud/info" + std::to_string(i + 1) + ".png";
        texPlayerMiniNumbers[i] = LoadTextureColorKey(path.c_str());
    }

    texPlayerNumbers.resize(12);
    for (int i = 0; i < 12; i++)
    {
        std::string path = "data/hud/nu" + std::to_string(i + 1) + ".png";
        texPlayerNumbers[i] = LoadTextureColorKey(path.c_str());
    }

    texEnemyNumbers.resize(33);
    for (int i = 0; i < 33; i++)
    {
        std::string path = "data/hud/enu" + std::to_string(i + 1) + ".png";
        texEnemyNumbers[i] = LoadTextureColorKey(path.c_str());
    }

    texTankIcons.resize(10);
    for (int squad = 0; squad < 10; squad++)
    {
        texTankIcons[squad].resize(8);
        for (int type = 0; type < 8; type++)
        {
            std::string path = "data/menu/tasq" + std::string(1, 'a' + squad) +
                               std::to_string(type + 1) + ".png";
            texTankIcons[squad][type] = LoadTextureColorKey(path.c_str());
        }
    }

    texHealthBars.resize(27);
    for (int i = 0; i < 27; i++)
    {
        std::string path = "data/hud/ene" + std::to_string(i + 1) + ".png";
        texHealthBars[i] = LoadTexture(path.c_str());
    }

    texCharging = LoadTextureColorKey("data/hud/data1.png");
    texReloading = LoadTextureColorKey("data/hud/data2.png");
    texReady = LoadTextureColorKey("data/hud/data3.png");
    texGo = LoadTextureColorKey("data/hud/data4.png");
    texBullet = LoadTextureColorKey("data/hud/data5.png");
    texTurbo = LoadTextureColorKey("data/hud/data6.png");

    texBarrier = LoadTextureColorKey("data/hud/pup1.png");
    texSuperBullet = LoadTextureColorKey("data/hud/pup2.png");
    texDestroyed = LoadTextureColorKey("data/hud/no.png");

    texSquadColors.resize(10);
    for (int i = 0; i < 10; i++)
    {
        std::string path = "data/hud/sq" + std::to_string(i + 1) + ".png";
        texSquadColors[i] = LoadTexture(path.c_str());
    }

    initialized = true;
}

void HUDSystem::shutdown()
{
    if (!initialized)
        return;

    UnloadTexture(texMapBounds);
    UnloadTexture(texRedCircle);
    UnloadTexture(texSelCircle);

    for (auto &tex : texPlayerMiniNumbers)
        UnloadTexture(tex);
    for (auto &tex : texPlayerNumbers)
        UnloadTexture(tex);
    for (auto &tex : texEnemyNumbers)
        UnloadTexture(tex);

    for (auto &squad : texTankIcons)
        for (auto &tex : squad)
            UnloadTexture(tex);

    for (auto &tex : texHealthBars)
        UnloadTexture(tex);

    UnloadTexture(texCharging);
    UnloadTexture(texReloading);
    UnloadTexture(texReady);
    UnloadTexture(texGo);
    UnloadTexture(texBullet);
    UnloadTexture(texTurbo);
    UnloadTexture(texBarrier);
    UnloadTexture(texSuperBullet);
    UnloadTexture(texDestroyed);

    for (auto &tex : texSquadColors)
        UnloadTexture(tex);

    initialized = false;
}

void HUDSystem::render(const TankSystem &tankSystem, const Camera3D &camera, int playerCommander,
                       const Vector3 &playerPos, bool showEnemyIDs, bool isChangingCamera)
{
    if (!initialized)
        return;

    float offsetX = (SCREEN_WIDTH - 640.0f) / 2.0f;
    float offsetY = (SCREEN_HEIGHT - 480.0f) / 2.0f;

    drawSquadPanel(tankSystem, playerCommander, offsetX, offsetY);
    drawMinimap(tankSystem, playerCommander, offsetX, offsetY);
    drawBottomPanel(tankSystem, playerCommander, offsetX, offsetY, isChangingCamera);
    drawTankIDs(tankSystem, camera, playerCommander, playerPos, showEnemyIDs);
}

void HUDSystem::drawSquadPanel(const TankSystem &tankSystem, int playerCommander,
                               float offsetX, float offsetY)
{
    for (int n = PLAYER_MIN; n <= PLAYER_MAX; n++)
    {
        const TankData &tank = tankSystem.getTank(n);
        if (tank.type == 0)
            continue;

        float yv = HUDConstants::SQUAD_PANEL_Y_OFFSET + (n - 1) * HUDConstants::SQUAD_PANEL_ITEM_HEIGHT;

        int squadIndex = (tank.squadId - 1);
        int typeIndex = (tank.baseType - 1);

        if (squadIndex >= 0 && squadIndex < 10 && typeIndex >= 0 && typeIndex < 8)
        {
            if (texTankIcons[squadIndex][typeIndex].id != 0)
            {
                DrawTexture(texTankIcons[squadIndex][typeIndex], 5, (int)(yv + 3 + offsetY), WHITE);
            }
        }

        if (n - 1 < 12 && texPlayerMiniNumbers[n - 1].id != 0)
        {
            DrawTexture(texPlayerMiniNumbers[n - 1], 48, (int)(yv + 10 + offsetY), WHITE);
        }

        if (n == playerCommander && texSelCircle.id != 0)
        {
            DrawTexture(texSelCircle, 47, (int)(yv + 9 + offsetY), WHITE);
        }

        if (tank.barrierCounter > 0 && texBarrier.id != 0)
        {
            DrawTexture(texBarrier, 64, (int)(yv + 10 + offsetY), WHITE);
        }

        if (tank.superBulletCounter > 0 && texSuperBullet.id != 0)
        {
            DrawTexture(texSuperBullet, 76, (int)(yv + 10 + offsetY), WHITE);
        }

        float healthPercent = tank.energy / tank.maxEnergy;
        int healthBarIndex = (int)(healthPercent * 26.0f);
        healthBarIndex = std::max(0, std::min(26, healthBarIndex));
        int va = 26 - healthBarIndex;

        if (va < 27 && texHealthBars[va].id != 0)
        {
            DrawTexture(texHealthBars[va], 14, (int)(yv + 28 + offsetY), WHITE);
        }

        if (tank.energy <= 0 && texDestroyed.id != 0)
        {
            DrawTexture(texDestroyed, 4, (int)(yv + 3 + offsetY), WHITE);
        }
    }
}

void HUDSystem::drawMinimap(const TankSystem &tankSystem, int playerCommander,
                            float offsetX, float offsetY)
{
    if (texMapBounds.id != 0)
    {
        Color tint = {255, 255, 255, 97};
        DrawTexture(texMapBounds, (int)(HUDConstants::MINIMAP_X), HUDConstants::MINIMAP_Y, tint);
    }

    for (int n = PLAYER_MIN; n <= TANKS_MAX; n++)
    {
        const TankData &tank = tankSystem.getTank(n);
        if (tank.type <= 0 || tank.energy <= 0)
            continue;

        float mapX = HUDConstants::MINIMAP_X + tank.x / HUDConstants::MINIMAP_SCALE;
        float mapY = 103.0f - tank.z / HUDConstants::MINIMAP_SCALE;

        if (n == playerCommander)
        {
            float angle = tank.yaw + 90.0f;
            float x2 = mapX - cosf(angle * DEG2RAD) * 7.0f;
            float y2 = mapY - sinf(angle * DEG2RAD) * 7.0f;

            Color lineColor = {40, 30, 20, 255};
            DrawLine(mapX, mapY, x2, y2, lineColor);
        }

        int squadIndex = tank.squadId - 1;
        if (squadIndex >= 0 && squadIndex < 10 && texSquadColors[squadIndex].id != 0)
        {
            DrawTexture(texSquadColors[squadIndex], mapX, mapY, WHITE);
        }

        if (n == playerCommander && texRedCircle.id != 0)
        {
            DrawTexture(texRedCircle, mapX - 1, mapY - 1, WHITE);
        }
    }
}

void HUDSystem::drawBottomPanel(const TankSystem &tankSystem, int playerCommander,
                                float offsetX, float offsetY, bool isChangingCamera)
{
    // gam(8)=0 означает что камера НЕ переключается (slipcam не активен)
    if (isChangingCamera)
        return;

    const TankData &playerTank = tankSystem.getTank(playerCommander);
    if (playerTank.type <= 0)
        return;

    // Turbo
    if (texTurbo.id != 0)
    {
        DrawTexture(texTurbo,
                    (int)(169.0f + offsetX),
                    (int)(429.0f + offsetY),
                    WHITE);
    }

    Texture2D *turboStatusTex = nullptr;
    float turboYOffset = 0.0f;

    if (playerTank.turboCounter > 0)
    {
        turboStatusTex = &texGo;
        turboYOffset = 2.0f;
    }
    else if (playerTank.turboCharger > 0)
    {
        turboStatusTex = &texCharging;
    }
    else
    {
        turboStatusTex = &texReady;
    }

    if (turboStatusTex != nullptr && turboStatusTex->id != 0)
    {
        DrawTexture(*turboStatusTex,
                    (int)(HUDConstants::BOTTOM_PANEL_STATUS_X + offsetX),
                    (int)(HUDConstants::BOTTOM_PANEL_Y + 1 + turboYOffset + offsetY),
                    WHITE);
    }

    // Bullet
    if (texBullet.id != 0)
    {
        DrawTexture(texBullet,
                    (int)(347.0f + offsetX),
                    (int)(429.0f + offsetY),
                    WHITE);
    }

    Texture2D *bulletStatusTex = nullptr;

    if (playerTank.reloadCounter <= 0)
    {
        bulletStatusTex = &texReady;
    }
    else
    {
        bulletStatusTex = &texReloading;
    }

    if (bulletStatusTex != nullptr && bulletStatusTex->id != 0)
    {
        DrawTexture(*bulletStatusTex,
                    (int)(HUDConstants::BOTTOM_PANEL_BULLET_X + offsetX),
                    (int)(HUDConstants::BOTTOM_PANEL_Y + 1 + offsetY),
                    WHITE);
    }
}

void HUDSystem::drawTankIDs(const TankSystem &tankSystem, const Camera3D &camera,
                            int playerCommander, const Vector3 &playerPos, bool showEnemyIDs)
{
    // Вспомогательная функция для проверки, находится ли объект в поле зрения камеры
    auto isInFrontOfCamera = [&camera](const Vector3 &worldPos) -> bool
    {
        // Вектор от камеры к объекту
        Vector3 toObject = {
            worldPos.x - camera.position.x,
            worldPos.y - camera.position.y,
            worldPos.z - camera.position.z};

        // Вектор направления камеры (вперед)
        Vector3 forward = {
            camera.target.x - camera.position.x,
            camera.target.y - camera.position.y,
            camera.target.z - camera.position.z};

        // Нормализация forward
        float fwdLen = sqrtf(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
        if (fwdLen > 0.001f)
        {
            forward.x /= fwdLen;
            forward.y /= fwdLen;
            forward.z /= fwdLen;
        }

        // Скалярное произведение: если < 0, объект за камерой
        float dot = toObject.x * forward.x + toObject.y * forward.y + toObject.z * forward.z;
        return dot > 0.0f; // Объект впереди камеры
    };

    for (int n = PLAYER_MIN; n <= PLAYER_MAX; n++)
    {
        const TankData &tank = tankSystem.getTank(n);
        if (tank.type <= 0 || tank.energy <= 0)
            continue;

        float dx = tank.interpX - playerPos.x;
        float dz = tank.interpZ - playerPos.z;
        float dy = tank.interpY - playerPos.y;
        float distance = sqrtf(dx * dx + dz * dz + dy * dy);

        if (distance >= HUDConstants::PLAYER_ID_MAX_DISTANCE)
            continue;

        float worldY = tank.interpY + tank.collisionHeight * 2.0f + distance / 375.0f + 13.0f;
        Vector3 worldPos = {tank.interpX, worldY, tank.interpZ};

        if (!isInFrontOfCamera(worldPos))
            continue;

        Vector2 screenPos = GetWorldToScreen(worldPos, camera);

        if (screenPos.x < -50.0f || screenPos.x > SCREEN_WIDTH + 50.0f ||
            screenPos.y < -50.0f || screenPos.y > SCREEN_HEIGHT + 50.0f)
            continue;

        float scale = 50.0f;
        if (distance < HUDConstants::ID_SCALE_MIN_DISTANCE)
        {
            scale = 100.0f - (distance / 10.0f);
        }

        float offset = (16.0f * scale) / 100.0f;
        float alpha = (HUDConstants::PLAYER_ID_MAX_DISTANCE - distance) / 25.0f;
        alpha = std::max(0.0f, std::min(255.0f, alpha * 1.5f));

        if (n - 1 < 12 && texPlayerNumbers[n - 1].id != 0)
        {
            Color tint = {255, 255, 255, (unsigned char)alpha};
            Rectangle src = {0, 0, (float)texPlayerNumbers[n - 1].width,
                             (float)texPlayerNumbers[n - 1].height};
            Rectangle dst = {
                screenPos.x - offset,
                screenPos.y - offset - 2.0f,
                (float)texPlayerNumbers[n - 1].width * scale / 100.0f,
                (float)texPlayerNumbers[n - 1].height * scale / 100.0f};
            Vector2 origin = {0, 0};
            DrawTexturePro(texPlayerNumbers[n - 1], src, dst, origin, 0.0f, tint);
        }
    }

    if (!showEnemyIDs)
        return;

    for (int n = ENEMY_MIN; n <= TANKS_MAX; n++)
    {
        const TankData &tank = tankSystem.getTank(n);
        if (tank.type <= 0 || tank.energy <= 0)
            continue;

        float dx = tank.interpX - playerPos.x;
        float dz = tank.interpZ - playerPos.z;
        float dy = tank.interpY - playerPos.y;
        float distance = sqrtf(dx * dx + dz * dz + dy * dy);

        if (distance >= HUDConstants::ENEMY_ID_MAX_DISTANCE)
            continue;

        float worldY = tank.interpY + tank.collisionHeight * 2.0f + distance / 375.0f + 13.0f;
        Vector3 worldPos = {tank.interpX, worldY, tank.interpZ};

        if (!isInFrontOfCamera(worldPos))
            continue;

        Vector2 screenPos = GetWorldToScreen(worldPos, camera);

        if (screenPos.x < -50.0f || screenPos.x > SCREEN_WIDTH + 50.0f ||
            screenPos.y < -50.0f || screenPos.y > SCREEN_HEIGHT + 50.0f)
            continue;

        float scale = 50.0f;
        if (distance < HUDConstants::ID_SCALE_MIN_DISTANCE)
        {
            scale = 100.0f - (distance / 10.0f);
        }

        float offset = (16.0f * scale) / 100.0f;
        float alpha = (HUDConstants::ENEMY_ID_MAX_DISTANCE - distance) / 20.0f;
        alpha = std::max(0.0f, std::min(255.0f, alpha * 1.5f));

        int enemyIndex = n - 13;
        if (enemyIndex < 33 && texEnemyNumbers[enemyIndex].id != 0)
        {
            Color tint = {255, 255, 255, (unsigned char)alpha};
            Rectangle src = {0, 0, (float)texEnemyNumbers[enemyIndex].width,
                             (float)texEnemyNumbers[enemyIndex].height};
            Rectangle dst = {
                screenPos.x - offset,
                screenPos.y - offset - 2.0f,
                (float)texEnemyNumbers[enemyIndex].width * scale / 100.0f,
                (float)texEnemyNumbers[enemyIndex].height * scale / 100.0f};
            Vector2 origin = {0, 0};
            DrawTexturePro(texEnemyNumbers[enemyIndex], src, dst, origin, 0.0f, tint);
        }
    }
}