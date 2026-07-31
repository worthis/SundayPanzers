#include "raylib.h"
#include "rlgl.h"
#include "GameConfig.h"
#include "Utils.h"
#include "Terrain.h"
#include "Skybox.h"
#include "CloudSystem.h"
#include "TreeSystem.h"
#include "FreeCamera.h"
#include <cstdio>

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sunday Panzers - Terrain Test");
    SetTargetFPS(60);

    DisableCursor();

    // Создание ландшафта
    Terrain terrain;
    terrain.generate(1);

    // Создание Skybox (небо)
    Skybox skybox;
    skybox.load(1);

    // Система деревьев
    TreeSystem treeSystem;
    treeSystem.init(&terrain);
    treeSystem.placeTrees(1); // Размещаем деревья для биома 1

    // Строим меш ландшафта
    terrain.buildMesh();

    // Система облаков
    CloudSystem cloudSystem;
    cloudSystem.init(&terrain);
    cloudSystem.generate(1);

    // Камера стартует ВЫШЕ ландшафта и смотрит вниз
    FreeCamera camera;
    // Камера стартует ВЫШЕ центра ландшафта и смотрит вниз
    camera.init((Vector3){2500.0f, 500.0f, 2500.0f});

    bool showDebug = true;

    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();

        if (IsKeyPressed(KEY_B))
        {
            int newBiome = terrain.getCurrentBiome() + 1;
            if (newBiome > 6)
                newBiome = 1;
            terrain.generate(newBiome);
            treeSystem.placeTrees(newBiome);
            skybox.load(newBiome);
            terrain.buildMesh();
            cloudSystem.generate(newBiome);
        }

        if (IsKeyPressed(KEY_F1))
            showDebug = !showDebug;

        cloudSystem.update(deltaTime);

        camera.update(deltaTime);

        Vector3 camPos = camera.getPosition();
        float groundH = terrain.getHeight(camPos.x, camPos.z);

        BeginDrawing();
        ClearBackground(terrain.getBackdropColor());

        BeginMode3D(camera.getCamera());

        // Устанавливаем дальность отрисовки через низкоуровневые функции rlgl
        // Аналог set camera range 6,7450 в DBP
        {
            float nearPlane = 0.1f;
            float farPlane = camera.getFarPlane();
            float fovy = camera.getCamera().fovy * DEG2RAD;
            float aspect = (float)SCREEN_WIDTH / SCREEN_HEIGHT;

            float top = nearPlane * tanf(fovy / 2.0f);
            float bottom = -top;
            float right = top * aspect;
            float left = -right;

            rlMatrixMode(RL_PROJECTION);
            rlLoadIdentity();
            rlFrustum(left, right, bottom, top, nearPlane, farPlane);
            rlMatrixMode(RL_MODELVIEW);
        }

        // 1. Отрисовка неба (фон)
        skybox.render();

        // 2. Отрисовка ландшафта
        terrain.render();

        // 3. Отрисовка деревьев
        treeSystem.render();

        cloudSystem.render();

        EndMode3D();

        // UI
        const char *biomeNames[] = {"", "GRASS", "MOUNTAINS", "DESERT", "FROZEN", "TUNDRA", "MOON"};
        DrawText(TextFormat("BIOME: %s", biomeNames[terrain.getCurrentBiome()]),
                 20, 20, 30, WHITE);
        DrawText("[B] Change Biome  [F1] Debug  [ESC] Toggle Cursor", 20, 60, 20, LIGHTGRAY);
        DrawText("WASD - Move | Mouse - Look | Q/E - Down/Up | Shift - Fast",
                 20, SCREEN_HEIGHT - 40, 18, LIGHTGRAY);
        DrawFPS(20, SCREEN_HEIGHT - 70);

        if (showDebug)
        {
            DrawText(TextFormat("Camera: %.1f, %.1f, %.1f", camPos.x, camPos.y, camPos.z),
                     20, 100, 18, YELLOW);
            DrawText(TextFormat("Ground height under camera: %.1f", groundH),
                     20, 125, 18, YELLOW);
            DrawText(TextFormat("Height above ground: %.1f", camPos.y - groundH),
                     20, 150, 18, YELLOW);
            DrawText(TextFormat("Cursor Hidden: %s", IsCursorHidden() ? "YES" : "NO"),
                     20, 175, 18, YELLOW);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}