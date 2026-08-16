#include "MenuSystem.h"
#include <cmath>
#include <algorithm>

// ============================================================
// Init / Start / Shutdown
// ============================================================

void MenuSystem::init(InputSystem *input, AudioSystem *audioSystem, Terrain *terrain, Skybox *skybox, TreeSystem *treeSystem,
                      CloudSystem *cloudSystem, TankSystem *tankSystem, TankCamera *camera)
{
    m_input = input;
    m_audioSystem = audioSystem;
    m_terrain = terrain;
    m_skybox = skybox;
    m_treeSystem = treeSystem;
    m_cloudSystem = cloudSystem;
    m_tankSystem = tankSystem;
    m_camera = camera;

    // Центрирование: исходное меню 640x480
    m_offsetX = (SCREEN_WIDTH - 640.0f) / 2.0f;
    m_offsetY = (SCREEN_HEIGHT - 480.0f) / 2.0f;

    loadAssets();
    loadPreviewModels();

    // Render textures for tank previews (180x120 = 210-30 x 420-300)
    m_previewLeft = LoadRenderTexture(180, 120);
    m_previewRight = LoadRenderTexture(180, 120);
}

void MenuSystem::start(int maxLevel, bool gameCompleted)
{
    m_maxLevel = maxLevel;
    m_gameCompleted = gameCompleted;
    m_selectedLevel = maxLevel; // gam(15)=gam(14)
    m_playerSquad = 1;          // gam(18)=1
    m_screen = MenuScreen::SELECT_LEVEL_SQUAD;
    m_finished = false;
    m_gamma = 0.0f;
    m_logoAngle = 0.0f;
    m_logoAngle2 = 0.0f;
    m_tankRotation = 0.0f;
    m_commander = 0;

    for (int i = 0; i < 13; i++)
    {
        m_playerTanks[i] = PlayerTankInfo{};
    }

    // Lock/unlock squads (from original menu())
    m_freeSquads = 2;
    if (m_maxLevel > 12)
        m_freeSquads = 3;
    if (m_maxLevel > 15)
        m_freeSquads = 4;
    if (m_maxLevel > 18)
        m_freeSquads = 5;
    if (m_maxLevel > 23)
        m_freeSquads = 6;
    if (m_maxLevel > 27)
        m_freeSquads = 7;
    if (m_maxLevel > 35)
        m_freeSquads = 8;
    if (m_maxLevel > 43)
        m_freeSquads = 9;
    m_frepSquads = m_freeSquads;
    if (m_gameCompleted)
        m_frepSquads = 10;

    // Generate random terrain (tr=1+rnd(599)/100)
    int biome = 1 + GetRandomValue(0, 599) / 100;
    m_terrain->reset();
    m_treeSystem->reset();
    m_cloudSystem->reset();

    m_terrain->generate(biome);
    m_skybox->load(biome);
    m_treeSystem->placeTrees(biome);
    m_terrain->buildMesh();
    m_cloudSystem->generate(biome);

    // Fake tank (analog of tk#(0,...))
    m_fakeTank = TankData{};
    m_fakeTank.x = 500.0f + GetRandomValue(0, 4000);
    m_fakeTank.z = 500.0f + GetRandomValue(0, 4000);
    m_fakeTank.y = m_terrain->getHeight(m_fakeTank.x, m_fakeTank.z);
    m_fakeTank.yaw = 0.0f;
    m_fakeTarget = {500.0f + GetRandomValue(0, 4000), 0.0f, 500.0f + GetRandomValue(0, 4000)};

    m_audioSystem->playMenuMusic();
}

void MenuSystem::shutdown()
{
    unloadAssets();
    unloadPreviewModels();
    UnloadRenderTexture(m_previewLeft);
    UnloadRenderTexture(m_previewRight);
}

// ============================================================
// Update
// ============================================================

void MenuSystem::update(float dt)
{
    // Fade in (ga=ga+5)
    if (m_gamma < 255.0f)
    {
        m_gamma += 5.0f;
        if (m_gamma > 255.0f)
            m_gamma = 255.0f;
    }

    m_cloudSystem->update(dt);
    updateFakeTank();

    // Logo animation (ang#=wrapvalue(ang#+0.15), ang2#=wrapvalue(ang2#+0.2))
    m_logoAngle = wrapValue(m_logoAngle + 0.15f);
    m_logoAngle2 = wrapValue(m_logoAngle2 + 0.2f);

    // Tank rotation for shop preview (tur#=wrapvalue(tur#+0.4))
    m_tankRotation = wrapValue(m_tankRotation + 0.4f);

    // Mouse (with offset correction)
    Vector2 mouse = m_input->getMousePosition();
    bool clicked = m_input->isMouseLeftPressed();

    if (!clicked && m_input->isTouchPressed())
    {
        mouse = m_input->getTouchPosition();
        clicked = true;
    }

    float mx = mouse.x - m_offsetX;
    float my = mouse.y - m_offsetY;

    if (m_screen == MenuScreen::SELECT_LEVEL_SQUAD)
        updateSelectLevelSquad(mx, my, clicked);
    else
        updateShop(mx, my, clicked);
}

void MenuSystem::updateFakeTank()
{
    // Exact port from menu() fake tank logic
    float dx = m_fakeTank.x - m_fakeTarget.x;
    float dz = m_fakeTank.z - m_fakeTarget.z;
    float r = sqrtf(dx * dx + dz * dz);

    if (r < 110.0f || GetRandomValue(0, 100) > 97)
    {
        m_fakeTarget = {500.0f + GetRandomValue(0, 4000), 0.0f, 500.0f + GetRandomValue(0, 4000)};
    }

    float ry = atan2f(m_fakeTarget.x - m_fakeTank.x, m_fakeTarget.z - m_fakeTank.z) * RAD2DEG;
    ry = wrapValue(ry);

    float tanAngle = wrapValue(m_fakeTank.yaw - ry);
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

    // Menu tank turns slower: xj#*0.01, limit 0.3
    if (xj != 0.0f)
    {
        m_fakeTank.spin += xj * 0.01f;
        if (fabsf(m_fakeTank.spin) > 0.3f)
        {
            if (m_fakeTank.spin < 0)
                m_fakeTank.spin = -0.3f;
            if (m_fakeTank.spin > 0)
                m_fakeTank.spin = 0.3f;
        }
    }

    if (xj == 0.0f && fabsf(m_fakeTank.spin) >= 0.05f)
    {
        m_fakeTank.spin /= 1.15f;
        if (fabsf(m_fakeTank.spin) <= 0.06f)
            m_fakeTank.spin = 0.0f;
    }

    m_fakeTank.yaw = wrapValue(m_fakeTank.yaw + m_fakeTank.spin);

    // Menu tank moves slower: f#=0.1+r#/1500
    float f = 0.1f + r / 1500.0f;
    m_fakeTank.x = newXValue(m_fakeTank.x, m_fakeTank.yaw, f);
    m_fakeTank.z = newZValue(m_fakeTank.z, m_fakeTank.yaw, f);
    m_fakeTank.y = m_terrain->getHeight(m_fakeTank.x, m_fakeTank.z);

    // Map limits
    if (m_fakeTank.x < 370.0f)
        m_fakeTank.x = 370.0f;
    if (m_fakeTank.z < 370.0f)
        m_fakeTank.z = 370.0f;
    if (m_fakeTank.x > 4630.0f)
        m_fakeTank.x = 4630.0f;
    if (m_fakeTank.z > 4630.0f)
        m_fakeTank.z = 4630.0f;

    // Sync interp fields for camera
    m_fakeTank.interpX = m_fakeTank.x;
    m_fakeTank.interpY = m_fakeTank.y;
    m_fakeTank.interpZ = m_fakeTank.z;
    m_fakeTank.interpYaw = m_fakeTank.yaw;

    // === Обновляем нашу камеру, заставляя её следить за фейковым танком! ===
    m_camera->track(m_fakeTank, *m_terrain, false);
}

void MenuSystem::updateSelectLevelSquad(float mx, float my, bool clicked)
{
    int som = 0;

    // === Действия ===
    auto doLevelLeft = [&]()
    {
        m_selectedLevel--;
        som = 1;
        if (m_selectedLevel < 1)
        {
            m_selectedLevel = 1;
            som = 2;
        }
    };
    auto doLevelRight = [&]()
    {
        m_selectedLevel++;
        som = 1;
        if (m_selectedLevel > m_maxLevel)
        {
            m_selectedLevel = m_maxLevel;
            som = 2;
        }
    };
    auto doSquadLeft = [&]()
    {
        m_playerSquad--;
        som = 1;
        if (m_playerSquad < 1)
            m_playerSquad = 10;
    };
    auto doSquadRight = [&]()
    {
        m_playerSquad++;
        som = 1;
        if (m_playerSquad > 10)
            m_playerSquad = 1;
    };
    auto doNext = [&]()
    {
        som = 2;
        if (m_frepSquads >= m_playerSquad)
        {
            m_screen = MenuScreen::SHOP;
            som = 1;
            m_selectedTankType = 1;
            m_selectedBox = 1;
            m_minAI = 1 + (int)(spec[m_selectedLevel - 1].ai1 / 2.5f);
            m_maxAI = spec[m_selectedLevel - 1].ai2;
            m_creditsMax = lev[m_selectedLevel - 1].credits;
            m_creditsUsed = 0;
            m_commander = 0;

            for (int i = 0; i < 13; i++)
            {
                m_playerTanks[i] = PlayerTankInfo{};
            }
        }
    };

    // === Мышь ===
    if (clicked)
    {
        if (inBox(mx, my, 190, 200, 210, 220))
            doLevelLeft();
        if (inBox(mx, my, 430, 200, 450, 220))
            doLevelRight();
        if (inBox(mx, my, 190, 250, 210, 270))
            doSquadLeft();
        if (inBox(mx, my, 430, 250, 450, 270))
            doSquadRight();
        if (inBox(mx, my, 270, 440, 370, 470))
            doNext();
    }

    // === Геймпад / Клавиатура ===
    if (m_input->isMenuLeftPressed())
        doLevelLeft();
    if (m_input->isMenuRightPressed())
        doLevelRight();
    if (m_input->isMenuUpPressed())
        doSquadLeft();
    if (m_input->isMenuDownPressed())
        doSquadRight();
    if (m_input->isMenuConfirmPressed() || m_input->isMenuNextPressed())
        doNext();

    if (som == 1)
        m_audioSystem->playMenuClick();
    if (som == 2)
        m_audioSystem->playMenuCancel();
}

void MenuSystem::updateShop(float mx, float my, bool clicked)
{
    int som = 0;

    // === Действия ===
    auto doBoxLeft = [&]()
    {
        m_selectedBox--;
        som = 1;
        if (m_selectedBox < 1)
        {
            m_selectedBox = 1;
            som = 2;
        }
    };
    auto doBoxRight = [&]()
    {
        m_selectedBox++;
        som = 1;
        if (m_selectedBox > 12)
        {
            m_selectedBox = 12;
            som = 2;
        }
    };
    auto doTankLeft = [&]()
    {
        m_selectedTankType--;
        som = 1;
        if (m_selectedTankType < 1)
        {
            m_selectedTankType = 1;
            som = 2;
        }
    };
    auto doTankRight = [&]()
    {
        m_selectedTankType++;
        som = 1;
        if (m_selectedTankType > 8)
        {
            m_selectedTankType = 8;
            som = 2;
        }
    };
    auto doAiReset = [&]()
    {
        if (m_playerTanks[m_selectedBox].type > 0 && m_playerTanks[m_selectedBox].ai == m_maxAI)
        {
            int diff = (m_maxAI - m_minAI) * 2;
            m_playerTanks[m_selectedBox].ai = m_minAI;
            m_creditsUsed -= diff;
        }
    };
    auto doAiUp = [&]()
    {
        som = 2;
        if (m_playerTanks[m_selectedBox].type > 0 && m_playerTanks[m_selectedBox].ai < m_maxAI)
        {
            int eval = m_creditsUsed + 2;
            if (eval <= m_creditsMax)
            {
                m_playerTanks[m_selectedBox].ai++;
                m_creditsUsed += 2;
                som = 1;
            }
        }
        else
            doAiReset();
    };
    auto doAiDown = [&]()
    {
        som = 2;
        if (m_playerTanks[m_selectedBox].type > 0 && m_playerTanks[m_selectedBox].ai > m_minAI)
        {
            m_playerTanks[m_selectedBox].ai--;
            m_creditsUsed -= 2;
            som = 1;
        }
    };
    auto doBuy = [&]()
    {
        som = 2;
        if (m_playerTanks[m_selectedBox].type == 0)
        {
            int eval = m_creditsUsed + TANK_COST[m_selectedTankType];
            if (eval <= m_creditsMax)
            {
                som = 1;
                m_playerTanks[m_selectedBox].type = m_selectedTankType;
                m_playerTanks[m_selectedBox].ai = m_minAI;
                m_creditsUsed += TANK_COST[m_selectedTankType];
            }
        }
    };
    auto doSell = [&]()
    {
        som = 2;
        if (m_playerTanks[m_selectedBox].type > 0)
        {
            m_creditsUsed -= TANK_COST[m_playerTanks[m_selectedBox].type];
            m_creditsUsed -= (m_playerTanks[m_selectedBox].ai - m_minAI) * 2;
            m_playerTanks[m_selectedBox].type = 0;
            m_playerTanks[m_selectedBox].ai = 0;
            if (m_commander == m_selectedBox)
                m_commander = 0;
            som = 1;
        }
    };
    auto doCommander = [&]()
    {
        som = 2;
        if (m_playerTanks[m_selectedBox].type > 0)
        {
            m_commander = m_selectedBox;
            som = 1;
        }
    };
    auto doBack = [&]()
    {
        m_screen = MenuScreen::SELECT_LEVEL_SQUAD;
        m_commander = 0;
        som = 1;
        for (int i = 0; i < 13; i++)
        {
            m_playerTanks[i] = PlayerTankInfo{};
        }
    };
    auto doBattle = [&]()
    {
        som = 2;
        if (m_creditsUsed > 0)
        {
            som = 1;
            finishMenu();
        }
    };

    // === Мышь ===
    if (clicked)
    {
        // Box selection - left/right arrows
        if (inBox(mx, my, 190, 200, 210, 220))
            doBoxLeft();
        if (inBox(mx, my, 430, 200, 450, 220))
            doBoxRight();

        // Intuition box selection (click directly on box row)
        if (my > 222 && my < 247)
        {
            int newBox = 1 + (int)(mx / 54);
            if (newBox >= 1 && newBox <= 12)
            {
                m_selectedBox = newBox;
                som = 1;
            }
        }

        // Tank selection - left/right arrows
        if (inBox(mx, my, 32, 398, 52, 418))
            doTankLeft();
        if (inBox(mx, my, 187, 398, 207, 418))
            doTankRight();

        // Buy button
        if (inBox(mx, my, 70, 259, 170, 289))
            doBuy();

        // Buy intuition on tank (click on preview area)
        if (inBox(mx, my, 30, 300, 210, 395))
            doBuy();

        // AI decrease
        if (inBox(mx, my, 430, 275, 450, 295))
            doAiDown();

        // AI increase
        if (inBox(mx, my, 590, 275, 610, 295))
            doAiUp();

        // Sell
        if (inBox(mx, my, 488, 400, 543, 413))
            doSell();

        // Back
        if (inBox(mx, my, 70, 435, 170, 465))
            doBack();

        // Make commander
        if (inBox(mx, my, 430, 421, 610, 434))
            doCommander();

        // Go battle
        if (inBox(mx, my, 470, 435, 570, 465))
            doBattle();
    }

    // === Геймпад / Клавиатура ===
    if (m_input->isMenuLeftPressed())
        doBoxLeft();
    if (m_input->isMenuRightPressed())
        doBoxRight();
    if (m_input->isMenuUpPressed())
        doTankLeft();
    if (m_input->isMenuDownPressed())
        doTankRight();
    if (m_input->isMenuConfirmPressed())
        doBuy();
    if (m_input->isMenuCancelPressed())
        doSell();
    if (m_input->isMenuSpecial1Pressed())
        doCommander();
    if (m_input->isMenuSpecial2Pressed())
        doAiUp();
    if (m_input->isMenuBackPressed())
        doBack();
    if (m_input->isMenuNextPressed())
        doBattle();

    if (som == 1)
        m_audioSystem->playMenuClick();
    if (som == 2)
        m_audioSystem->playMenuCancel();
}

void MenuSystem::finishMenu()
{
    m_audioSystem->stopMusic();

    // Setting oppo and guest (from original)
    int oppn = ((m_freeSquads - 1) * 100) + 99;

    int enemySquad;
    do
    {
        enemySquad = 1 + GetRandomValue(0, oppn - 1) / 100;
    } while (enemySquad == m_playerSquad);

    int guestSquad;
    do
    {
        guestSquad = 1 + GetRandomValue(0, 899) / 100;
    } while (guestSquad == enemySquad || guestSquad == m_playerSquad);

    m_result.level = m_selectedLevel;
    m_result.playerSquad = m_playerSquad;
    m_result.enemySquad = enemySquad;
    m_result.guestSquad = guestSquad;
    m_result.commander = m_commander;
    std::memcpy(m_result.player, m_playerTanks, sizeof(m_playerTanks));

    m_finished = true;
}

// ============================================================
// Draw
// ============================================================

void MenuSystem::draw()
{
    // Render tank previews to textures first
    if (m_screen == MenuScreen::SHOP)
    {
        renderTankPreviews();
    }

    BeginDrawing();

    drawBackground3D();

    if (m_screen == MenuScreen::SELECT_LEVEL_SQUAD)
    {
        drawSelectLevelSquad();
    }
    else
    {
        drawShop();
    }

    drawLogo();

    if (m_input->isMouseEnabled())
    {
        Vector2 mouse = m_input->getMousePosition();
        drawCursor(mouse.x - m_offsetX, mouse.y - m_offsetY);
    }

    // Fade in overlay (analog of set gamma)
    if (m_gamma < 255.0f)
    {
        unsigned char alpha = (unsigned char)(255 - m_gamma);
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{0, 0, 0, alpha});
    }

    EndDrawing();
}

void MenuSystem::drawBackground3D()
{
    ClearBackground(m_terrain->getBackdropColor());
    BeginMode3D(m_camera->getCamera());
    m_camera->applyRange();
    m_skybox->render();
    m_terrain->render();
    m_treeSystem->render();
    m_cloudSystem->render();
    EndMode3D();
}

void MenuSystem::drawSelectLevelSquad()
{
    // slevel.bmp at (0, 200)
    drawImage(m_texSLevel, 0, 200);
    // ssquad.bmp at (0, 250)
    drawImage(m_texSSquad, 0, 250);

    // Level number (two digits at 375,200 and 385,200)
    drawDigits(m_selectedLevel, 375, 200);

    // Arrows
    drawImage(m_texArrowLeft, 190, 200);
    drawImage(m_texArrowLeft, 190, 250);
    drawImage(m_texArrowRight, 430, 200);
    drawImage(m_texArrowRight, 430, 250);

    // Squad info
    drawImage(m_texSquads[m_playerSquad], 169, 300);

    // Next button (p1.bmp)
    drawImage(m_texButtons[1], 270, 440);

    // Locked indicator (p20.bmp with alpha 125)
    if (m_frepSquads < m_playerSquad)
    {
        drawImageAlpha(m_texButtons[20], 170, 301, 125);
    }
}

void MenuSystem::drawShop()
{
    // === Left panel (tank to buy) ===
    // Draw tank previews (from render textures, flipped vertically)
    {
        Rectangle src = {0, 0, 180, -120};
        Vector2 dest = {30 + m_offsetX, 300 + m_offsetY};
        DrawTextureRec(m_previewLeft.texture, src, dest, WHITE);
    }
    DrawRectangle(30 + m_offsetX, 300 + m_offsetY, 180, 20, Color{0, 0, 0, 255});
    drawImage(m_texButtons[6], 32, 302); // p6 - "shop"
    drawImage(m_texTankNames[m_selectedTankType], 98, 302);

    // Cost
    drawDigits(TANK_COST[m_selectedTankType], 185, 300);
    drawImage(m_texButtons[11], 165, 302); // p11 - "cr"

    // Tank selection label
    drawImage(m_texButtons[8], 32, 398); // p8 - "select tank"

    drawImage(m_texButtons[7], 30, 300); // p7 - camera background

    // === Right panel (tank in box) ===
    // Draw tank previews (from render textures, flipped vertically)
    if (m_playerTanks[m_selectedBox].type > 0)
    {
        Rectangle src = {0, 0, 180, -120};
        Vector2 dest = {430 + m_offsetX, 300 + m_offsetY};
        DrawTextureRec(m_previewRight.texture, src, dest, WHITE);
        DrawRectangle(430 + m_offsetX, 300 + m_offsetY, 180, 20, Color{0, 0, 0, 255});
    }
    else
        DrawRectangle(430 + m_offsetX, 300 + m_offsetY, 180, 120, Color{0, 0, 0, 255});
    drawImage(m_texButtons[12], 430, 275); // p12 - aibar background

    if (m_playerTanks[m_selectedBox].type > 0)
    {
        int boxType = m_playerTanks[m_selectedBox].type;
        drawImage(m_texTankNames[boxType], 498, 302);
        drawImage(m_texButtons[11], 565, 302); // "cr"
        drawDigits(TANK_COST[boxType], 585, 300);

        // AI bar
        for (int t = 1; t <= m_playerTanks[m_selectedBox].ai; t++)
        {
            int im = 14; // p14 - green ai
            if (t <= m_minAI)
                im = 13; // p13 - grensc ai
            drawImage(m_texButtons[im], 453 + (t - 1) * 13, 279);
        }
        for (int t = m_playerTanks[m_selectedBox].ai + 1; t <= m_maxAI; t++)
        {
            drawImage(m_texButtons[15], 453 + (t - 1) * 13, 279); // p15 - black ai
        }
    }

    drawImage(m_texButtons[5], 432, 302);  // p5 - "in box"
    drawImage(m_texButtons[9], 432, 398);  // p9 - "sell tank"
    drawImage(m_texButtons[16], 430, 260); // p16 - "ai select"
    drawImage(m_texButtons[17], 430, 421); // p17 - "make commander"
    drawImage(m_texButtons[7], 430, 300);  // camera background

    // === Boxes (12 slots) ===
    for (int b = 1; b <= 12; b++)
    {
        Texture2D boxTex = (b == m_selectedBox) ? m_texBoxS : m_texBox;
        drawImage(boxTex, 5 + (b - 1) * 53, 222);

        if (m_playerTanks[b].type > 0)
        {
            // Small tank image (tasqa/tasqb/... per squad)
            drawImage(m_texTankSmall[m_playerSquad][m_playerTanks[b].type], 6 + (b - 1) * 53, 223);
        }

        if (m_commander == b)
        {
            drawImage(m_texButtons[18], 16 + (b - 1) * 53, 223); // p18 - commander mark
        }
    }

    // "Make your squad" label (p19)
    drawImage(m_texButtons[19], 220, 165);

    // Select box background
    drawImage(m_texBoxSel, 0, 200);

    // Box number
    drawDigits(m_selectedBox, 337, 200);

    // Enemy tanks count
    int enemyCount = spec[m_selectedLevel - 1].numEnemy + 1;
    drawDigits(enemyCount, 605, 200);

    // Arrows for box selection
    drawImage(m_texArrowLeft, 190, 200);
    drawImage(m_texArrowRight, 430, 200);

    // Back button (p2)
    drawImage(m_texButtons[2], 70, 435);

    // Buy button (p10)
    drawImage(m_texButtons[10], 70, 259);

    // Battle button (p3)
    drawImage(m_texButtons[3], 470, 435);

    // Credits
    drawDigits(m_creditsMax, 95, 200);
    drawDigits(m_creditsMax - m_creditsUsed, 55, 200);
}

void MenuSystem::drawLogo()
{
    // sprite 1,320,95,10001 / scale 70+45*sin(ang2#) / rotate 10*cos(ang#)
    float scale = 70.0f + 45.0f * sinf(m_logoAngle2 * DEG2RAD);
    float rotation = 10.0f * cosf(m_logoAngle * DEG2RAD);

    Rectangle src = {0, 0, (float)m_texLogo.width, (float)m_texLogo.height};
    Rectangle dest = {320 + m_offsetX, 95 + m_offsetY,
                      (float)m_texLogo.width * scale / 100.0f,
                      (float)m_texLogo.height * scale / 100.0f};
    Vector2 origin = {dest.width / 2.0f, dest.height / 2.0f};

    DrawTexturePro(m_texLogo, src, dest, origin, rotation, WHITE);
}

void MenuSystem::drawCursor(float mx, float my)
{
    drawImage(m_texCursor, mx, my);
}

void MenuSystem::renderTankPreviews()
{
    // Camera orbit (from original: xc2#=46*cos(tur#), zc2#=47*sin(tur#), yc2#=4*sin(tur#))
    float xc2 = 46.0f * cosf(m_tankRotation * DEG2RAD);
    float zc2 = 47.0f * sinf(m_tankRotation * DEG2RAD);
    float yc2 = 4.0f * sinf(m_tankRotation * DEG2RAD);

    Texture2D squadTex = m_tankSystem->getSquadTexture(m_playerSquad);

    // === Left preview (tank to buy) ===
    BeginTextureMode(m_previewLeft);
    ClearBackground(BLACK);

    Camera3D camLeft = {};
    camLeft.position = {xc2, 60.0f + yc2, -zc2};
    camLeft.target = {0, 10, 0};
    camLeft.up = {0, 1, 0};
    camLeft.fovy = 45.0f;
    camLeft.projection = CAMERA_PERSPECTIVE;

    BeginMode3D(camLeft);
    if (m_previewModelsLoaded[m_selectedTankType])
    {
        // Set squad texture on preview model
        for (int j = 0; j < m_previewModels[m_selectedTankType].materialCount; j++)
        {
            m_previewModels[m_selectedTankType].materials[j].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
            m_previewModels[m_selectedTankType].materials[j].maps[MATERIAL_MAP_DIFFUSE].texture = squadTex;
        }
        Vector3 sc = {TANK_SCALE_X[m_selectedTankType], TANK_SCALE_Y[m_selectedTankType], TANK_SCALE_Z[m_selectedTankType]};
        DrawModelEx(m_previewModels[m_selectedTankType], {0, 0, 0}, {0, 1, 0}, 0.0f, sc, WHITE);
    }
    EndMode3D();
    EndTextureMode();

    // === Right preview (tank in box) ===
    if (m_playerTanks[m_selectedBox].type > 0)
    {
        int boxType = m_playerTanks[m_selectedBox].type;

        BeginTextureMode(m_previewRight);
        ClearBackground(BLACK);

        Camera3D camRight = {};
        camRight.position = {xc2, 60.0f + yc2, zc2};
        camRight.target = {0, 10, 0};
        camRight.up = {0, 1, 0};
        camRight.fovy = 45.0f;
        camRight.projection = CAMERA_PERSPECTIVE;

        BeginMode3D(camRight);
        if (m_previewModelsLoaded[boxType])
        {
            // Set squad texture on preview model
            for (int j = 0; j < m_previewModels[boxType].materialCount; j++)
            {
                m_previewModels[boxType].materials[j].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
                m_previewModels[boxType].materials[j].maps[MATERIAL_MAP_DIFFUSE].texture = squadTex;
            }
            Vector3 sc = {TANK_SCALE_X[boxType], TANK_SCALE_Y[boxType], TANK_SCALE_Z[boxType]};
            DrawModelEx(m_previewModels[boxType], {0, 0, 0}, {0, 1, 0}, 0.0f, sc, WHITE);
        }
        EndMode3D();
        EndTextureMode();
    }
}

// ============================================================
// Helpers
// ============================================================

bool MenuSystem::inBox(float mx, float my, float x1, float y1, float x2, float y2) const
{
    return mx > x1 && mx < x2 && my > y1 && my < y2;
}

void MenuSystem::drawImage(Texture2D tex, float x, float y) const
{
    if (tex.id != 0)
    {
        DrawTexture(tex, (int)(x + m_offsetX), (int)(y + m_offsetY), WHITE);
    }
}

void MenuSystem::drawImageAlpha(Texture2D tex, float x, float y, unsigned char alpha) const
{
    if (tex.id != 0)
    {
        DrawTexture(tex, (int)(x + m_offsetX), (int)(y + m_offsetY), Color{255, 255, 255, alpha});
    }
}

void MenuSystem::drawDigits(int value, float x, float y) const
{
    if (value < 0)
        value = 0;
    if (value > 999)
        value = 999;

    int hundreds = value / 100;
    int tens = (value % 100) / 10;
    int units = value % 10;

    float nx = x;

    if (hundreds > 0 && m_texDigits[hundreds].id != 0)
    {
        drawImage(m_texDigits[hundreds], nx, y);
        nx += 10.0f;
    }

    if (m_texDigits[tens].id != 0)
    {
        drawImage(m_texDigits[tens], nx, y);
        nx += 10.0f;
    }

    if (m_texDigits[units].id != 0)
    {
        drawImage(m_texDigits[units], nx, y);
    }
}

// ============================================================
// Assets
// ============================================================

void MenuSystem::loadAssets()
{
    m_texLogo = LoadTextureColorKey("data/menu/title2.png");
    m_texCursor = LoadTextureColorKey("data/menu/mouse.png");
    m_texSLevel = LoadTexture("data/menu/slevel.png");
    m_texSSquad = LoadTexture("data/menu/ssquad.png");
    m_texArrowLeft = LoadTexture("data/menu/ar1.png");
    m_texArrowRight = LoadTexture("data/menu/ar2.png");
    m_texBoxSel = LoadTexture("data/menu/boxsel.png");
    m_texBox = LoadTexture("data/menu/box.png");
    m_texBoxS = LoadTexture("data/menu/boxs.png");

    for (int i = 0; i < 10; i++)
    {
        m_texDigits[i] = LoadTextureColorKey(TextFormat("data/menu/dg%d.png", i));
    }
    for (int i = 1; i <= 10; i++)
    {
        m_texSquads[i] = LoadTexture(TextFormat("data/menu/sq%d.png", i));
    }
    for (int i = 1; i <= 20; i++)
    {
        if (i == 7 || i == 18 || i == 19)
            m_texButtons[i] = LoadTextureColorKey(TextFormat("data/menu/p%d.png", i));
        else
            m_texButtons[i] = LoadTexture(TextFormat("data/menu/p%d.png", i));
    }
    for (int i = 1; i <= 8; i++)
    {
        m_texTankNames[i] = LoadTexture(TextFormat("data/menu/no%d.png", i));
    }

    // Small tank images for boxes (scr/tasqa..tasqj)
    const char *letters[] = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j"};
    for (int squad = 1; squad <= 10; squad++)
    {
        for (int type = 1; type <= 8; type++)
        {
            m_texTankSmall[squad][type] = LoadTextureColorKey(TextFormat("data/menu/tasq%s%d.png", letters[squad - 1], type));
        }
    }
}

void MenuSystem::unloadAssets()
{
    UnloadTexture(m_texLogo);
    UnloadTexture(m_texCursor);
    UnloadTexture(m_texSLevel);
    UnloadTexture(m_texSSquad);
    UnloadTexture(m_texArrowLeft);
    UnloadTexture(m_texArrowRight);
    UnloadTexture(m_texBoxSel);
    UnloadTexture(m_texBox);
    UnloadTexture(m_texBoxS);
    for (int i = 0; i < 10; i++)
        UnloadTexture(m_texDigits[i]);
    for (int i = 1; i <= 10; i++)
        UnloadTexture(m_texSquads[i]);
    for (int i = 1; i <= 20; i++)
        UnloadTexture(m_texButtons[i]);
    for (int i = 1; i <= 8; i++)
        UnloadTexture(m_texTankNames[i]);
    for (int s = 1; s <= 10; s++)
        for (int t = 1; t <= 8; t++)
            UnloadTexture(m_texTankSmall[s][t]);
}

void MenuSystem::loadPreviewModels()
{
    for (int i = 1; i <= 8; i++)
    {
        const char *path = TextFormat("data/tanks/t%d.glb", i);
        if (FileExists(path))
        {
            m_previewModels[i] = LoadModel(path);
            m_previewModelsLoaded[i] = true;
        }
    }
}

void MenuSystem::unloadPreviewModels()
{
    for (int i = 1; i <= 8; i++)
    {
        if (m_previewModelsLoaded[i])
        {
            UnloadModel(m_previewModels[i]);
            m_previewModelsLoaded[i] = false;
        }
    }
}

bool MenuSystem::gamepadPressed(std::initializer_list<int> buttons) const
{
    if (!m_input || !m_input->isGamepadAvailable())
        return false;
    return m_input->isGamepadAnyPressed(buttons);
}