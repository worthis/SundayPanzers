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
      introGamma(0.0f),
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

    audioSystem.init();
    treeSystem.init(&terrain);
    cloudSystem.init(&terrain);
    tankSystem.init(&audioSystem, &terrain, &treeSystem);
    bulletSystem.init(&audioSystem, &terrain, &tankSystem, &treeSystem);
    powerUpSystem.init(&audioSystem, &terrain, &tankSystem);
    aiSystem.init(&audioSystem, &tankSystem);
    bulletSystem.loadAssets();
    powerUpSystem.loadAssets();

    menuSystem.init(&input, &audioSystem, &terrain, &skybox, &treeSystem, &cloudSystem, &tankSystem, &camera);

    introTimer = 0.0f;

    StartLogoIntro();
}

void Game::Update(float dt)
{
    input.update();
    audioSystem.update(dt);

    switch (currentState)
    {
    case GameState::LOGO_INTRO:
        UpdateLogoIntro(dt);
        break;
    case GameState::GAME_INTRO:
        UpdateGameIntro(dt);
        break;
    case GameState::MAIN_MENU:
        menuSystem.update(dt);
        if (menuSystem.isFinished())
        {
            MenuResult res = menuSystem.getResult();
            StartBattle(res.level, res.playerSquad, res.enemySquad,
                        res.guestSquad, res.player, res.commander);
        }
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
    switch (currentState)
    {
    case GameState::LOGO_INTRO:
        DrawLogoIntro();
        break;
    case GameState::GAME_INTRO:
        DrawGameIntro();
        break;
    case GameState::MAIN_MENU:
        menuSystem.draw();
        break;
    case GameState::BATTLE_INTRO:
    case GameState::BATTLE:
    case GameState::BATTLE_END:
        DrawBattle();
        break;
    }
}

void Game::Shutdown()
{
    audioSystem.shutdown();
    menuSystem.shutdown();
    CloseAudioDevice();
}

// === Логотип разработчика (entra) ===
void Game::StartLogoIntro()
{
    texLogo = LoadTexture("data/menu/logo.png");
    texData1 = LoadTexture("data/menu/data1.png");
    texData2 = LoadTexture("data/menu/data2.png");
    sndLogo = LoadSound("data/sound/logo.wav");

    currentState = GameState::LOGO_INTRO;
}

void Game::UpdateLogoIntro(float dt)
{
    introTimer += dt * 105.0f; // Эмуляция sync rate 105 из DBPro

    // Fade in (ga=ga+5 из оригинала)
    introGamma += 5.0f;
    if (introGamma > 255.0f)
        introGamma = 255.0f;

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

    // Центрирование: исходные координаты рассчитаны на 640x480
    float offsetX = (SCREEN_WIDTH - 640.0f) / 2.0f;
    float offsetY = (SCREEN_HEIGHT - 480.0f) / 2.0f;

    // Эффект тряски из оригинала: yu = 300 - et
    float yu = 300.0f - introTimer;
    if (yu < 0)
        yu = 0;

    // Тряска логотипа (dx=rnd(yu)/2-(yu/4))
    float dx = GetRandomValue(-(int)(yu / 4), (int)(yu / 4));
    float dy = GetRandomValue(-(int)(yu / 4), (int)(yu / 4));

    // Тряска data1 (ddx=rnd(yu)/10-(yu/20))
    float ddx = GetRandomValue(-(int)(yu / 20), (int)(yu / 20));
    float ddy = GetRandomValue(-(int)(yu / 20), (int)(yu / 20));

    // paste image 1,120+dx,45+dy
    DrawTexture(texLogo, (int)(120 + dx + offsetX), (int)(45 + dy + offsetY), WHITE);
    // paste image 2,2+ddx,400+ddy
    DrawTexture(texData1, (int)(2 + ddx + offsetX), (int)(400 + ddy + offsetY), WHITE);

    // if et>350 then paste image 3,258,360
    if (introTimer > 350.0f)
    {
        DrawTexture(texData2, (int)(258 + offsetX), (int)(360 + offsetY), WHITE);
    }

    // Fade-in overlay (аналог set gamma ga,ga,ga)
    if (introGamma < 255.0f)
    {
        unsigned char alpha = (unsigned char)(255 - introGamma);
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{0, 0, 0, alpha});
    }

    EndDrawing();
}

// === Заставка игры (gameintro) ===
void Game::StartGameIntro()
{
    introTimer = 0.0f;
    introGamma = 0.0f;

    // Загрузка тайтла и музыки
    texTitle = LoadTexture("data/menu/title.png");
    /// musicIntro = LoadMusicStream("data/music/intro.ogg");
    // PlayMusicStream(musicIntro);

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

    audioSystem.playIntroMusic();
    currentState = GameState::GAME_INTRO;
}

void Game::UpdateGameIntro(float dt)
{
    // UpdateMusicStream(musicIntro);
    introTimer += dt * 105.0f;

    // Fade in (ga=ga+5 из оригинала)
    introGamma += 5.0f;
    if (introGamma > 255.0f)
        introGamma = 255.0f;

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

    // === ИСПРАВЛЕНИЕ БАГА: Синхронизация для камеры ===
    introFakeTank.interpX = introFakeTank.x;
    introFakeTank.interpY = introFakeTank.y;
    introFakeTank.interpZ = introFakeTank.z;
    introFakeTank.interpYaw = introFakeTank.yaw;

    // === Обновляем нашу камеру, заставляя её следить за фейковым танком! ===
    camera.track(introFakeTank, terrain, false);

    bool skip = IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    if (introTimer > 5.0f && skip)
    {
        // StopMusicStream(musicIntro);
        // UnloadMusicStream(musicIntro);
        UnloadTexture(texTitle);

        currentState = GameState::MAIN_MENU;
        menuSystem.start(10, false); // maxLevel, gameCompleted
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

    // Центрирование: исходные координаты рассчитаны на 640x480
    float offsetX = (SCREEN_WIDTH - 640.0f) / 2.0f;
    float offsetY = (SCREEN_HEIGHT - 480.0f) / 2.0f;

    // Рендер 2D заголовка поверх (аналог sprite 1 из DBPro)
    float ang = introTimer * 0.55f;
    float ang2 = introTimer * 1.35f;
    float alpha = introTimer / 2.0f;
    if (alpha > 255)
        alpha = 255;

    float rotation = cos(ang * DEG2RAD) * 20.0f;
    float scale = 75.0f + sin(ang2 * DEG2RAD) * 40.0f;

    Rectangle source = {0, 0, (float)texTitle.width, (float)texTitle.height};
    // В оригинале: sprite 1,320,240,60000
    Rectangle dest = {320.0f + offsetX, 240.0f + offsetY,
                      (float)texTitle.width * scale / 100.0f,
                      (float)texTitle.height * scale / 100.0f};
    // В оригинале: offset sprite 1,253,192 (центр спрайта)
    Vector2 origin = {dest.width / 2.0f, dest.height / 2.0f};

    // В Raylib цвет с прозрачностью: WHITE с альфой
    Color tint = {255, 255, 255, (unsigned char)alpha};
    DrawTexturePro(texTitle, source, dest, origin, rotation, tint);

    // Fade-in overlay (аналог set gamma ga,ga,ga)
    if (introGamma < 255.0f)
    {
        unsigned char alphaOverlay = (unsigned char)(255 - introGamma);
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{0, 0, 0, alphaOverlay});
    }

    EndDrawing();
}

void Game::StartBattle(int level, int playerSquad, int enemySquad, int guestSquad, PlayerTankInfo *player, int commander)
{
    terrain.reset();
    treeSystem.reset();
    cloudSystem.reset();
    tankSystem.reset();
    bulletSystem.reset();
    powerUpSystem.reset();

    int sce = 1 + GetRandomValue(0, 14); // 1+rnd(15)
    int biome = lev[level - 1].scenario[sce - 1];

    terrain.generate(biome);
    skybox.load(biome);
    treeSystem.placeTrees(biome);
    terrain.buildMesh();
    cloudSystem.generate(biome);
    aiSystem.setPowupSearchFactor(96 - level); // gam(12) = 96 - level

    playerCommander = commander;
    makeSortie(tankSystem, level, playerSquad, enemySquad, guestSquad, player, playerCommander);

    tankSystem.spawnExtrasForBiome(biome);
    powerUpSystem.respawn();

    // Сброс флагов
    accumulator = 0.0f;
    battleEnded = false;
    playerSquadAlive = true;
    enemySquadAlive = true;

    audioSystem.playBattleMusic();
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
            if (tankSystem.getTank(n).type <= 0)
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

            if (n == playerCommander)
            {
                const TankData &ptk = tankSystem.getTank(n);
                audioSystem.updatePlayerPos(ptk.x, ptk.y, ptk.z);
            }
        }

        for (int n = PLAYER_MIN; n <= COMBAT_MAX; n++)
            powerUpSystem.checkPickup(n);

        powerUpSystem.update();
        tankSystem.updateCollisions();
        treeSystem.update();
        bulletSystem.update();

        accumulator -= FIXED_DT;
    }

    updateEngineSounds();

    // Обработка M для mute музыки (keystate(50))
    if (IsKeyPressed(KEY_M))
    {
        audioSystem.toggleMusicMute();
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
            DrawText("VICTORY! Click to continue", SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f, 30, GREEN);
        }
        else
        {
            DrawText("DEFEAT! Click to continue", SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f, 30, RED);
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
    audioSystem.stopMusic();
    currentState = GameState::MAIN_MENU;
    menuSystem.start(10, false); // Возврат в меню после боя
    audioSystem.playMenuMusic();
}

float Game::findNearestTankDistance(int &nearestTankId) const
{
    // Точный порт из DBPro:
    // gam(2)=7500:gam(3)=0
    // for n=1 to obmax:
    //   dxl=tk#(n,1)-tk#(gam(1),1):dzl=...:dyl=...
    //   rp1=sqrt(...)
    //   if rp1<gam(2) and n<>gam(1) and tk#(n,0)>0 then gam(2)=rp1:gam(3)=n

    float minDistance = 7500.0f;
    nearestTankId = 0;
    const TankData &player = tankSystem.getTank(playerCommander);

    for (int n = PLAYER_MIN; n <= COMBAT_MAX; n++)
    {
        if (n == playerCommander)
            continue;

        const TankData &t = tankSystem.getTank(n);
        if (t.type <= 0)
            continue;
        if (t.energy <= 0)
            continue;

        float dist = Vector3Distance({player.x, player.y, player.z}, {t.x, t.y, t.z});
        if (dist < minDistance)
        {
            minDistance = dist;
            nearestTankId = n;
        }
    }

    return minDistance;
}

void Game::updateEngineSounds()
{
    const TankData &player = tankSystem.getTank(playerCommander);
    bool changingCamera = false; //(camera.isChanging()); // gam(8)>0 означает что камера переключается
    audioSystem.updatePlayerEngine(player.rpm, player.energy, player.soundStart, changingCamera);

    // Двигатель ближайшего танка
    int nearestId = 0;
    float nearestDist = findNearestTankDistance(nearestId);
    if (nearestId > PLAYER_MIN && nearestDist < 7500.0f)
    {
        const TankData &nearest = tankSystem.getTank(nearestId);
        audioSystem.updateNearbyEngine(nearest.rpm, nearest.soundStart, nearestDist);
    }
}