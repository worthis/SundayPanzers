#include "ConfigSystem.h"
#include "InputMappings.h"
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

    // Загрузка input настроек
    if (j.contains("input"))
    {
        auto &input = j["input"];

        auto loadBinding = [&](const std::string &name, KeyBinding &binding)
        {
            if (!input.contains(name))
                return;

            auto &obj = input[name];

            // Загрузка клавиатуры
            if (obj.contains("keyboard") && obj["keyboard"].is_array())
            {
                for (const auto &keyName : obj["keyboard"])
                {
                    int code = stringToKey(keyName.get<std::string>());
                    if (code != KEY_NULL)
                    {
                        binding.keyboard.push_back(code);
                    }
                }
            }

            // Загрузка мыши
            if (obj.contains("mouse") && obj["mouse"].is_array())
            {
                for (const auto &btnName : obj["mouse"])
                {
                    int code = stringToMouseButton(btnName.get<std::string>());
                    if (code != -1)
                    {
                        binding.mouse.push_back(code);
                    }
                }
            }

            // Загрузка геймпада
            if (obj.contains("gamepad") && obj["gamepad"].is_array())
            {
                for (const auto &btnName : obj["gamepad"])
                {
                    int code = stringToGamepadButton(btnName.get<std::string>());
                    if (code != -1)
                    {
                        binding.gamepad.push_back(code);
                    }
                }
            }
        };

        loadBinding("tankForward", inputConfig.tankForward);
        loadBinding("tankBackward", inputConfig.tankBackward);
        loadBinding("tankLeft", inputConfig.tankLeft);
        loadBinding("tankRight", inputConfig.tankRight);

        loadBinding("fire", inputConfig.fire);
        loadBinding("turbo", inputConfig.turbo);
        loadBinding("rearView", inputConfig.rearView);
        loadBinding("nextTank", inputConfig.nextTank);
        loadBinding("prevTank", inputConfig.prevTank);
        loadBinding("toggleId", inputConfig.toggleId);

        loadBinding("menuUp", inputConfig.menuUp);
        loadBinding("menuDown", inputConfig.menuDown);
        loadBinding("menuLeft", inputConfig.menuLeft);
        loadBinding("menuRight", inputConfig.menuRight);
        loadBinding("menuConfirm", inputConfig.menuConfirm);
        loadBinding("menuCancel", inputConfig.menuCancel);
        loadBinding("menuSpecial1", inputConfig.menuSpecial1);
        loadBinding("menuSpecial2", inputConfig.menuSpecial2);
        loadBinding("menuNext", inputConfig.menuNext);
        loadBinding("menuFire", inputConfig.menuFire);

        loadBinding("quit", inputConfig.quit);
    }

    TraceLog(LOG_INFO, "Settings loaded from %s", path.c_str());
    return true;
}

void ConfigSystem::saveSettings(const std::string &path)
{
    auto saveBinding = [&](const KeyBinding &binding) -> json
    {
        json obj;

        // Сохранение клавиатуры
        if (binding.hasKeyboard())
        {
            json keys = json::array();
            for (int code : binding.keyboard)
            {
                keys.push_back(keyToString(code));
            }
            obj["keyboard"] = keys;
        }

        // Сохранение мыши
        if (binding.hasMouse())
        {
            json buttons = json::array();
            for (int code : binding.mouse)
            {
                buttons.push_back(mouseButtonToString(code));
            }
            obj["mouse"] = buttons;
        }

        // Сохранение геймпада
        if (binding.hasGamepad())
        {
            json buttons = json::array();
            for (int code : binding.gamepad)
            {
                buttons.push_back(gamepadButtonToString(code));
            }
            obj["gamepad"] = buttons;
        }

        return obj;
    };

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

    j["input"] = {
        {"tankForward", saveBinding(inputConfig.tankForward)},
        {"tankBackward", saveBinding(inputConfig.tankBackward)},
        {"tankLeft", saveBinding(inputConfig.tankLeft)},
        {"tankRight", saveBinding(inputConfig.tankRight)},

        {"fire", saveBinding(inputConfig.fire)},
        {"turbo", saveBinding(inputConfig.turbo)},
        {"rearView", saveBinding(inputConfig.rearView)},
        {"nextTank", saveBinding(inputConfig.nextTank)},
        {"prevTank", saveBinding(inputConfig.prevTank)},
        {"toggleId", saveBinding(inputConfig.toggleId)},

        {"menuUp", saveBinding(inputConfig.menuUp)},
        {"menuDown", saveBinding(inputConfig.menuDown)},
        {"menuLeft", saveBinding(inputConfig.menuLeft)},
        {"menuRight", saveBinding(inputConfig.menuRight)},
        {"menuConfirm", saveBinding(inputConfig.menuConfirm)},
        {"menuCancel", saveBinding(inputConfig.menuCancel)},
        {"menuSpecial1", saveBinding(inputConfig.menuSpecial1)},
        {"menuSpecial2", saveBinding(inputConfig.menuSpecial2)},
        {"menuNext", saveBinding(inputConfig.menuNext)},
        {"menuFire", saveBinding(inputConfig.menuFire)},

        {"quit", saveBinding(inputConfig.quit)}};

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
    // Display settings
    displayConfig.width = 1280;
    displayConfig.height = 720;
    displayConfig.fullscreen = false;
    displayConfig.vsync = false;
    displayConfig.targetFPS = 60;

    // Gameplay settings
    gameplayConfig.showDebug = true;
    gameplayConfig.showEnemyIDs = false;
    gameplayConfig.musicEnabled = true;
    gameplayConfig.sound3DEnabled = true;

    // Input settings с множественными привязками
    inputConfig.tankForward.keyboard = {KEY_W, KEY_UP};
    inputConfig.tankForward.gamepad = {GAMEPAD_BUTTON_LEFT_FACE_UP};

    inputConfig.tankBackward.keyboard = {KEY_S, KEY_DOWN};
    inputConfig.tankBackward.gamepad = {GAMEPAD_BUTTON_LEFT_FACE_DOWN};

    inputConfig.tankLeft.keyboard = {KEY_A, KEY_LEFT};
    inputConfig.tankLeft.gamepad = {GAMEPAD_BUTTON_LEFT_FACE_LEFT};

    inputConfig.tankRight.keyboard = {KEY_D, KEY_RIGHT};
    inputConfig.tankRight.gamepad = {GAMEPAD_BUTTON_LEFT_FACE_RIGHT};

    inputConfig.fire.keyboard = {KEY_SPACE};
    inputConfig.fire.mouse = {MOUSE_BUTTON_LEFT};
    inputConfig.fire.gamepad = {GAMEPAD_BUTTON_RIGHT_FACE_RIGHT}; // A

    inputConfig.turbo.keyboard = {KEY_LEFT_CONTROL, KEY_RIGHT_CONTROL};
    inputConfig.turbo.mouse = {MOUSE_BUTTON_RIGHT};
    inputConfig.turbo.gamepad = {GAMEPAD_BUTTON_RIGHT_FACE_DOWN}; // B

    inputConfig.rearView.keyboard = {KEY_LEFT_ALT, KEY_RIGHT_ALT};
    inputConfig.rearView.gamepad = {GAMEPAD_BUTTON_RIGHT_FACE_UP}; // X

    inputConfig.nextTank.keyboard = {KEY_E};
    inputConfig.nextTank.gamepad = {GAMEPAD_BUTTON_RIGHT_TRIGGER_1}; // R1

    inputConfig.prevTank.keyboard = {KEY_Q};
    inputConfig.prevTank.gamepad = {GAMEPAD_BUTTON_LEFT_TRIGGER_1}; // L1

    inputConfig.toggleId.keyboard = {KEY_T};
    inputConfig.toggleId.gamepad = {GAMEPAD_BUTTON_RIGHT_FACE_LEFT}; // Y

    inputConfig.menuUp.keyboard = {KEY_UP, KEY_W};
    inputConfig.menuUp.gamepad = {GAMEPAD_BUTTON_LEFT_FACE_UP, GAMEPAD_BUTTON_LEFT_TRIGGER_1};

    inputConfig.menuDown.keyboard = {KEY_DOWN, KEY_S};
    inputConfig.menuDown.gamepad = {GAMEPAD_BUTTON_LEFT_FACE_DOWN, GAMEPAD_BUTTON_RIGHT_TRIGGER_1};

    inputConfig.menuLeft.keyboard = {KEY_LEFT, KEY_A};
    inputConfig.menuLeft.gamepad = {GAMEPAD_BUTTON_LEFT_FACE_LEFT, GAMEPAD_BUTTON_LEFT_TRIGGER_2};

    inputConfig.menuRight.keyboard = {KEY_RIGHT, KEY_D};
    inputConfig.menuRight.gamepad = {GAMEPAD_BUTTON_LEFT_FACE_RIGHT, GAMEPAD_BUTTON_RIGHT_TRIGGER_2};

    inputConfig.menuConfirm.keyboard = {KEY_ENTER, KEY_SPACE};
    inputConfig.menuConfirm.gamepad = {GAMEPAD_BUTTON_RIGHT_FACE_RIGHT}; // A

    inputConfig.menuCancel.keyboard = {KEY_BACKSPACE};
    inputConfig.menuCancel.gamepad = {GAMEPAD_BUTTON_RIGHT_FACE_DOWN}; // B

    inputConfig.menuSpecial1.keyboard = {KEY_X};
    inputConfig.menuSpecial1.gamepad = {GAMEPAD_BUTTON_RIGHT_FACE_UP}; // X

    inputConfig.menuSpecial2.keyboard = {KEY_Y};
    inputConfig.menuSpecial2.gamepad = {GAMEPAD_BUTTON_RIGHT_FACE_LEFT}; // Y

    inputConfig.menuNext.keyboard = {KEY_ENTER};
    inputConfig.menuNext.gamepad = {GAMEPAD_BUTTON_MIDDLE_RIGHT}; // Start

    inputConfig.menuFire.keyboard = {KEY_SPACE};
    inputConfig.menuFire.mouse = {MOUSE_BUTTON_LEFT};
    inputConfig.menuFire.gamepad = {GAMEPAD_BUTTON_RIGHT_FACE_RIGHT}; // A

    inputConfig.quit.keyboard = {KEY_ESCAPE};
    inputConfig.quit.gamepad = {GAMEPAD_BUTTON_MIDDLE_LEFT}; // Select
}