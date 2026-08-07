#pragma once
#include "raylib.h"
#include "GameConfig.h"
#include "InputSystem.h"
#include "Terrain.h"
#include "Skybox.h"
#include "TreeSystem.h"
#include "CloudSystem.h"
#include "TankSystem.h"
#include "TankCamera.h"
#include "SortieSystem.h"
#include "GameData.h"
#include "Utils.h"
#include <cstring>
#include <initializer_list>

struct MenuResult
{
    int level = 1;
    int playerSquad = 1;
    int enemySquad = 1;
    int guestSquad = 1;
    int commander = 0;
    PlayerTankInfo player[13] = {};
};

enum class MenuScreen
{
    SELECT_LEVEL_SQUAD, // mensel=1
    SHOP                // mensel=2/3
};

class MenuSystem
{
public:
    void init(InputSystem *input, Terrain *terrain, Skybox *skybox, TreeSystem *treeSystem,
              CloudSystem *cloudSystem, TankSystem *tankSystem, TankCamera *camera);
    void start(int maxLevel, bool gameCompleted);
    void update(float dt);
    void draw();
    void shutdown();

    bool isFinished() const { return m_finished; }
    MenuResult getResult() const { return m_result; }

private:
    // Update
    void updateFakeTank(float dt);
    void updateSelectLevelSquad(float mx, float my, bool clicked);
    void updateShop(float mx, float my, bool clicked);
    void finishMenu();

    // Draw
    void drawBackground3D();
    void drawSelectLevelSquad();
    void drawShop();
    void drawLogo();
    void drawCursor(float mx, float my);
    void renderTankPreviews();

    // Helpers
    bool inBox(float mx, float my, float x1, float y1, float x2, float y2) const;
    void drawImage(Texture2D tex, float x, float y) const;
    void drawImageAlpha(Texture2D tex, float x, float y, unsigned char alpha) const;
    void drawDigits(int value, float x, float y) const;
    bool gamepadPressed(std::initializer_list<int> buttons) const;

    // Assets
    void loadAssets();
    void unloadAssets();
    void loadPreviewModels();
    void unloadPreviewModels();

    // Systems
    InputSystem *m_input = nullptr;
    Terrain *m_terrain = nullptr;
    Skybox *m_skybox = nullptr;
    TreeSystem *m_treeSystem = nullptr;
    CloudSystem *m_cloudSystem = nullptr;
    TankSystem *m_tankSystem = nullptr;

    // Fake tank (3D background)
    TankData m_fakeTank = {};
    Vector3 m_fakeTarget = {};
    TankCamera *m_camera = nullptr;

    // Menu state
    MenuScreen m_screen = MenuScreen::SELECT_LEVEL_SQUAD;
    int m_maxLevel = 10;
    bool m_gameCompleted = false;
    int m_selectedLevel = 1;
    int m_playerSquad = 1;
    int m_freeSquads = 2;
    int m_frepSquads = 2;

    // Shop state
    int m_selectedTankType = 1; // stt
    int m_selectedBox = 1;      // boxsel
    int m_creditsMax = 0;       // crmax
    int m_creditsUsed = 0;      // crused
    int m_minAI = 1;            // minai
    int m_maxAI = 1;            // maxai
    int m_commander = 0;        // gam(1)
    PlayerTankInfo m_playerTanks[13] = {};

    // Animation
    float m_logoAngle = 0.0f;
    float m_logoAngle2 = 0.0f;
    float m_tankRotation = 0.0f;
    float m_gamma = 0.0f;

    // Result
    bool m_finished = false;
    MenuResult m_result;

    // Centering offset (640x480 -> SCREEN_WIDTH x SCREEN_HEIGHT)
    float m_offsetX = 0.0f;
    float m_offsetY = 0.0f;

    // Assets
    Texture2D m_texLogo = {};
    Texture2D m_texCursor = {};
    Texture2D m_texSLevel = {};
    Texture2D m_texSSquad = {};
    Texture2D m_texArrowLeft = {};
    Texture2D m_texArrowRight = {};
    Texture2D m_texBoxSel = {};
    Texture2D m_texBox = {};
    Texture2D m_texBoxS = {};
    Texture2D m_texDigits[10] = {};
    Texture2D m_texSquads[11] = {};
    Texture2D m_texButtons[21] = {};
    Texture2D m_texTankNames[9] = {};
    Texture2D m_texTankSmall[11][9] = {}; // [squad][type]

    // 3D preview
    RenderTexture2D m_previewLeft = {};
    RenderTexture2D m_previewRight = {};
    Model m_previewModels[9] = {};
    bool m_previewModelsLoaded[9] = {};

    // Sound
    Sound m_sndClick = {};
    Sound m_sndError = {};
    bool m_soundsLoaded = false;

    // Music
    Music m_music = {};
    bool m_musicLoaded = false;

    // Tank prices (cost() from original)
    static constexpr int TANK_COST[9] = {0, 3, 4, 6, 11, 14, 20, 25, 40};

    // Tank scales for preview (from tankloadermenu)
    static constexpr float TANK_SCALE_X[9] = {0, 1.0f, 1.02f, 1.02f, 1.0f, 1.0f, 1.0f, 1.0f, 1.05f};
    static constexpr float TANK_SCALE_Y[9] = {0, 1.1f, 1.05f, 1.05f, 0.98f, 0.95f, 0.99f, 1.01f, 1.01f};
    static constexpr float TANK_SCALE_Z[9] = {0, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.01f, 1.01f, 1.01f};
};