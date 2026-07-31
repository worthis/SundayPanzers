#include "raylib.h"
#include "rlgl.h"
#include "GameConfig.h"
#include "Utils.h"
#include "InputSystem.h"
#include "Terrain.h"
#include "Skybox.h"
#include "CloudSystem.h"
#include "TreeSystem.h"
#include "TankSystem.h"
#include "TankCamera.h"
#include <cstdio>

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sunday Panzers - Terrain Test");
    SetTargetFPS(60);

    DisableCursor();

    // === Создание мира ===
    Terrain terrain;
    terrain.generate(1);

    Skybox skybox;
    skybox.load(1);

    TreeSystem treeSystem;
    treeSystem.init(&terrain);
    treeSystem.placeTrees(1);

    terrain.buildMesh();

    CloudSystem cloudSystem;
    cloudSystem.init(&terrain);
    cloudSystem.generate(1);

    // === Система танков ===
    TankSystem tankSystem;
    tankSystem.init(&terrain);

    // Загружаем танк игрока (тип 1, команда 1)
    tankSystem.loadTank(1, 1, 1);
    tankSystem.placeTank(1, MAP_CENTER, MAP_CENTER, 0.0f);

    // === Камера следует за танком (DBP: track) ===
    TankCamera camera;
    camera.init(MAP_CENTER, 500.0f, MAP_CENTER + 200.0f);

    InputSystem input;

    bool showDebug = true;

    // === Фиксированный timestep (DBP: sync rate 105 ≈ 100 FPS) ===
    const float FIXED_DT = 1.0f / 100.0f; // 100 тиков в секунду, как в DBP
    float accumulator = 0.0f;

    while (!WindowShouldClose())
    {
        float frameTime = GetFrameTime();
        if (frameTime > 0.1f)
            frameTime = 0.1f;

        input.update();

        if (IsKeyPressed(KEY_B) ||
            (input.isGamepadConnected() && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_UP)))
        {
            int newBiome = terrain.getCurrentBiome() + 1;
            if (newBiome > 6)
                newBiome = 1;
            terrain.generate(newBiome);
            treeSystem.placeTrees(newBiome);
            skybox.load(newBiome);
            terrain.buildMesh();
            cloudSystem.generate(newBiome);
            tankSystem.placeTank(1, MAP_CENTER, MAP_CENTER, 0.0f);
        }

        if (IsKeyPressed(KEY_F1))
            showDebug = !showDebug;

        cloudSystem.update(frameTime);

        // ============================================================
        // ИСПРАВЛЕНИЕ #3: Фиксированный timestep для физики
        // DBP работает на sync rate 105 (~100 FPS), все инкременты
        // (spin += 0.02, accel -= 0.028) рассчитаны на 1 тик = 1/100 сек
        // ============================================================
        accumulator += frameTime;
        while (accumulator >= FIXED_DT)
        {
            for (int n = 1; n < MAX_TANKS; n++)
            {
                float xj = 0.0f, yj = 0.0f;

                if (n == 1)
                {
                    xj = input.getTankX();
                    yj = input.getTankY();
                }

                tankSystem.updateTank(n, xj, yj, FIXED_DT);
            }
            accumulator -= FIXED_DT;
        }

        // === ИНТЕРПОЛЯЦИЯ (ключевое исправление ряби) ===
        float alpha = accumulator / FIXED_DT;  // 0.0 .. 1.0
        tankSystem.interpolate(alpha);

        // ============================================================
        // ИСПРАВЛЕНИЕ #2: Камера следует за танком
        // ============================================================
        const TankData &playerTank = tankSystem.getTank(1);
        bool rearView = input.isRearViewPressed();
        camera.track(playerTank, terrain, rearView);

        // === Отрисовка ===
        Vector3 camPos = camera.getPosition();
        float groundH = terrain.getHeight(camPos.x, camPos.z);

        BeginDrawing();
        ClearBackground(terrain.getBackdropColor());

        BeginMode3D(camera.getCamera());

        // Устанавливаем дальность отрисовки через низкоуровневые функции rlgl
        // Аналог set camera range 6,7450 в DBP
        {
            float nearPlane = CAMERA_NEAR;
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

        skybox.render();
        terrain.render();
        treeSystem.render();
        cloudSystem.render();
        tankSystem.render();

        EndMode3D();

        // === UI ===
        const char *biomeNames[] = {"", "GRASS", "MOUNTAINS", "DESERT",
                                    "FROZEN", "TUNDRA", "MOON"};
        DrawText(TextFormat("BIOME: %s", biomeNames[terrain.getCurrentBiome()]),
                 20, 20, 30, WHITE);
        DrawText("[B] Biome  [F1] Debug  [ESC] Quit", 20, 60, 20, LIGHTGRAY);
        DrawText("W/S - Gas/Brake  A/D - Turn  SPACE - Fire  RCtrl - Rear",
                 20, SCREEN_HEIGHT - 40, 18, LIGHTGRAY);
        DrawFPS(20, SCREEN_HEIGHT - 70);

        if (showDebug)
        {
            int y = 100;
            DrawText(TextFormat("Tank pos: %.1f, %.1f, %.1f",
                                playerTank.x, playerTank.y, playerTank.z),
                     20, y, 18, YELLOW);
            y += 25;
            DrawText(TextFormat("Tank yaw: %.1f  spin: %.3f",
                                playerTank.yaw, playerTank.spin),
                     20, y, 18, YELLOW);
            y += 25;
            DrawText(TextFormat("Tank accel: %.3f  rpm: %.3f  onGround: %d",
                                playerTank.accel, playerTank.rpm,
                                (int)playerTank.onGround),
                     20, y, 18, YELLOW);
            y += 25;
            DrawText(TextFormat("Input: tankX=%.1f tankY=%.1f",
                                input.getTankX(), input.getTankY()),
                     20, y, 18, GREEN);
            y += 25;
            DrawText(TextFormat("Camera: %.1f, %.1f, %.1f",
                                camPos.x, camPos.y, camPos.z),
                     20, y, 18, YELLOW);
            y += 25;
            DrawText(TextFormat("Ground under cam: %.1f", groundH),
                     20, y, 18, YELLOW);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}