#include "InputSystem.h"
#include <cmath>

InputSystem::InputSystem()
    : tankX(0), tankY(0), camX(0), camY(0), lookX(0), lookY(0)
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
    tankX = 0.0f;
    tankY = 0.0f;
    camX = 0.0f;
    camY = 0.0f;
    lookX = 0.0f;
    lookY = 0.0f;

    // === Танк: WASD ===
    if (IsKeyDown(KEY_A))
        tankX -= 1.0f;
    if (IsKeyDown(KEY_D))
        tankX += 1.0f;
    if (IsKeyDown(KEY_W))
        tankY -= 1.0f;
    if (IsKeyDown(KEY_S))
        tankY += 1.0f;

    // === Камера: стрелки ===
    if (IsKeyDown(KEY_LEFT))
        camX -= 1.0f;
    if (IsKeyDown(KEY_RIGHT))
        camX += 1.0f;
    if (IsKeyDown(KEY_UP))
        camY -= 1.0f;
    if (IsKeyDown(KEY_DOWN))
        camY += 1.0f;

    // === Геймпад ===
    if (IsGamepadAvailable(0))
    {
        // Левый стик → танк
        tankX += applyDeadzone(GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X));
        tankY += applyDeadzone(GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y));

        // Правый стик → обзор камеры
        lookX = applyDeadzone(GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X));
        lookY = applyDeadzone(GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y));

        // D-pad → камера
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT))
            camX -= 1.0f;
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT))
            camX += 1.0f;
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_UP))
            camY -= 1.0f;
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN))
            camY += 1.0f;
    }

    // Клампим
    if (tankX > 1.0f)
        tankX = 1.0f;
    if (tankX < -1.0f)
        tankX = -1.0f;
    if (tankY > 1.0f)
        tankY = 1.0f;
    if (tankY < -1.0f)
        tankY = -1.0f;
}

float InputSystem::getTankX() const { return tankX; }
float InputSystem::getTankY() const { return tankY; }
float InputSystem::getCamX() const { return camX; }
float InputSystem::getCamY() const { return camY; }
float InputSystem::getLookX() const { return lookX; }
float InputSystem::getLookY() const { return lookY; }

bool InputSystem::isFirePressed() const
{
    bool c = IsKeyDown(KEY_SPACE);
    if (IsGamepadAvailable(0))
        c = c || IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
    return c;
}

bool InputSystem::isRearViewPressed() const
{
    bool c = IsKeyDown(KEY_RIGHT_CONTROL);
    if (IsGamepadAvailable(0))
        c = c || IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
    return c;
}

bool InputSystem::isTurboPressed() const
{
    bool c = IsKeyDown(KEY_LEFT_CONTROL);
    if (IsGamepadAvailable(0))
        c = c || IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
    return c;
}

bool InputSystem::isToggleIdPressed() const
{
    bool c = IsKeyDown(KEY_T);
    if (IsGamepadAvailable(0))
        c = c || IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_UP);
    return c;
}

bool InputSystem::isNextTankPressed() const
{
    if (IsGamepadAvailable(0))
        return IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1);
    return false;
}

bool InputSystem::isPrevTankPressed() const
{
    if (IsGamepadAvailable(0))
        return IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_TRIGGER_1);
    return false;
}

bool InputSystem::isQuitPressed() const
{
    bool c = IsKeyDown(KEY_Q);
    if (IsGamepadAvailable(0))
        c = c || IsGamepadButtonDown(0, GAMEPAD_BUTTON_MIDDLE_RIGHT);
    return c;
}

bool InputSystem::isMusicTogglePressed() const
{
    bool c = IsKeyDown(KEY_M);
    if (IsGamepadAvailable(0))
        c = c || IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_TRIGGER_2);
    return c;
}

bool InputSystem::isFpsTogglePressed() const
{
    bool c = IsKeyDown(KEY_F);
    if (IsGamepadAvailable(0))
        c = c || IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_2);
    return c;
}

bool InputSystem::isGamepadConnected() const
{
    return IsGamepadAvailable(0);
}