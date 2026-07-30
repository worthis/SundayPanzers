#pragma once
#include "raylib.h"
#include "raymath.h"

class FreeCamera
{
public:
    FreeCamera();

    void init(Vector3 position);
    void update(float deltaTime);
    Camera3D getCamera() const { return camera; }
    float getFarPlane() const { return farPlane; }

    void setPosition(Vector3 pos);
    Vector3 getPosition() const { return camera.position; }

private:
    Camera3D camera;
    float yaw;   // поворот вокруг Y (влево-вправо)
    float pitch; // наклон (вверх-вниз)
    float moveSpeed;
    float mouseSensitivity;
    float farPlane;
};