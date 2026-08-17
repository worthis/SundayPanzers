#include "Game.h"
#include "GameData.h"
#include "SortieSystem.h"
#include <cmath>

Game::Game()
{
}

Game::~Game()
{
}

void Game::Init()
{
    InitAudioDevice();
    initGameData();
    loadAssets();

    SaveSystem::load(saveData); // Загружаем сохранения

    audioSystem.init(&eventSystem);
    terrain.init(&eventSystem);
    treeSystem.init(&eventSystem, &terrain);
    cloudSystem.init(&terrain);
    tankSystem.init(&eventSystem, &terrain);
    bulletSystem.init(&eventSystem);
    powerUpSystem.init(&eventSystem, &terrain, &tankSystem);
    aiSystem.init(&eventSystem, &tankSystem);
    bulletSystem.loadAssets();
    powerUpSystem.loadAssets();

    menuSystem.init(&input, &audioSystem, &terrain, &skybox, &treeSystem, &cloudSystem, &tankSystem, &camera);
    hudSystem.init();

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
        if (menuSystem.isQuitRequested())
        {
            quitRequested = true;
            break;
        }
        if (menuSystem.isFinished())
        {
            MenuResult res = menuSystem.getResult();
            StartBattle(res.level, res.playerSquad, res.enemySquad,
                        res.guestSquad, res.player, res.commander);
        }
        break;
    case GameState::BATTLE_INTRO:
        UpdateBattleIntro(dt);
        break;
    case GameState::BATTLE:
        UpdateBattle(dt);
        break;
    case GameState::BATTLE_END:
        UpdateBattleEnding(dt);
        break;
    case GameState::GAME_COMPLETED:
        UpdateGameCompleted(dt);
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
        DrawBattleIntro();
        break;
    case GameState::BATTLE:
        DrawBattle();
        break;
    case GameState::BATTLE_END:
        DrawBattleEnding();
        break;
    case GameState::GAME_COMPLETED:
        DrawGameCompleted();
        break;
    }
}

void Game::Shutdown()
{
    audioSystem.shutdown();
    menuSystem.shutdown();
    hudSystem.shutdown();
    CloseAudioDevice();

    unloadAssets();
}

// === Логотип разработчика (entra) ===
void Game::StartLogoIntro()
{
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

    bool skip = input.isMenuConfirmPressed() ||
                input.isMenuNextPressed() ||
                input.isTouchPressed();

    if (introTimer > 350.0f && skip)
    {
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

    initFakeTank();
    camera.init(introFakeTank.x, 1000.0f, introFakeTank.z);

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

    // Движение фейкового танка
    UpdateFakeTankMovement();
    // Камера следит за фейковым танком
    camera.track(introFakeTank, terrain, false);

    bool skip = input.isMenuConfirmPressed() ||
                input.isMenuNextPressed() ||
                input.isTouchPressed();

    if (introTimer > 5.0f && skip)
    {
        currentState = GameState::MAIN_MENU;
        menuSystem.start(saveData.maxLevel, saveData.gameCompleted);
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

    input.setTankSelected(playerCommander);

    StartBattleIntro();
}

void Game::StartBattleIntro()
{
    introTimer = 0.0f;
    introGamma = 0.0f;

    initFakeTank();
    camera.init(introFakeTank.x, 1000.0f, introFakeTank.z);

    // Сброс параметров
    battleEndingBounce = 0.0f;
    battleEndingBounceAcc = 0.0f;

    currentState = GameState::BATTLE_INTRO;
}

void Game::UpdateBattleIntro(float dt)
{
    introTimer += dt * 105.0f;

    // Fade in (ga=ga+5 из оригинала)
    introGamma += 5.0f;
    if (introGamma > 255.0f)
        introGamma = 255.0f;

    // Движение фейкового танка
    UpdateFakeTankMovement();
    // Камера следит за фейковым танком
    camera.track(introFakeTank, terrain, false);

    // Выход: если ga>=255 и нажата любая клавиша/мышь
    // В оригинале: mv=abs(joystick x/200)+abs(joystick y/200)+rightkey()+leftkey()+upkey()+downkey()
    bool skip = input.isMenuConfirmPressed() ||
                input.isMenuNextPressed() ||
                input.isTankMoved() ||
                input.isTouchPressed();

    if (introTimer >= 5.0f && skip)
    {
        // Переход к бою
        audioSystem.playBattleMusic();
        currentState = GameState::BATTLE;
    }
}

void Game::DrawBattleIntro()
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

    // === 2D UI (координаты оригинала 640x480) ===
    float offsetX = (SCREEN_WIDTH - 640.0f) / 2.0f;
    float offsetY = (SCREEN_HEIGHT - 480.0f) / 2.0f;

    // Анимация текста "Sunday Panzers" (как в оригинале)
    // ang#=wrapvalue(ang#+0.95)  ang2#=wrapvalue(ang2#+1.35)
    float ang = introTimer * 0.95f;
    float ang2 = introTimer * 1.35f;
    float alpha = introTimer / 2.0f;
    if (alpha > 255.0f)
        alpha = 255.0f;

    float rotation = cosf(ang * DEG2RAD) * 12.0f;
    float scale = 100.0f + sinf(ang2 * DEG2RAD) * 25.0f;

    int biome = terrain.getCurrentBiome();

    // sprite 1,320,240,13
    Rectangle src = {0, 0, (float)texStart.width, (float)texStart.height};
    Rectangle dst = {
        320.0f + offsetX,
        240.0f + offsetY,
        (float)texStart.width * scale / 100.0f,
        (float)texStart.height * scale / 100.0f};
    Vector2 origin = {dst.width / 2.0f, dst.height / 2.0f};
    Color tint = {255, 255, 255, (unsigned char)alpha};
    DrawTexturePro(texStart, src, dst, origin, rotation, tint);

    // Fade in overlay (gamma эффект)
    if (introGamma < 255.0f)
    {
        unsigned char overlayAlpha = (unsigned char)(255 - introGamma);
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{0, 0, 0, overlayAlpha});
    }

    // paste image 84+gam(25),10,10,1 - название сценария
    if (biome >= 1 && biome <= NUM_BIOMES)
    {
        DrawTexture(texScenario[biome - 1], 10, 10, WHITE);
    }

    EndDrawing();
}

// ============================================================
// BATTLE ENDING (camending в оригинале)
// ============================================================

void Game::StartBattleEnding()
{
    introTimer = 0.0f;
    introGamma = 0.0f;

    // gam(1)=0:gam(8)=-1 - игрок больше не управляет
    // gam(22)=0 означает поражение (player squad уничтожен)
    battleEndingVictory = playerSquadAlive && !enemySquadAlive;

    // Фейковый танк стартует из позиции последнего игрока
    // В оригинале: tk#(n,1)=tk#(gam(1),1):tk#(n,3)=tk#(gam(1),3)
    const TankData &lastPlayer = tankSystem.getTank(playerCommander);
    initFakeTank(lastPlayer.x, lastPlayer.z, lastPlayer.yaw);

    currentState = GameState::BATTLE_END;
}

void Game::UpdateBattleEnding(float dt)
{
    introTimer += dt * 105.0f;

    // Fade in (ga=ga+5 из оригинала)
    introGamma += 5.0f;
    if (introGamma > 255.0f)
        introGamma = 255.0f;

    cloudSystem.update(dt);

    accumulator += dt;
    while (accumulator >= FIXED_DT)
    {
        aiSystem.update();

        for (int n = 1; n <= COMBAT_MAX; n++)
        {
            if (tankSystem.getTank(n).type == 0)
                continue;

            AIOutput ai = aiSystem.computeInput(n);
            tankSystem.updateTank(n, ai.xj, ai.yj);
            if (ai.fire)
                tankSystem.fireBullet(n);
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

    // Движение фейкового танка
    UpdateFakeTankMovement();
    // Камера следит за фейковым танком
    camera.track(introFakeTank, terrain, false);

    Camera3D cam = camera.getCamera();
    Vector3 forward = Vector3Subtract(cam.target, cam.position);
    float fwdLen = Vector3Length(forward);
    if (fwdLen > 0.001f)
    {
        forward.x /= fwdLen;
        forward.y /= fwdLen;
        forward.z /= fwdLen;
    }
    audioSystem.setListenerOrientation(camera.getPosition(), forward, cam.up);

    // Выход: если mv>0 и msg>=400
    bool skip = input.isMenuNextPressed() ||
                input.isTouchPressed();

    if (introTimer >= 400.0f && skip)
    {
        // Проверяем, нужно ли показать финальную заставку
        int currentLevel = menuSystem.getResult().level;
        if (currentLevel == 50 && saveData.gameCompleted && !endGamePlayed)
        {
            StartGameCompleted();
        }
        else
        {
            ReturnToMenu();
        }
    }
}

void Game::DrawBattleEnding()
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

    // === 2D UI ===
    float offsetX = (SCREEN_WIDTH - 640.0f) / 2.0f;
    float offsetY = (SCREEN_HEIGHT - 480.0f) / 2.0f;

    float ang = introTimer * 0.95f;
    float ang2 = introTimer * 1.35f;
    float alpha = introTimer / 2.0f;
    if (alpha > 255.0f)
        alpha = 255.0f;

    float rotation = cosf(ang * DEG2RAD) * 12.0f;
    float scale = 100.0f + sinf(ang2 * DEG2RAD) * 25.0f;

    // Выбор текстуры: victory (2001) или defeat (2002)
    Texture2D &resultTex = battleEndingVictory ? texVictory : texDefeat;
    if (resultTex.id != 0)
    {
        Rectangle src = {0, 0, (float)resultTex.width, (float)resultTex.height};
        Rectangle dst = {
            320.0f + offsetX,
            190.0f + offsetY,
            (float)resultTex.width * scale / 100.0f,
            (float)resultTex.height * scale / 100.0f};
        Vector2 origin = {165.0f * scale / 100.0f, 35.0f * scale / 100.0f};
        Color tint = {255, 255, 255, (unsigned char)alpha};
        DrawTexturePro(resultTex, src, dst, origin, rotation, tint);
    }

    // if msg>395 then paste image 2003 - "click to continue"
    if (introTimer > 395.0f && texClick.id != 0)
    {
        // xof=10*cos(ang#)  yof=10*sin(ang#)
        float xof = 10.0f * cosf(ang * DEG2RAD);
        float yof = 10.0f * sinf(ang * DEG2RAD);
        DrawTexture(texClick,
                    (int)(200 + xof + offsetX),
                    (int)(380 - yof + offsetY), WHITE);
    }

    EndDrawing();
}

// Инициализация фейкового танка
void Game::initFakeTank(float x, float z, float yaw)
{
    introFakeTank = TankData{};
    introFakeTank.x = x;
    introFakeTank.z = z;
    introFakeTank.y = terrain.getHeight(introFakeTank.x, introFakeTank.z);
    introFakeTank.yaw = yaw;
    introFakeTank.spin = 0.0f;

    // Первая случайная цель для фейкового танка
    introTarget = {500.0f + GetRandomValue(0, 4000), 0.0f, 500.0f + GetRandomValue(0, 4000)};

    battleEndingBounce = 0.0f;
    battleEndingBounceAcc = 0.0f;
}

void Game::initFakeTank()
{
    introFakeTank = TankData{};
    introFakeTank.x = 500.0f + GetRandomValue(0, 4000);
    introFakeTank.z = 500.0f + GetRandomValue(0, 4000);
    introFakeTank.y = terrain.getHeight(introFakeTank.x, introFakeTank.z);
    introFakeTank.yaw = 0.0f;
    introFakeTank.spin = 0.0f;

    // Первая случайная цель для фейкового танка
    introTarget = {500.0f + GetRandomValue(0, 4000), 0.0f, 500.0f + GetRandomValue(0, 4000)};

    battleEndingBounce = 0.0f;
    battleEndingBounceAcc = 0.0f;
}

// Движение фейкового танка
void Game::UpdateFakeTankMovement()
{
    // Расчёт дистанции до цели
    float dx = introFakeTank.x - introTarget.x;
    float dz = introFakeTank.z - introTarget.z;
    float r = sqrtf(dx * dx + dz * dz);

    // if r#<110 or rnd(100)>97 then новая цель
    if (r < 110.0f || GetRandomValue(0, 99) > 97)
    {
        introTarget = {500.0f + GetRandomValue(0, 4000), 0.0f, 500.0f + GetRandomValue(0, 4000)};
    }

    // Вычисление угла к цели
    float ry = atan2f(introTarget.x - introFakeTank.x, introTarget.z - introFakeTank.z) * RAD2DEG;
    ry = wrapValue(ry);

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

    // Поворот танка (в intro: 0.05, в ending: 0.05 тоже)
    if (xj != 0.0f)
    {
        introFakeTank.spin += xj * 0.05f;
        if (fabsf(introFakeTank.spin) > 0.7f)
        {
            introFakeTank.spin = (introFakeTank.spin < 0) ? -0.7f : 0.7f;
        }
    }
    else if (fabsf(introFakeTank.spin) >= 0.05f)
    {
        introFakeTank.spin /= 1.15f;
        if (fabsf(introFakeTank.spin) <= 0.06f)
            introFakeTank.spin = 0.0f;
    }

    introFakeTank.yaw = wrapValue(introFakeTank.yaw + introFakeTank.spin);

    // Bounce эффект (rnd(1000)>997)
    if (GetRandomValue(0, 999) > 997 && battleEndingBounce <= 0.0f)
    {
        battleEndingBounce = 0.001f;
        battleEndingBounceAcc = 10.0f + GetRandomValue(0, 30);
    }
    if (battleEndingBounce > 0.0f)
    {
        battleEndingBounce += battleEndingBounceAcc;
        battleEndingBounceAcc -= 0.5f;
        if (battleEndingBounce < 0.0f)
            battleEndingBounce = 0.0f;
    }

    // Движение: f#=0.5+r#/500
    float f = 0.5f + r / 500.0f;
    introFakeTank.x = newXValue(introFakeTank.x, introFakeTank.yaw, f);
    introFakeTank.z = newZValue(introFakeTank.z, introFakeTank.yaw, f);
    introFakeTank.y = terrain.getHeight(introFakeTank.x, introFakeTank.z) + battleEndingBounce / 10.0f;

    // Ограничения карты
    if (introFakeTank.x < 370.0f)
        introFakeTank.x = 370.0f;
    if (introFakeTank.z < 370.0f)
        introFakeTank.z = 370.0f;
    if (introFakeTank.x > 4630.0f)
        introFakeTank.x = 4630.0f;
    if (introFakeTank.z > 4630.0f)
        introFakeTank.z = 4630.0f;

    // Синхронизация интерполяции
    introFakeTank.interpX = introFakeTank.x;
    introFakeTank.interpY = introFakeTank.y;
    introFakeTank.interpZ = introFakeTank.z;
    introFakeTank.interpYaw = introFakeTank.yaw;
}

void Game::UpdateBattle(float dt)
{
    if (IsKeyPressed(KEY_F1))
        showDebug = !showDebug;

    if (input.isQuitPressed())
    {
        ReturnToMenu();
        return;
    }

    if (input.isToggleIdPressed())
        showEnemyIDs = !showEnemyIDs;

    // === СМЕНА ТАНКА ===
    int requestedTankId = input.getRequestedTank();
    if (requestedTankId > 0 && requestedTankId != playerCommander && !camera.isSlipCamActive())
    {
        const TankData &newTank = tankSystem.getTank(requestedTankId);
        if (newTank.type > 0 && newTank.energy > 0)
        {
            // Запускаем slipcam
            const TankData &currentTank = tankSystem.getTank(playerCommander);
            camera.startSlipCam(currentTank, newTank);
            requestedTank = requestedTankId;
            TraceLog(LOG_INFO, "Switching to tank %d", requestedTankId);
        }
    }

    // Обновляем slipcam если активен
    if (camera.isSlipCamActive())
    {
        const TankData &targetTank = tankSystem.getTank(requestedTank);
        camera.updateSlipCam(dt, terrain, targetTank);

        if (camera.isSlipCamFinished())
        {
            // Переключаем управление
            playerCommander = requestedTank;
            camera.resetSlipCam();
            requestedTank = 0;
            TraceLog(LOG_INFO, "Switched control to tank %d", playerCommander);
        }
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

            // Если slipcam активен, игрок не управляет
            bool isPlayerControlled = (n == playerCommander) && !camera.isSlipCamActive();
            if (isPlayerControlled)
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
                tankSystem.fireBullet(n);
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

    // === ИНТЕРПОЛЯЦИЯ ===
    float alpha = accumulator / FIXED_DT; // 0.0 .. 1.0
    tankSystem.interpolate(alpha);

    CheckBattleEndConditions();

    // ============================================================
    // Камера следует за танком
    // ============================================================
    const TankData &playerTank = tankSystem.getTank(playerCommander);
    Camera3D cam = camera.getCamera();
    Vector3 forward = Vector3Subtract(cam.target, cam.position);
    float fwdLen = Vector3Length(forward);
    if (fwdLen > 0.001f)
    {
        forward.x /= fwdLen;
        forward.y /= fwdLen;
        forward.z /= fwdLen;
    }
    audioSystem.setListenerOrientation(cam.position, forward, cam.up);

    if (!camera.isSlipCamActive())
        camera.track(playerTank, terrain, input.isRearViewPressed());

    bool skip = input.isMenuNextPressed();
    if (battleEnded && skip)
    {
        StartBattleEnding();
    }
}

void Game::CheckBattleEndConditions()
{
    if (battleEnded)
        return;

    // Если одна из сторон уничтожена
    if (!playerSquadAlive || !enemySquadAlive)
    {
        battleEnded = true;

        // === ПРОГРЕССИЯ ИГРОКА ===
        // Если игрок победил и прошел текущий уровень
        if (playerSquadAlive && !enemySquadAlive)
        {
            int currentLevel = menuSystem.getResult().level;

            // Если это максимальный доступный уровень и он < 50
            if (currentLevel == saveData.maxLevel && saveData.maxLevel < 50)
            {
                saveData.maxLevel++;
                SaveSystem::save(saveData);
                TraceLog(LOG_INFO, "Level completed! New maxLevel: %d", saveData.maxLevel);
            }

            // Если пройден 50 уровень - игра завершена
            if (currentLevel == 50 && !saveData.gameCompleted)
            {
                saveData.gameCompleted = true;
                SaveSystem::save(saveData);
                TraceLog(LOG_INFO, "Game completed!");
            }
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

    const TankData &playerTank = tankSystem.getTank(playerCommander);
    Vector3 playerPos = {playerTank.interpX, playerTank.interpY, playerTank.interpZ};
    hudSystem.render(tankSystem, camera.getCamera(), playerCommander, playerPos, showEnemyIDs);

    if (battleEnded)
    {
        float offsetX = (SCREEN_WIDTH - 640.0f) / 2.0f;
        DrawTexture(texBattleOver, 141 + offsetX, 20, WHITE);
    }

    if (showDebug)
    {
        DrawFPS(10, 10);
    }

    EndDrawing();
}

void Game::StartGameCompleted()
{
    currentState = GameState::GAME_COMPLETED;
    introTimer = 0.0f;
    introGamma = 0.0f;
    endGamePlayed = true;

    // Загружаем финальную заставку
    texEndGame = LoadTexture("data/menu/end.png");

    audioSystem.playEndMusic();
}

void Game::UpdateGameCompleted(float dt)
{
    introTimer += dt * 105.0f;

    // Fade in (ga=ga+1)
    introGamma += 1.0f;
    if (introGamma > 255.0f)
        introGamma = 255.0f;

    // Ожидание клика после полного fade-in
    bool skip = input.isMenuConfirmPressed() ||
                input.isMenuNextPressed() ||
                input.isMenuBackPressed() ||
                input.isTouchPressed();

    if (introGamma >= 255.0f && skip)
    {
        // Fade out
        UnloadTexture(texEndGame);
        texEndGame = {};

        ReturnToMenu();
    }
}

void Game::DrawGameCompleted()
{
    BeginDrawing();
    ClearBackground(BLACK);

    // Центрирование
    float offsetX = (SCREEN_WIDTH - 640.0f) / 2.0f;
    float offsetY = (SCREEN_HEIGHT - 480.0f) / 2.0f;

    // Отрисовка финальной заставки
    if (texEndGame.id != 0)
    {
        DrawTexture(texEndGame, (int)offsetX, (int)offsetY, WHITE);
    }

    // Fade-in overlay
    if (introGamma < 255.0f)
    {
        unsigned char alpha = (unsigned char)(255 - introGamma);
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{0, 0, 0, alpha});
    }

    EndDrawing();
}

void Game::ReturnToMenu()
{
    audioSystem.stopMusic();
    currentState = GameState::MAIN_MENU;
    menuSystem.start(saveData.maxLevel, saveData.gameCompleted);
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

    nearestTankId = 0;
    float minDistance = NEAREST_TANK_DISTANCE_MAX;
    const TankData &player = tankSystem.getTank(playerCommander);

    for (int n = PLAYER_MIN; n <= TANKS_MAX; n++)
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
    audioSystem.updatePlayerEngine(player.rpm, player.energy, player.soundStart, camera.isSlipCamActive());

    // Двигатель ближайшего танка
    NearbyData nearbyData = NearbyData{};
    nearbyData.distance = findNearestTankDistance(nearbyData.id);

    if (nearbyData.id > PLAYER_MIN)
    {
        const TankData &nearest = tankSystem.getTank(nearbyData.id);

        nearbyData.pos = {nearest.x, nearest.y, nearest.z};
        nearbyData.rpm = nearest.rpm;
        nearbyData.soundStart = nearest.soundStart;
    }

    audioSystem.updateNearbyEngine(nearbyData);
}

void Game::loadAssets()
{
    // logo assets
    sndLogo = LoadSound("data/sound/logo.wav");
    texLogo = LoadTexture("data/menu/logo.png");
    texData1 = LoadTexture("data/menu/data1.png");
    texData2 = LoadTexture("data/menu/data2.png");

    // intro assets
    const char *scenarioFiles[6] = {
        "data/menu/sc1.png",
        "data/menu/sc2.png",
        "data/menu/sc3.png",
        "data/menu/sc4.png",
        "data/menu/sc5.png",
        "data/menu/sc6.png"};

    for (int i = 0; i < 6; i++)
    {
        texScenario[i] = LoadTextureColorKey(scenarioFiles[i]);
    }

    texTitle = LoadTextureColorKey("data/menu/title.png");
    texStart = LoadTextureColorKey("data/menu/start.png");
    texVictory = LoadTextureColorKey("data/menu/victory.png");
    texDefeat = LoadTextureColorKey("data/menu/defeat.png");
    texClick = LoadTextureColorKey("data/menu/click.png");
    texBattleOver = LoadTextureColorKey("data/menu/battleover.png");
}

void Game::unloadAssets()
{
    // logo assets
    UnloadSound(sndLogo);
    UnloadTexture(texLogo);
    UnloadTexture(texData1);
    UnloadTexture(texData2);

    // intro assets
    for (int i = 0; i < 6; i++)
    {
        UnloadTexture(texScenario[i]);
    }

    UnloadTexture(texTitle);
    UnloadTexture(texStart);
    UnloadTexture(texVictory);
    UnloadTexture(texDefeat);
    UnloadTexture(texClick);
    UnloadTexture(texBattleOver);
}