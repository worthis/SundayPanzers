#pragma once
#include "raylib.h"
#include "rlgl.h"
#include "GameConfig.h"
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

class Game
{
public:
    Game();
    ~Game();

    void Init();
    void Update(float dt);
    void Draw();
    void Shutdown();

    // Методы управления потоком игры
    void StartBattle(int level, int playerSquad, int enemySquad, int guestSquad, PlayerTankInfo *player, int commander);
    void ReturnToMenu();

private:
    // Логика состояний
    void UpdateLogoIntro(float dt);
    void DrawLogoIntro();

    void StartGameIntro();
    void UpdateGameIntro(float dt);
    void DrawGameIntro();

    void UpdateBattle(float dt);
    void DrawBattle();
    void CheckBattleEndConditions();

    GameState currentState;

    // Игровые системы (вместо глобальных переменных)
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
    int playerCommander;
    float accumulator;
    bool showDebug;

    // Флаги окончания боя (аналог gam(22) и gam(23) из DBPro)
    bool playerSquadAlive;
    bool enemySquadAlive;
    bool battleEnded;

    // Ассеты и состояние для интро
    float introTimer;
    bool logoSoundPlayed;
    Texture2D texLogo, texData1, texData2, texTitle;
    Sound sndLogo;
    Music musicIntro;

    // Фейковый танк для интро (аналог tk#(0,...) в DBPro)
    TankData introFakeTank;
    Vector3 introTarget;
};