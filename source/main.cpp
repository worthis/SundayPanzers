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
#include "BulletSystem.h"
#include "AISystem.h"
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
    tankSystem.init(&terrain, &treeSystem);

    BulletSystem bulletSystem;
    bulletSystem.init(&terrain, &tankSystem, &treeSystem);
    bulletSystem.loadAssets();

    // === AI ===
    AISystem aiSystem;
    aiSystem.init(&tankSystem);
    aiSystem.setPowupSearchFactor(90); // DBP: 96 - level(10) ≈ 86..90

    // Загружаем вражеские танки для теста
    // DBP: tankloader(e, tipe, se)
    // Enemy squad: индексы 13-40, тип 1-8, команда 2
    tankSystem.loadTank(13, 3, 2);
    tankSystem.placeTank(13, 2500.0f, 4500.0f, 180.0f);
    tankSystem.getTankMut(13).aiType = 3;      // AI tipology
    tankSystem.getTankMut(13).aimRatio = 20;   // spec(l,3)
    tankSystem.getTankMut(13).fireRatio = 985; // spec(l,4)

    tankSystem.loadTank(14, 5, 2);
    tankSystem.placeTank(14, 3000.0f, 4500.0f, 180.0f);
    tankSystem.getTankMut(14).aiType = 5;
    tankSystem.getTankMut(14).aimRatio = 18;
    tankSystem.getTankMut(14).fireRatio = 980;

    tankSystem.loadTank(15, 7, 2);
    tankSystem.placeTank(15, 2000.0f, 4500.0f, 180.0f);
    tankSystem.getTankMut(15).aiType = 7;
    tankSystem.getTankMut(15).aimRatio = 16;
    tankSystem.getTankMut(15).fireRatio = 975;

    // Загружаем танк игрока (тип 1, команда 1)
    tankSystem.loadTank(1, 8, 1);
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
            // AI глобальный тик
            aiSystem.update();

            // Ввод
            float xj = input.getTankX();
            float yj = input.getTankY();
            bool firePressed = IsKeyDown(KEY_SPACE);

            for (int n = 1; n < MAX_TANKS; n++)
            {
                if (tankSystem.getTank(n).type == 0)
                    continue;

                float xj = 0, yj = 0;
                bool fire = false;

                if (n == 1)
                {
                    xj = input.getTankX();
                    yj = input.getTankY();
                    fire = input.isFirePressed();
                }
                else // AI
                {
                    AIOutput ai = aiSystem.computeInput(n);
                    xj = ai.xj;
                    yj = ai.yj;
                    fire = ai.fire;
                }

                tankSystem.updateTank(n, xj, yj, FIXED_DT);

                if (fire)
                    bulletSystem.fireBullet(n);
            }

            tankSystem.updateCollisions();
            treeSystem.update();
            bulletSystem.update();

            accumulator -= FIXED_DT;
        }

        // === ИНТЕРПОЛЯЦИЯ ===
        float alpha = accumulator / FIXED_DT; // 0.0 .. 1.0
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
        bulletSystem.render();

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
            DrawText(TextFormat("Tank: %.1f, %.1f, %.1f  yaw=%.1f",
                                playerTank.x, playerTank.y, playerTank.z, playerTank.yaw),
                     20, y, 18, YELLOW);
            y += 25;
            DrawText(TextFormat("accel=%.3f rpm=%.3f bounce=%.3f onGround=%d",
                                playerTank.accel, playerTank.rpm,
                                playerTank.bounceForce, (int)playerTank.onGround),
                     20, y, 18, YELLOW);
            y += 25;
            DrawText(TextFormat("energy=%.1f/%.1f reload=%d bullet=%d",
                                playerTank.energy, playerTank.originalEnergy,
                                playerTank.reloadCounter, playerTank.bulletCounter),
                     20, y, 18, GREEN);
            y += 25;
            DrawText(TextFormat("Trees: %d active", treeSystem.getTreeCount()),
                     20, y, 18, LIGHTGRAY);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}