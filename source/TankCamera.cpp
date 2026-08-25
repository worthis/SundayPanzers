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
    aspect = (float)GetScreenWidth() / GetScreenHeight();

    camPos = {x, y, z};
    camera = {};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.position = camPos;
    camera.target = {x, 0.0f, z + 100.0f};

    rlSetClipPlanes(CAMERA_NEAR, CAMERA_FAR);

    tanHalfDiagFov = tanf(camera.fovy * DEG2RAD * 0.5f) * sqrtf(1.0f + aspect * aspect);
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
    camPos.x = Lerp(camPos.x, tx, 1.0f / 14.0f);
    camPos.y = Lerp(camPos.y, ty, 1.0f / 16.0f);
    camPos.z = Lerp(camPos.z, tz, 1.0f / 16.0f);

    // DBP: position camera / point camera
    camera.position = camPos;
    camera.target = {tk.interpX, tk.interpY + 30.0f, tk.interpZ};

    camForward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
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
    float r = Vector2Length({slipCamFakeTank.x - slipCamTarget.x,
                             slipCamFakeTank.z - slipCamTarget.z});

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

bool TankCamera::isObjectVisible(Vector3 position, float radius) const
{
    Vector3 toTank = Vector3Subtract(position, camera.position);
    float dist = Vector3Length(toTank);

    if (dist > 0.001f)
    {
        // Расстояние вдоль оси камеры (проекция)
        float projDist = Vector3DotProduct(toTank, camForward);

        // Танк позади камеры — отсечь сразу
        if (projDist <= 0.0f)
            return false;

        // Перпендикулярное расстояние от центра танка до оси камеры
        // Используем теорему Пифагора: dist² = projDist² + perpDist²
        float perpDistSq = dist * dist - projDist * projDist;
        float coneRadius = projDist * tanHalfDiagFov; // Радиус конуса обзора на расстоянии танка
        float expandedRadius = coneRadius + radius;   // Расширяем радиус конуса на радиус танка

        // Если танк полностью вне конуса — отсечь
        if (perpDistSq > expandedRadius * expandedRadius)
            return false;
    }

    return true;
}