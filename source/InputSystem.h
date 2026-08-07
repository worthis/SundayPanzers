#pragma once
#include "raylib.h"
#include <initializer_list>

class InputSystem
{
public:
    InputSystem();

    void update();

    // Управление танком (для боя)
    float getTankX() const;
    float getTankY() const;
    bool isFirePressed() const;
    bool isTurboPressed() const;
    bool isRearViewPressed() const;

    // Геймпад
    bool isGamepadButtonDown(int button) const;
    bool isGamepadButtonJustPressed(int button) const;
    bool isGamepadAnyPressed(std::initializer_list<int> buttons) const;
    bool isGamepadAvailable() const;

    // === Камера (стрелки / D-pad) ===
    float getCamX() const;
    float getCamY() const;

    // === Обзор (мышь / правый стик) ===
    float getLookX() const;
    float getLookY() const;

    // === Действия ===
    bool isToggleIdPressed() const;
    bool isNextTankPressed() const;
    bool isPrevTankPressed() const;
    bool isQuitPressed() const;
    bool isMusicTogglePressed() const;
    bool isFpsTogglePressed() const;

    bool isGamepadConnected() const;

private:
    float tankX, tankY;
    float camX, camY;
    float lookX, lookY;

    static constexpr float DEADZONE = 0.15f;
    float applyDeadzone(float value) const;

    static constexpr int MAX_GAMEPAD_BUTTONS = 32;

    bool m_gamepadDown[MAX_GAMEPAD_BUTTONS] = {};     // текущее состояние
    bool m_gamepadPrevDown[MAX_GAMEPAD_BUTTONS] = {}; // предыдущий кадр

    void updateGamepadState();
};