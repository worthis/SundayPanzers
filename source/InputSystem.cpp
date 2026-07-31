#include "InputSystem.h"
#include <cmath>

InputSystem::InputSystem()
    : moveX(0), moveY(0), lookX(0), lookY(0),
      prevFire(false), prevRear(false), prevTurbo(false),
      prevToggleId(false), prevNext(false), prevPrev(false),
      prevQuit(false), prevMusic(false), prevFps(false)
{
}

float InputSystem::applyDeadzone(float value) const
{
    if (fabsf(value) < DEADZONE)
        return 0.0f;
    return value;
}

void InputSystem::update()
{
    moveX = 0.0f;
    moveY = 0.0f;
    lookX = 0.0f;
    lookY = 0.0f;

    // === Клавиатура ===
    // Движение: WASD / стрелки
    // Инвертировано: A = вправо, D = влево (согласовано с FreeCamera)
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
        moveX += 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
        moveX -= 1.0f;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
        moveY -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
        moveY += 1.0f;

    // === Геймпад (Switch) ===
    if (IsGamepadAvailable(0))
    {
        // Левый стик — движение
        // Инвертировано: X и Y оси на Switch имеют обратные знаки
        float gx = applyDeadzone(GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X));
        float gy = applyDeadzone(GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y));
        moveX -= gx; // инвертировано
        moveY -= gy; // инвертировано (на Switch вверх = +1)

        // Правый стик — камера
        lookX = applyDeadzone(GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X));
        lookY = -applyDeadzone(GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y)); // инвертировано
    }

    // Клампим значения
    if (moveX > 1.0f)
        moveX = 1.0f;
    if (moveX < -1.0f)
        moveX = -1.0f;
    if (moveY > 1.0f)
        moveY = 1.0f;
    if (moveY < -1.0f)
        moveY = -1.0f;
}

float InputSystem::getMoveX() const { return moveX; }
float InputSystem::getMoveY() const { return moveY; }
float InputSystem::getLookX() const { return lookX; }
float InputSystem::getLookY() const { return lookY; }

// === Действия (edge-detection: срабатывают один раз при нажатии) ===

bool InputSystem::isFirePressed() const
{
    bool current = IsKeyDown(KEY_SPACE);
    if (IsGamepadAvailable(0))
    {
        current = current || IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN); // A
    }
    return current;
}

bool InputSystem::isRearViewPressed() const
{
    bool current = IsKeyDown(KEY_RIGHT_CONTROL);
    if (IsGamepadAvailable(0))
    {
        current = current || IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT); // B
    }
    return current;
}

bool InputSystem::isTurboPressed() const
{
    bool current = IsKeyDown(KEY_LEFT_CONTROL);
    if (IsGamepadAvailable(0))
    {
        current = current || IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT); // X
    }
    return current;
}

bool InputSystem::isToggleIdPressed() const
{
    bool current = IsKeyDown(KEY_T);
    if (IsGamepadAvailable(0))
    {
        current = current || IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_UP); // Y
    }
    return current;
}

bool InputSystem::isNextTankPressed() const
{
    bool current = false;
    // F1-F10 на клавиатуре обрабатываются отдельно в main.cpp
    if (IsGamepadAvailable(0))
    {
        current = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1); // R
    }
    return current;
}

bool InputSystem::isPrevTankPressed() const
{
    bool current = false;
    if (IsGamepadAvailable(0))
    {
        current = IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_TRIGGER_1); // L
    }
    return current;
}

bool InputSystem::isQuitPressed() const
{
    bool current = IsKeyDown(KEY_Q);
    if (IsGamepadAvailable(0))
    {
        current = current || IsGamepadButtonDown(0, GAMEPAD_BUTTON_MIDDLE_RIGHT); // Plus
    }
    return current;
}

bool InputSystem::isMusicTogglePressed() const
{
    bool current = IsKeyDown(KEY_M);
    if (IsGamepadAvailable(0))
    {
        current = current || IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_TRIGGER_2); // ZL
    }
    return current;
}

bool InputSystem::isFpsTogglePressed() const
{
    bool current = IsKeyDown(KEY_F);
    if (IsGamepadAvailable(0))
    {
        current = current || IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_2); // ZR
    }
    return current;
}

bool InputSystem::isGamepadConnected() const
{
    return IsGamepadAvailable(0);
}