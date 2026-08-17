#include "raylib.h"
#include "Game.h"
#include "GameConfig.h"
#include <cstdio>

int main()
{
    std::freopen("game_log.txt", "w", stdout);
    std::freopen("game_log.txt", "a", stderr);

    // SetConfigFlags(FLAG_MSAA_4X_HINT);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sunday Panzers");
    SetTargetFPS(60);
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