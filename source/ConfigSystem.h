#pragma once

#include "raylib.h"
#include "third_party/json.hpp"
#include <string>
#include <map>
#include <vector>

using json = nlohmann::json;

// Структура настроек дисплея
struct DisplayConfig
{
    int width = 1280;
    int height = 720;
    bool fullscreen = false;
    bool vsync = false;
    int targetFPS = 60;
};

// Структура настроек геймплея
struct GameplayConfig
{
    bool showDebug = true;
    bool showEnemyIDs = false;
    bool musicEnabled = true;
    bool sound3DEnabled = true;
};

struct KeyBinding
{
    std::vector<int> keyboard; // коды клавиш (KEY_W, KEY_SPACE и т.д.)
    std::vector<int> mouse;    // коды кнопок мыши (MOUSE_BUTTON_LEFT и т.д.)
    std::vector<int> gamepad;  // коды кнопок геймпада (GAMEPAD_BUTTON_* и т.д.)

    bool hasKeyboard() const { return !keyboard.empty(); }
    bool hasMouse() const { return !mouse.empty(); }
    bool hasGamepad() const { return !gamepad.empty(); }
};

struct InputConfig
{
    // Управление танком
    KeyBinding tankForward;
    KeyBinding tankBackward;
    KeyBinding tankLeft;
    KeyBinding tankRight;

    // Действия в бою
    KeyBinding fire;
    KeyBinding turbo;
    KeyBinding rearView;
    KeyBinding nextTank;
    KeyBinding prevTank;
    KeyBinding toggleId;

    // Меню
    KeyBinding menuUp;
    KeyBinding menuDown;
    KeyBinding menuLeft;
    KeyBinding menuRight;
    KeyBinding menuConfirm;
    KeyBinding menuCancel;
    KeyBinding menuSpecial1;
    KeyBinding menuSpecial2;
    KeyBinding menuNext;
    KeyBinding menuFire;

    // Системные
    KeyBinding quit;
};

// Класс для загрузки и хранения конфигурации
class ConfigSystem
{
public:
    static ConfigSystem &instance()
    {
        static ConfigSystem instance;
        return instance;
    }

    bool loadSettings(const std::string &path);
    void saveSettings(const std::string &path);

    // Геттеры для настроек
    const DisplayConfig &getDisplayConfig() const { return displayConfig; }
    const GameplayConfig &getGameplayConfig() const { return gameplayConfig; }
    const InputConfig &getInputConfig() const { return inputConfig; }

    // Сеттеры для настроек (с сохранением)
    void setShowDebug(bool value);
    void setShowEnemyIDs(bool value);
    void setMusicEnabled(bool value);
    void setSound3DEnabled(bool value);
    void setKeyBinding(const std::string &action, int key);

private:
    ConfigSystem() = default;
    ~ConfigSystem() = default;
    ConfigSystem(const ConfigSystem &) = delete;
    ConfigSystem &operator=(const ConfigSystem &) = delete;

    DisplayConfig displayConfig;
    GameplayConfig gameplayConfig;
    InputConfig inputConfig;

    std::string settingsPath;

    void createDefaultSettings();
};