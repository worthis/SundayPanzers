#pragma once
#include "raylib.h"
#include "rlgl.h"
#include "GameConfig.h"
#include "AudioSystem.h"
#include "Terrain.h"
#include "Skybox.h"
#include "TreeSystem.h"
#include "CloudSystem.h"
#include "TankSystem.h"
#include "BulletSystem.h"
#include "PowerUpSystem.h"
#include "AISystem.h"
#include "TankCamera.h"
#include "InputSystem.h"
#include "SortieSystem.h"
#include "MenuSystem.h"
#include "HUDSystem.h"
#include "Utils.h"

// Состояния игры (State Machine)
enum class GameState
{
    LOGO_INTRO,   // entra()
    GAME_INTRO,   // gameintro()
    MAIN_MENU,    // menu()
    BATTLE_INTRO, // camintro()
    BATTLE,       // tanks()
    BATTLE_END    // camending()
};

enum class MenuItem
{
    START_GAME,
    QUIT,
    COUNT
};

class Game
{
public:
    Game();
    ~Game();

    void Init();
    void Update(float dt);
    void Draw();
    void Shutdown();

private:
    void loadAssets();
    void unloadAssets();

    // Логика состояний
    void StartLogoIntro();
    void UpdateLogoIntro(float dt);
    void DrawLogoIntro();

    void StartGameIntro();
    void UpdateGameIntro(float dt);
    void DrawGameIntro();

    void StartBattleIntro();
    void UpdateBattleIntro(float dt);
    void DrawBattleIntro();

    void StartBattle(int level, int playerSquad, int enemySquad, int guestSquad, PlayerTankInfo *player, int commander);
    void UpdateBattle(float dt);
    void DrawBattle();
    void CheckBattleEndConditions();

    void StartBattleEnding();
    void UpdateBattleEnding(float dt);
    void DrawBattleEnding();

    void ReturnToMenu();

    void updateEngineSounds();
    float findNearestTankDistance(int &nearestTankId) const;

    void initFakeTank();
    void initFakeTank(float x, float z, float yaw);
    void UpdateFakeTankMovement();

    GameState currentState;
    MenuSystem menuSystem;
    HUDSystem hudSystem;

    // Игровые системы
    AudioSystem audioSystem;
    Terrain terrain;
    Skybox skybox;
    TreeSystem treeSystem;
    CloudSystem cloudSystem;
    TankSystem tankSystem;
    BulletSystem bulletSystem;
    PowerUpSystem powerUpSystem;
    AISystem aiSystem;
    TankCamera camera;
    InputSystem input;

    // Состояние текущего матча
    int playerCommander = 1;
    float accumulator;
    bool showDebug = false;
    bool showEnemyIDs = false;
    bool playerSquadAlive = false;
    bool enemySquadAlive = false;
    bool battleEnded = false;

    // Ассеты и состояние для интро
    float introTimer;
    float introGamma;
    bool logoSoundPlayed = false;
    Texture2D texLogo = {};
    Texture2D texData1 = {};
    Texture2D texData2 = {};
    Texture2D texTitle = {};
    Sound sndLogo = {};
    TankData introFakeTank = {};
    Vector3 introTarget = Vector3Zero();

    // Ассеты и состояние для интро боя
    float battleEndingBounce = 0.0f;    // b# (bounce эффект)
    float battleEndingBounceAcc = 0.0f; // ac# (ускорение bounce)
    bool battleEndingVictory = false;   // результат боя
    Texture2D texScenario[6] = {};      // image 84+gam(25) - имя сценария
    Texture2D texVictory = {};          // image 2001
    Texture2D texDefeat = {};           // image 2002
    Texture2D texClick = {};            // image 2003
    Texture2D texStart = {};
    Texture2D texBattleOver = {};
};