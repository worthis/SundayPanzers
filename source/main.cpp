#include "raylib.h"
#include "Game.h"
#include "GameConfig.h"
#include "ConfigSystem.h"
#include <cstdio>

int main()
{
    std::freopen("game_log.txt", "w", stdout);
    std::freopen("game_log.txt", "a", stderr);

    // Загрузка конфигурации
    ConfigSystem &config = ConfigSystem::instance();
    config.loadSettings("settings.json");

    // SetConfigFlags(FLAG_MSAA_4X_HINT);

    const DisplayConfig &display = config.getDisplayConfig();

    if (display.fullscreen)
    {
        SetConfigFlags(FLAG_FULLSCREEN_MODE);
    }
    if (display.vsync)
    {
        SetConfigFlags(FLAG_VSYNC_HINT);
    }

    InitWindow(display.width, display.height, "Sunday Panzers");
    SetTargetFPS(display.targetFPS);
    DisableCursor();

    SetExitKey(KEY_NULL);

    Game game;
    game.Init();

    while (!WindowShouldClose() && !game.isQuitRequested())
    {
        float dt = GetFrameTime();
        game.Update(dt);
        game.Draw();
    }

    game.Shutdown();
    CloseWindow();
    return 0;
}