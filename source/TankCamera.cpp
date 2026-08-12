#include "TankCamera.h"
#include "Utils.h"

TankCamera::TankCamera()
{
}

TankCamera::~TankCamera()
{
}

void TankCamera::init(float x, float y, float z)
{
    camPos = {x, y, z};
    camera = {};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.position = camPos;
    camera.target = {x, 0.0f, z + 100.0f};
}

// Устанавливаем дальность отрисовки через низкоуровневые функции rlgl
// Аналог set camera range 6,7450 в DBP
void TankCamera::applyRange()
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
void TankCamera::track(const TankData &tk, const Terrain &terrain, bool rearView)
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

void TankCamera::startSlipCam(const TankData &fromTank, const TankData &toTank)
{
    slipCamActive = true;
    slipCamFinished = false;

    // Инициализируем фейковый танк из позиции текущего танка игрока
    slipCamFakeTank = TankData{};
    slipCamFakeTank.x = fromTank.x;
    slipCamFakeTank.y = fromTank.y;
    slipCamFakeTank.z = fromTank.z;
    slipCamFakeTank.yaw = wrapValue(atan2f(toTank.x - fromTank.x, toTank.z - fromTank.z) * RAD2DEG);
    slipCamFakeTank.spin = 0.0f;

    slipCamFakeTank.interpX = fromTank.x;
    slipCamFakeTank.interpY = fromTank.y;
    slipCamFakeTank.interpZ = fromTank.z;
    slipCamFakeTank.interpYaw = fromTank.yaw;

    // Цель - запрашиваемый танк
    slipCamTarget = {toTank.x, 0.0f, toTank.z};
}

void TankCamera::updateSlipCam(float dt, const Terrain &terrain, const TankData &targetTank)
{
    if (!slipCamActive || slipCamFinished)
        return;

    slipCamTarget = {targetTank.x, 0.0f, targetTank.z};

    // Расчёт дистанции до цели
    float dx = slipCamFakeTank.x - slipCamTarget.x;
    float dz = slipCamFakeTank.z - slipCamTarget.z;
    float r = sqrtf(dx * dx + dz * dz);

    if (r < 7.5f)
    {
        slipCamFinished = true;
        slipCamFakeTank.interpX = slipCamFakeTank.x;
        slipCamFakeTank.interpY = slipCamFakeTank.y;
        slipCamFakeTank.interpZ = slipCamFakeTank.z;
        slipCamFakeTank.interpYaw = slipCamFakeTank.yaw;
        track(slipCamFakeTank, terrain, false);
        return;
    }

    // Вычисление угла к цели
    float ry = atan2f(slipCamTarget.x - slipCamFakeTank.x,
                      slipCamTarget.z - slipCamFakeTank.z) *
               RAD2DEG;
    slipCamFakeTank.yaw = wrapValue(ry);
    slipCamFakeTank.spin = 0.0f;

    // Движение: f#=6+r#/125
    float f = 6.0f + r / 125.0f;
    if (f > r)
        f = r;

    slipCamFakeTank.x = newXValue(slipCamFakeTank.x, slipCamFakeTank.yaw, f);
    slipCamFakeTank.z = newZValue(slipCamFakeTank.z, slipCamFakeTank.yaw, f);
    slipCamFakeTank.y = terrain.getHeight(slipCamFakeTank.x, slipCamFakeTank.z);

    // Ограничения карты
    if (slipCamFakeTank.x < 370.0f)
        slipCamFakeTank.x = 370.0f;
    if (slipCamFakeTank.z < 370.0f)
        slipCamFakeTank.z = 370.0f;
    if (slipCamFakeTank.x > 4630.0f)
        slipCamFakeTank.x = 4630.0f;
    if (slipCamFakeTank.z > 4630.0f)
        slipCamFakeTank.z = 4630.0f;

    slipCamFakeTank.interpX = slipCamFakeTank.x;
    slipCamFakeTank.interpY = slipCamFakeTank.y;
    slipCamFakeTank.interpZ = slipCamFakeTank.z;
    slipCamFakeTank.interpYaw = slipCamFakeTank.yaw;

    // Камера следует за фейковым танком
    track(slipCamFakeTank, terrain, false);
}