#include "ConfigSystem.h"
#include <fstream>
#include <iostream>

bool ConfigSystem::loadSettings(const std::string &path)
{
    settingsPath = path;

    std::ifstream file(path);
    if (!file.is_open())
    {
        TraceLog(LOG_WARNING, "Settings file not found, creating default: %s", path.c_str());
        createDefaultSettings();
        saveSettings(path);
        return false;
    }

    try
    {
        json j;
        file >> j;

        // Display settings
        if (j.contains("display"))
        {
            auto &disp = j["display"];
            displayConfig.width = disp.value("width", 1280);
            displayConfig.height = disp.value("height", 720);
            displayConfig.fullscreen = disp.value("fullscreen", false);
            displayConfig.vsync = disp.value("vsync", false);
            displayConfig.targetFPS = disp.value("targetFPS", 60);
        }

        // Gameplay settings
        if (j.contains("gameplay"))
        {
            auto &game = j["gameplay"];
            gameplayConfig.showDebug = game.value("showDebug", true);
            gameplayConfig.showEnemyIDs = game.value("showEnemyIDs", false);
            gameplayConfig.musicEnabled = game.value("musicEnabled", true);
            gameplayConfig.sound3DEnabled = game.value("soundEnabled", true);
        }

        TraceLog(LOG_INFO, "Settings loaded from %s", path.c_str());
        return true;
    }
    catch (const json::exception &e)
    {
        TraceLog(LOG_ERROR, "Failed to parse settings JSON: %s", e.what());
        return false;
    }
}

void ConfigSystem::saveSettings(const std::string &path)
{
    json j;

    j["display"] = {
        {"width", displayConfig.width},
        {"height", displayConfig.height},
        {"fullscreen", displayConfig.fullscreen},
        {"vsync", displayConfig.vsync},
        {"targetFPS", displayConfig.targetFPS}};

    j["gameplay"] = {
        {"showDebug", gameplayConfig.showDebug},
        {"showEnemyIDs", gameplayConfig.showEnemyIDs},
        {"musicEnabled", gameplayConfig.musicEnabled},
        {"sound3DEnabled", gameplayConfig.sound3DEnabled}};

    std::ofstream file(path);
    if (file.is_open())
    {
        file << j.dump(4);
        TraceLog(LOG_INFO, "Settings saved to %s", path.c_str());
    }
    else
    {
        TraceLog(LOG_WARNING, "Failed to save settings to %s", path.c_str());
    }
}

void ConfigSystem::setShowDebug(bool value)
{
    gameplayConfig.showDebug = value;
    saveSettings(settingsPath);
}

void ConfigSystem::setShowEnemyIDs(bool value)
{
    gameplayConfig.showEnemyIDs = value;
    saveSettings(settingsPath);
}

void ConfigSystem::setMusicEnabled(bool value)
{
    gameplayConfig.musicEnabled = value;
    saveSettings(settingsPath);
}

void ConfigSystem::setSound3DEnabled(bool value)
{
    gameplayConfig.sound3DEnabled = value;
    saveSettings(settingsPath);
}

void ConfigSystem::createDefaultSettings()
{
    displayConfig.width = 1280;
    displayConfig.height = 720;
    displayConfig.fullscreen = false;
    displayConfig.vsync = false;
    displayConfig.targetFPS = 60;

    gameplayConfig.showDebug = true;
    gameplayConfig.showEnemyIDs = false;
    gameplayConfig.musicEnabled = true;
    gameplayConfig.sound3DEnabled = true;
}