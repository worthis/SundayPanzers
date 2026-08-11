#pragma once

#include "raylib.h"
#include "GameConfig.h"
#include "TankSystem.h"
#include <vector>
#include <string>

/**
 * HUDSystem - полный порт интерфейса боя из оригинала DBPro.
 *
 * Элементы интерфейса:
 * - Левая панель: иконки танков, номера, здоровье, барьеры, супер-пули
 * - Мини-карта: границы, точки танков, компас игрока
 * - Нижняя панель: статус перезарядки, пуля, турбо
 * - ID над танками: номера с прозрачностью по дистанции
 */
class HUDSystem
{
public:
    HUDSystem();
    ~HUDSystem();

    void init();
    void shutdown();

    /**
     * Отрисовка всего HUD.
     * Вызывать в Game::DrawBattle() ПОСЛЕ EndMode3D().
     */
    void render(const TankSystem &tankSystem, const Camera3D &camera, int playerCommander,
                const Vector3 &playerPos, bool showEnemyIDs, bool isChangingCamera = false);

private:
    void drawSquadPanel(const TankSystem &tankSystem, int playerCommander,
                        float offsetX, float offsetY);
    void drawMinimap(const TankSystem &tankSystem, int playerCommander,
                     float offsetX, float offsetY);
    void drawBottomPanel(const TankSystem &tankSystem, int playerCommander,
                         float offsetX, float offsetY, bool isChangingCamera = false);
    void drawTankIDs(const TankSystem &tankSystem, const Camera3D &camera,
                     int playerCommander, const Vector3 &playerPos, bool showEnemyIDs);

    // Текстурные ассеты
    Texture2D texMapBounds; // image 150 - границы карты
    Texture2D texRedCircle; // image 36 - красный круг для игрока на карте
    Texture2D texSelCircle; // image 33 - круг выделения

    std::vector<Texture2D> texPlayerMiniNumbers; // 12 номеров (image 21-32)
    std::vector<Texture2D> texPlayerNumbers;     // 12 номеров
    std::vector<Texture2D> texEnemyNumbers;      // 33 номера (image 501-533)

    std::vector<std::vector<Texture2D>> texTankIcons; // [squad][tankType]

    std::vector<Texture2D> texHealthBars; // 27 вариантов (image 50-76)

    Texture2D texCharging;  // image 77
    Texture2D texReloading; // image 78
    Texture2D texReady;     // image 79
    Texture2D texGo;        // image 80
    Texture2D texBullet;    // image 81
    Texture2D texTurbo;     // image 82

    Texture2D texBarrier;     // image 83
    Texture2D texSuperBullet; // image 84
    Texture2D texDestroyed;   // image 49

    std::vector<Texture2D> texSquadColors; // 10 цветов (image 151-160)

    bool initialized;
};