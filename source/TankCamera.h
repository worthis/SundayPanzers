#pragma once
#include "GameConfig.h"
#include "GameData.h"
#include "Terrain.h"
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

class TankCamera
{
public:
    TankCamera();
    ~TankCamera();

    void init(float x, float y, float z);
    void track(const TankData &tk, const Terrain &terrain, bool rearView);

    Camera3D getCamera() const { return camera; }
    Vector3 getPosition() const { return camPos; }

    bool isObjectVisible(Vector3 position, float radius) const;

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
    float tanHalfDiagFov;
    Vector3 camForward = Vector3Zero();

    // SlipCam state
    bool slipCamActive = false;
    bool slipCamFinished = false;
    TankData slipCamFakeTank = {};
    Vector3 slipCamTarget = {};
};