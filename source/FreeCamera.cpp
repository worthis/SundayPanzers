#include "FreeCamera.h"
#include <cmath>

FreeCamera::FreeCamera()
    : yaw(-90.0f), pitch(-25.0f), moveSpeed(500.0f), mouseSensitivity(0.15f), farPlane(15000.0f)
{
    camera = {0};
}

void FreeCamera::init(Vector3 position)
{
    camera.position = position;
    camera.target = Vector3Add(position, (Vector3){0.0f, -1.0f, 0.0f});
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Камера смотрит вниз под углом 45 градусов
    yaw = 0.0f;     // Смотрим вдоль +Z (к центру ландшафта)
    pitch = -45.0f; // Смотрим вниз под углом 45°
}

void FreeCamera::setPosition(Vector3 pos)
{
    camera.position = pos;
}

void FreeCamera::update(const InputSystem &input, float deltaTime)
{
    // === Вращение камеры ===
    if (!input.isGamepadConnected())
    {
        Vector2 mouseDelta = GetMouseDelta();
        yaw -= mouseDelta.x * mouseSensitivity;
        pitch -= mouseDelta.y * mouseSensitivity;
    }
    else
    {
        yaw -= input.getLookX() * 2.0f;
        pitch -= input.getLookY() * 2.0f;
    }

    // Ограничение pitch, чтобы камера не перевернулась
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    // === Перемещение камеры ===
    float speed = moveSpeed * deltaTime;

    // Ускорение (Shift / левый триггер)
    if (IsKeyDown(KEY_LEFT_SHIFT) ||
        (IsGamepadAvailable(0) && IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_TRIGGER_2)))
    {
        speed *= 3.0f;
    }

    float yawRad = yaw * DEG2RAD;
    float pitchRad = pitch * DEG2RAD;

    // 1. Вектор "вперед" (направление взгляда, включая вертикальный наклон pitch)
    // При yaw=0, pitch=0: forward = {0, 0, 1} (вдоль +Z)
    Vector3 forward = {
        cosf(pitchRad) * sinf(yawRad),
        sinf(pitchRad),
        cosf(pitchRad) * cosf(yawRad)
    };

    // 2. Вектор "вправо" (строго горизонтальный, перпендикулярный направлению взгляда)
    // При yaw=0: right = {1, 0, 0} (вдоль +X). При yaw=90: right = {0, 0, -1} (вдоль -Z).
    Vector3 right = {
        cosf(yawRad),
        0.0f,
        -sinf(yawRad)
    };

    float mx = input.getMoveX(); // A = -1, D = 1
    float my = input.getMoveY(); // W = -1, S = 1

    // Движение вперед/назад (W/S) - теперь учитывает наклон камеры (pitch)
    camera.position.x += forward.x * (-my) * speed;
    camera.position.y += forward.y * (-my) * speed;
    camera.position.z += forward.z * (-my) * speed;

    // Движение влево/вправо (A/D) - строго горизонтально
    camera.position.x += right.x * mx * speed;
    camera.position.z += right.z * mx * speed;

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

    // Обновление target (точки, куда смотрит камера)
    camera.target = {
        camera.position.x + forward.x,
        camera.position.y + forward.y,
        camera.position.z + forward.z
    };
}