#pragma once
#include "raylib.h"

class InputSystem
{
public:
    InputSystem();

    void update();

    // === Танк (WASD / левый стик) ===
    float getTankX() const; // +1 = поворот вправо
    float getTankY() const; // -1 = вперёд, +1 = назад

    // === Камера (стрелки / D-pad) ===
    float getCamX() const;
    float getCamY() const;

    // === Обзор (мышь / правый стик) ===
    float getLookX() const;
    float getLookY() const;

    // === Действия ===
    bool isFirePressed() const;
    bool isRearViewPressed() const;
    bool isTurboPressed() const;
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
};