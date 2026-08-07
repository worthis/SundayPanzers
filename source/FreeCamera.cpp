#include "FreeCamera.h"
#include <cmath>

FreeCamera::FreeCamera()
    : yaw(-90.0f), pitch(-25.0f), moveSpeed(500.0f), mouseSensitivity(0.15f), farPlane(15000.0f)
{
    camera = {};
}

void FreeCamera::init(Vector3 position)
{
    camera.position = position;
    camera.target = Vector3Add(position, (Vector3){0.0f, -1.0f, 0.0f});
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    yaw = 0.0f;
    pitch = -45.0f;
}

void FreeCamera::setPosition(Vector3 pos)
{
    camera.position = pos;
}

void FreeCamera::update(const InputSystem &input, float deltaTime)
{
    // === Обзор: мышь / правый стик ===
    if (!input.isGamepadConnected())
    {
        Vector2 mouseDelta = GetMouseDelta();
        yaw += mouseDelta.x * mouseSensitivity;
        pitch -= mouseDelta.y * mouseSensitivity;
    }
    else
    {
        yaw += input.getLookX() * 2.0f;
        pitch -= input.getLookY() * 2.0f;
    }

    // === Стрелки / D-pad: влево/вправо = вращение yaw ===
    // Инвертировано: стрелка вправо должна поворачивать камеру вправо
    yaw -= input.getCamX() * 1.5f;

    // Стрелки вверх/вниз НЕ меняют pitch — они двигают камеру (см. ниже)

    // Ограничение pitch
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    // === Перемещение камеры ===
    float speed = moveSpeed * deltaTime;

    // Ускорение (Shift / ZL)
    if (IsKeyDown(KEY_LEFT_SHIFT) ||
        (IsGamepadAvailable(0) && IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_TRIGGER_2)))
    {
        speed *= 3.0f;
    }

    float yawRad = yaw * DEG2RAD;
    float pitchRad = pitch * DEG2RAD;

    // Вектор "вперёд" (направление взгляда)
    Vector3 forward = {
        cosf(pitchRad) * sinf(yawRad),
        sinf(pitchRad),
        cosf(pitchRad) * cosf(yawRad)};

    // Стрелки вверх/вниз = движение камеры вперёд/назад
    float my = input.getCamY();

    // Движение вперёд/назад (стрелки)
    camera.position.x += forward.x * (-my) * speed;
    camera.position.y += forward.y * (-my) * speed;
    camera.position.z += forward.z * (-my) * speed;

    // Вверх/вниз (E/Q на клавиатуре, D-pad на геймпаде)
    if (IsKeyDown(KEY_E) ||
        (IsGamepadAvailable(0) && IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_UP)))
    {
        camera.position.y += speed;
    }
    if (IsKeyDown(KEY_Q) ||
        (IsGamepadAvailable(0) && IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN)))
    {
        camera.position.y -= speed;
    }

    // Обновление target
    camera.target = {
        camera.position.x + forward.x,
        camera.position.y + forward.y,
        camera.position.z + forward.z};
}