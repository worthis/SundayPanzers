#pragma once
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "GameConfig.h"
#include "TankSystem.h"
#include "Terrain.h"

class TankCamera
{
public:
    TankCamera();
    ~TankCamera();

    void init(float x, float y, float z);
    void track(const TankData &tk, const Terrain &terrain, bool rearView);

    Camera3D getCamera() const { return camera; }
    Vector3 getPosition() const { return camPos; }

    void startSlipCam(const TankData &fromTank, const TankData &toTank);
    void updateSlipCam(float dt, const Terrain &terrain, const TankData &targetTank);
    bool isSlipCamActive() const { return slipCamActive; }
    bool isSlipCamFinished() const { return slipCamFinished; }
    void resetSlipCam()
    {
        slipCamActive = false;
        slipCamFinished = false;
    }

private:
    Camera3D camera = {};
    Vector3 camPos = {MAP_CENTER, 1000, MAP_CENTER};

    float aspect;

    // SlipCam state
    bool slipCamActive = false;
    bool slipCamFinished = false;
    TankData slipCamFakeTank = {};
    Vector3 slipCamTarget = {};
};