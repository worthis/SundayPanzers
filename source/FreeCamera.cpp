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

void FreeCamera::update(float deltaTime)
{
    Vector2 delta = GetMouseDelta();

    if (delta.x != 0 || delta.y != 0)
    {
        yaw += delta.x * mouseSensitivity;
        pitch -= delta.y * mouseSensitivity;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    float yawRad = yaw * DEG2RAD;
    float pitchRad = pitch * DEG2RAD;

    Vector3 forward = {
        cosf(pitchRad) * cosf(yawRad),
        sinf(pitchRad),
        cosf(pitchRad) * sinf(yawRad)};

    Vector3 right = {
        cosf(yawRad - 3.14159265f / 2.0f),
        0.0f,
        sinf(yawRad - 3.14159265f / 2.0f)};

    float speed = moveSpeed * deltaTime;
    if (IsKeyDown(KEY_LEFT_SHIFT))
        speed *= 3.0f;

    Vector3 movement = {0.0f, 0.0f, 0.0f};

    if (IsKeyDown(KEY_W))
        movement = Vector3Add(movement, Vector3Scale(forward, speed));
    if (IsKeyDown(KEY_S))
        movement = Vector3Subtract(movement, Vector3Scale(forward, speed));
    if (IsKeyDown(KEY_D))
        movement = Vector3Subtract(movement, Vector3Scale(right, speed));
    if (IsKeyDown(KEY_A))
        movement = Vector3Add(movement, Vector3Scale(right, speed));
    if (IsKeyDown(KEY_E))
        movement.y += speed;
    if (IsKeyDown(KEY_Q))
        movement.y -= speed;

    camera.position = Vector3Add(camera.position, movement);
    camera.target = Vector3Add(camera.position, forward);
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
}