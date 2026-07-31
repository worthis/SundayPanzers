#pragma once
#include "raylib.h"

class InputSystem
{
public:
    InputSystem();

    // Вызывать каждый кадр перед использованием
    void update();

    // === Движение (аналог стрелок/WASD или левого стика) ===
    float getMoveX() const; // -1..1 (влево..вправо)
    float getMoveY() const; // -1..1 (назад..вперёд)

    // === Камера (аналог мыши или правого стика) ===
    float getLookX() const; // -1..1
    float getLookY() const; // -1..1

    // === Действия ===
    bool isFirePressed() const;        // Пробел / A
    bool isRearViewPressed() const;    // Правый Ctrl / B
    bool isTurboPressed() const;       // Левый Ctrl / X
    bool isToggleIdPressed() const;    // T / Y
    bool isNextTankPressed() const;    // F1-F10 / R
    bool isPrevTankPressed() const;    // - / L
    bool isQuitPressed() const;        // Q / Plus
    bool isMusicTogglePressed() const; // M / ZL
    bool isFpsTogglePressed() const;   // F / ZR

    // === Служебное ===
    bool isGamepadConnected() const;

private:
    float moveX, moveY;
    float lookX, lookY;

    // Состояния кнопок (для edge-detection)
    bool prevFire, prevRear, prevTurbo, prevToggleId;
    bool prevNext, prevPrev, prevQuit, prevMusic, prevFps;

    static constexpr float DEADZONE = 0.15f;

    float applyDeadzone(float value) const;
};