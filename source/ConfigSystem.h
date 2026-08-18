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
    
    // Сеттеры для настроек (с сохранением)
    void setShowDebug(bool value);
    void setShowEnemyIDs(bool value);
    void setMusicEnabled(bool value);
    void setSound3DEnabled(bool value);

private:
    ConfigSystem() = default;
    ~ConfigSystem() = default;
    ConfigSystem(const ConfigSystem &) = delete;
    ConfigSystem &operator=(const ConfigSystem &) = delete;

    DisplayConfig displayConfig;
    GameplayConfig gameplayConfig;

    std::string settingsPath;

    void createDefaultSettings();
};