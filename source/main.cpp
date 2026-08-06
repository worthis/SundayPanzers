#include "raylib.h"
#include "Game.h"
#include "GameConfig.h"

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sunday Panzers");
    SetTargetFPS(60);
    DisableCursor();

    Game game;
    game.Init();

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        game.Update(dt);
        game.Draw();
    }

    game.Shutdown();
    CloseWindow();
    return 0;
}