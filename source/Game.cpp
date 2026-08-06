#include "Game.h"
#include "GameData.h"
#include "SortieSystem.h"
#include <cmath>

Game::Game()
    : currentState(GameState::LOGO_INTRO),
      playerCommander(0),
      accumulator(0.0f),
      showDebug(false),
      playerSquadAlive(true),
      enemySquadAlive(true),
      battleEnded(false),
      introTimer(0.0f),
      logoSoundPlayed(false)
{
}

Game::~Game()
{
}

void Game::Init()
{
    InitAudioDevice();
    initGameData();

    treeSystem.init(&terrain);
    cloudSystem.init(&terrain);
    tankSystem.init(&terrain, &treeSystem);
    bulletSystem.init(&terrain, &tankSystem, &treeSystem);
    powerUpSystem.init(&terrain, &tankSystem);
    aiSystem.init(&tankSystem);
    // camera.init(MAP_CENTER, 500.0f, MAP_CENTER + 200.0f);

    bulletSystem.loadAssets();
    powerUpSystem.loadAssets();

    // Загрузка ассетов для логотипа разработчика
    texLogo = LoadTexture("data/intro/logo.png");
    texData1 = LoadTexture("data/intro/data1.png");
    texData2 = LoadTexture("data/intro/data2.png");
    sndLogo = LoadSound("data/intro/logo.wav");

    currentState = GameState::LOGO_INTRO;
    introTimer = 0.0f;

    // ВРЕМЕННО: Сразу начинаем бой для теста, пока не сделано Меню
    /*PlayerTankInfo player[13] = {};
    player[1].type = 8; player[1].ai = 3;
    player[2].type = 2; player[2].ai = 2;
    player[3].type = 2; player[3].ai = 2;
    player[4].type = 2; player[4].ai = 2;

    StartBattle(10, 1, 3, 5, player, 1);*/
}

void Game::Update(float dt)
{
    input.update();

    switch (currentState)
    {
    case GameState::LOGO_INTRO:
        UpdateLogoIntro(dt);
        break;
    case GameState::GAME_INTRO:
        UpdateGameIntro(dt);
        break;
    case GameState::MAIN_MENU:
        // Здесь будет UpdateMenu(dt)
        break;
    case GameState::BATTLE_INTRO:
        // Здесь будет UpdateBattleIntro(dt)
        break;
    case GameState::BATTLE:
    case GameState::BATTLE_END:
        UpdateBattle(dt);
        break;
    }
}

void Game::Draw()
{
    // BeginDrawing();
    // ClearBackground(BLACK);

    switch (currentState)
    {
    case GameState::LOGO_INTRO:
        DrawLogoIntro();
        break;
    case GameState::GAME_INTRO:
        DrawGameIntro();
        break;
    case GameState::MAIN_MENU:
        // DrawMenu()
        DrawText("MENU (Not implemented yet)", 10, 10, 20, WHITE);
        break;
    case GameState::BATTLE_INTRO:
    case GameState::BATTLE:
    case GameState::BATTLE_END:
        DrawBattle();
        break;
    }

    // EndDrawing();
}

void Game::Shutdown()
{
    CloseAudioDevice();
    // Глобальная выгрузка при закрытии игры
}

// === Логотип разработчика (entra) ===
void Game::UpdateLogoIntro(float dt)
{
    introTimer += dt * 105.0f; // Эмуляция sync rate 105 из DBPro

    if (introTimer >= 100.0f && !logoSoundPlayed)
    {
        PlaySound(sndLogo);
        logoSoundPlayed = true;
    }

    bool skip = IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    if (introTimer > 350.0f && skip)
    {
        UnloadSound(sndLogo);
        UnloadTexture(texLogo);
        UnloadTexture(texData1);
        UnloadTexture(texData2);

        StartGameIntro();
    }
}

void Game::DrawLogoIntro()
{
    BeginDrawing();
    ClearBackground(BLACK);

    // Эффект тряски из оригинала: yu = 300 - et
    float yu = 300.0f - introTimer;
    if (yu < 0)
        yu = 0;

    float dx = GetRandomValue(-(int)(yu / 4), (int)(yu / 4));
    float dy = GetRandomValue(-(int)(yu / 4), (int)(yu / 4));

    DrawTexture(texLogo, 120 + (int)dx, 45 + (int)dy, WHITE);
    DrawTexture(texData1, 2, 400, WHITE);

    if (introTimer > 350.0f)
    {
        DrawTexture(texData2, 258, 360, WHITE);
    }

    EndDrawing();
}

// === Заставка игры (gameintro) ===
void Game::StartGameIntro()
{
    currentState = GameState::GAME_INTRO;
    introTimer = 0.0f;

    // Загрузка тайтла и музыки
    texTitle = LoadTexture("data/intro/title.png");
    musicIntro = LoadMusicStream("data/intro/intro.mp3");
    PlayMusicStream(musicIntro);

    // Генерация случайного ландшафта (без танков)
    int biome = GetRandomValue(1, 6);

    terrain.reset();
    treeSystem.reset();
    cloudSystem.reset();

    terrain.generate(biome);
    skybox.load(biome);
    treeSystem.placeTrees(biome);
    terrain.buildMesh();
    cloudSystem.generate(biome);

    // === Инициализация фейкового танка (аналог n=0 в DBPro) ===
    introFakeTank = TankData{}; // Сброс в дефолтное состояние
    introFakeTank.x = 500.0f + GetRandomValue(0, 4000);
    introFakeTank.z = 500.0f + GetRandomValue(0, 4000);
    introFakeTank.y = terrain.getHeight(introFakeTank.x, introFakeTank.z);
    introFakeTank.yaw = 0.0f;
    introFakeTank.spin = 0.0f;

    // Инициализация нашей камеры
    camera.init(introFakeTank.x, 1000.0f, introFakeTank.z);

    // Первая случайная цель для фейкового танка
    introTarget = {500.0f + GetRandomValue(0, 4000), 0.0f, 500.0f + GetRandomValue(0, 4000)};
}

void Game::UpdateGameIntro(float dt)
{
    UpdateMusicStream(musicIntro);
    introTimer += dt * 105.0f;

    // === Логика движения фейкового танка (точный порт из DBPro) ===
    float dx = introFakeTank.x - introTarget.x;
    float dz = introFakeTank.z - introTarget.z;
    float r = sqrtf(dx * dx + dz * dz);

    if (r < 110.0f || GetRandomValue(0, 100) > 97)
    {
        introTarget = {500.0f + GetRandomValue(0, 4000), 0.0f, 500.0f + GetRandomValue(0, 4000)};
    }

    // Вычисление угла к цели (аналог point object 65000)
    // В DBPro 0 градусов = +Z, 90 = +X. atan2(x, z) дает именно это.
    float ry = atan2f(introTarget.x - introFakeTank.x, introTarget.z - introFakeTank.z) * RAD2DEG;
    ry = wrapValue(ry); // Если у вас функции без namespace, уберите Utils::

    float tanAngle = wrapValue(introFakeTank.yaw - ry);
    int flag = 1;
    float ta = fabsf(tanAngle);
    float xj = 0.0f;

    if (ta >= 355.0f || ta <= 5.0f)
    {
        xj = 0.0f;
    }
    else
    {
        if (ta > 180.0f)
            flag = -flag;
        if (tanAngle > 0.0f)
            xj = -(float)flag;
        if (tanAngle < 0.0f)
            xj = (float)flag;
    }

    if (xj != 0.0f)
    {
        introFakeTank.spin += xj * 0.05f;
        if (fabsf(introFakeTank.spin) > 0.7f)
        {
            if (introFakeTank.spin < 0)
                introFakeTank.spin = -0.7f;
            if (introFakeTank.spin > 0)
                introFakeTank.spin = 0.7f;
        }
    }

    if (xj == 0.0f && fabsf(introFakeTank.spin) >= 0.05f)
    {
        introFakeTank.spin /= 1.15f;
        if (fabsf(introFakeTank.spin) <= 0.06f)
            introFakeTank.spin = 0.0f;
    }

    introFakeTank.yaw = wrapValue(introFakeTank.yaw + introFakeTank.spin);

    // Движение танка
    float f = 0.5f + r / 500.0f;
    introFakeTank.x = newXValue(introFakeTank.x, introFakeTank.yaw, f);
    introFakeTank.z = newZValue(introFakeTank.z, introFakeTank.yaw, f);
    introFakeTank.y = terrain.getHeight(introFakeTank.x, introFakeTank.z);

    // Ограничения карты
    if (introFakeTank.x < 370.0f)
        introFakeTank.x = 370.0f;
    if (introFakeTank.z < 370.0f)
        introFakeTank.z = 370.0f;
    if (introFakeTank.x > 4630.0f)
        introFakeTank.x = 4630.0f;
    if (introFakeTank.z > 4630.0f)
        introFakeTank.z = 4630.0f;

    // === Обновляем нашу камеру, заставляя её следить за фейковым танком! ===
    camera.track(introFakeTank, terrain, false);

    bool skip = IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    if (introTimer > 5.0f && skip)
    {
        StopMusicStream(musicIntro);
        UnloadMusicStream(musicIntro);
        UnloadTexture(texTitle);

        currentState = GameState::MAIN_MENU;
        // Здесь позже будет вызов MenuSystem::Init()
    }
}

void Game::DrawGameIntro()
{
    BeginDrawing();
    ClearBackground(terrain.getBackdropColor());
    BeginMode3D(camera.getCamera());

    camera.applyRange();
    skybox.render();
    terrain.render();
    treeSystem.render();
    cloudSystem.render();

    EndMode3D();

    // Рендер 2D заголовка поверх (аналог sprite 1 из DBPro)
    float ang = introTimer * 0.55f;
    float ang2 = introTimer * 1.35f;
    float alpha = introTimer / 2.0f;
    if (alpha > 255)
        alpha = 255;

    float rotation = cos(ang * DEG2RAD) * 20.0f;
    float scale = 75.0f + sin(ang2 * DEG2RAD) * 40.0f;

    Rectangle source = {0, 0, (float)texTitle.width, (float)texTitle.height};
    Rectangle dest = {320, 240, (float)texTitle.width * scale / 100.0f, (float)texTitle.height * scale / 100.0f};
    Vector2 origin = {texTitle.width / 2.0f, texTitle.height / 2.0f};

    // В Raylib цвет с прозрачностью: WHITE с альфой
    Color tint = {255, 255, 255, (unsigned char)alpha};
    DrawTexturePro(texTitle, source, dest, origin, rotation, tint);

    EndDrawing();
}

void Game::StartBattle(int level, int playerSquad, int enemySquad, int guestSquad, PlayerTankInfo *player, int commander)
{
    // 1. Очистка предыдущего состояния (КРИТИЧНО для DBPro-порта)
    // В оригинале при выходе из боя массивы обнулялись и объекты удалялись.
    // Вам нужно добавить метод Reset() в ваши системы, чтобы очищать std::vector и выгружать меши.
    terrain.reset();
    treeSystem.reset();
    cloudSystem.reset();
    tankSystem.reset();
    bulletSystem.reset();
    powerUpSystem.reset();

    // 2. Генерация мира (аналог maketerrain)
    int sce = 1 + GetRandomValue(0, 14); // 1+rnd(15)
    int biome = lev[level].scenario[sce];

    terrain.generate(biome);
    skybox.load(biome);
    treeSystem.placeTrees(biome);
    terrain.buildMesh();
    cloudSystem.generate(biome);
    aiSystem.setPowupSearchFactor(96 - level); // gam(12) = 96 - level

    // 4. Расстановка юнитов
    playerCommander = commander;
    makeSortie(tankSystem, level, playerSquad, enemySquad, guestSquad, player, playerCommander);

    tankSystem.spawnExtrasForBiome(biome);

    // 5. Камера
    camera.init(MAP_CENTER, 500.0f, MAP_CENTER + 200.0f);

    // Сброс флагов
    accumulator = 0.0f;
    battleEnded = false;
    playerSquadAlive = true;
    enemySquadAlive = true;

    currentState = GameState::BATTLE;
}

void Game::UpdateBattle(float dt)
{
    if (IsKeyPressed(KEY_F1))
        showDebug = !showDebug;
    if (IsKeyPressed(KEY_ESCAPE))
    {
        ReturnToMenu();
        return;
    }

    cloudSystem.update(dt);

    accumulator += dt;
    while (accumulator >= FIXED_DT)
    {
        aiSystem.update();

        playerSquadAlive = false;
        enemySquadAlive = false;

        for (int n = 1; n <= COMBAT_MAX; n++)
        {
            if (tankSystem.getTank(n).type == 0)
                continue;

            // Проверка живости (аналог for p=1 to 12 ... if tk#(p,0)>0)
            if (n <= PLAYER_MAX && tankSystem.getTank(n).energy > 0)
                playerSquadAlive = true;
            if (n > PLAYER_MAX && n <= GUEST_MAX && tankSystem.getTank(n).energy > 0)
                enemySquadAlive = true;

            float xj = 0, yj = 0;
            bool fire = false;

            if (n == playerCommander)
            {
                xj = input.getTankX();
                yj = input.getTankY();
                fire = input.isFirePressed();

                if (input.isTurboPressed())
                {
                    TankData &ptk = tankSystem.getTankMut(n);
                    if (ptk.turboCharger <= 0)
                    {
                        ptk.turboCounter = ptk.turboTime;
                        ptk.turboCharger = ptk.turboReload;
                    }
                }
            }
            else
            {
                AIOutput ai = aiSystem.computeInput(n);
                xj = ai.xj;
                yj = ai.yj;
                fire = ai.fire;
            }

            tankSystem.updateTank(n, xj, yj);
            if (fire)
                bulletSystem.fireBullet(n);
        }

        for (int n = PLAYER_MIN; n <= COMBAT_MAX; n++)
            powerUpSystem.checkPickup(n);

        powerUpSystem.update();
        tankSystem.updateCollisions();
        treeSystem.update();
        bulletSystem.update();

        accumulator -= FIXED_DT;
    }

    // === ИНТЕРПОЛЯЦИЯ ===
    float alpha = accumulator / FIXED_DT; // 0.0 .. 1.0
    tankSystem.interpolate(alpha);

    CheckBattleEndConditions();

    // ============================================================
    // Камера следует за танком
    // ============================================================
    const TankData &playerTank = tankSystem.getTank(playerCommander);
    bool rearView = input.isRearViewPressed();
    camera.track(playerTank, terrain, rearView);
}

void Game::CheckBattleEndConditions()
{
    if (!battleEnded)
    {
        // Если одна из сторон уничтожена
        if (!playerSquadAlive || !enemySquadAlive)
        {
            battleEnded = true;
            currentState = GameState::BATTLE_END;
            // Здесь позже будет вызов camending() и сохранение прогресса
        }
    }
}

void Game::DrawBattle()
{
    BeginDrawing();
    ClearBackground(terrain.getBackdropColor());
    BeginMode3D(camera.getCamera());

    camera.applyRange();

    skybox.render();
    terrain.render();
    treeSystem.render();
    cloudSystem.render();
    tankSystem.render();
    powerUpSystem.render();
    bulletSystem.render();
    tankSystem.renderShields();

    EndMode3D();

    if (showDebug)
        DrawFPS(10, 10);

    if (currentState == GameState::BATTLE_END)
    {
        if (playerSquadAlive)
        {
            DrawText("VICTORY! Click to continue", 200, 200, 30, GREEN);
        }
        else
        {
            DrawText("DEFEAT! Click to continue", 200, 200, 30, RED);
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            ReturnToMenu();
        }
    }

    EndDrawing();
}

void Game::ReturnToMenu()
{
    currentState = GameState::MAIN_MENU;
    // Очистка тяжелых ресурсов (3D модели, меши) перед загрузкой меню
    // tankSystem.unloadAssets();
    // terrain.unloadMesh();
}