#pragma once
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "GameConfig.h"
#include "TankSystem.h"
#include "Terrain.h"
#include "Utils.h"

// Точный порт DBP track(n) + slipcam()
class TankCamera
{
public:
    TankCamera() : camPos{MAP_CENTER, 1000, MAP_CENTER}, farPlane(CAMERA_FAR) {}

    void init(float x, float y, float z)
    {
        camPos = {x, y, z};
        camera = {0};
        camera.fovy = 60.0f;
        camera.projection = CAMERA_PERSPECTIVE;
        camera.up = {0.0f, 1.0f, 0.0f};
        camera.position = camPos;
        camera.target = {x, 0.0f, z + 100.0f};
    }

    // Устанавливаем дальность отрисовки через низкоуровневые функции rlgl
    // Аналог set camera range 6,7450 в DBP
    void applyRange()
    {
        float nearPlane = CAMERA_NEAR;
        float fovy = camera.fovy * DEG2RAD;
        float aspect = (float)SCREEN_WIDTH / SCREEN_HEIGHT;

        float top = nearPlane * tanf(fovy / 2.0f);
        float bottom = -top;
        float right = top * aspect;
        float left = -right;

        rlMatrixMode(RL_PROJECTION);
        rlLoadIdentity();
        rlFrustum(left, right, bottom, top, nearPlane, farPlane);
        rlMatrixMode(RL_MODELVIEW);
    }

    // DBP: track(n) — камера следует за танком
    void track(const TankData &tk, const Terrain &terrain, bool rearView)
    {
        // DBP: rv=joystick fire b()+keystate(157)
        float rv = rearView ? 1.0f : 0.0f;

        // DBP: tx#=newxvalue(tk#(n,1),tk#(n,5),-175+rv*375)
        float dist = -175.0f + rv * 375.0f;
        float tx = newXValue(tk.interpX, tk.interpYaw, dist);
        float tz = newZValue(tk.interpZ, tk.interpYaw, dist);

        // DBP: camera on tree?
        int xcm = (int)(tx / 100.0f);
        int zcm = (int)(tz / 100.0f);
        float htree = 0.0f;
        if (xcm >= 0 && xcm < HEIGHTMAP_SIZE && zcm >= 0 && zcm < HEIGHTMAP_SIZE)
        {
            if (terrain.getCell(xcm, zcm).objectType == 1)
                htree = 40.0f;
        }

        // DBP: ty#=get ground height(1,tx#,tz#)+105+htree+tk#(n,2)/11
        float ty = terrain.getHeight(tx, tz) + 105.0f + htree + tk.interpY / 11.0f;

        // DBP: инерция камеры
        // cam#(1)=cam#(1)+(tx#-cam#(1))/14
        camPos.x += (tx - camPos.x) / 14.0f;
        camPos.y += (ty - camPos.y) / 16.0f;
        camPos.z += (tz - camPos.z) / 16.0f;

        // DBP: position camera / point camera
        camera.position = camPos;
        camera.target = {tk.interpX, tk.interpY + 30.0f, tk.interpZ};
    }

    Camera3D getCamera() const { return camera; }
    float getFarPlane() const { return farPlane; }
    Vector3 getPosition() const { return camPos; }

private:
    Camera3D camera;
    Vector3 camPos;
    float farPlane;
};