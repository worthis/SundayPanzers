#include "BulletSystem.h"
#include "Utils.h"
#include "rlgl.h"
#include <cstdio>

// ============================================================
// Конструктор — value-initialization вместо memset
// ============================================================
BulletSystem::BulletSystem()
{
    for (int i = 0; i <= MAX_BULLETS; i++)
        bullets[i] = BulletData{};
    for (int i = 0; i <= MAX_HIT_EFFECTS; i++)
        hits[i] = HitEffect{};
    for (int i = 0; i < MAX_EXPLOSIONS; i++)
        explosions[i] = ExplosionData{};
}

BulletSystem::~BulletSystem()
{
    if (bulletModelsLoaded)
    {
        UnloadModel(bulletModel1);
        UnloadModel(bulletModel2);
    }
    if (hitModelsLoaded)
    {
        UnloadModel(hitModel1);
        UnloadModel(hitModel2);
    }
    if (explosionModelsLoaded)
    {
        UnloadModel(ringModel);
        UnloadModel(ballModel);
    }
    if (texturesLoaded)
    {
        UnloadTexture(bulletTex1);
        UnloadTexture(bulletTex2);
        UnloadTexture(hitTex1);
        UnloadTexture(hitTex2);
        UnloadTexture(ringTex);
        UnloadTexture(exploTex);
    }
}

void BulletSystem::init(Terrain *terrain, TankSystem *tankSystem)
{
    this->terrain = terrain;
    this->tankSystem = tankSystem;
}

// ============================================================
// Привязка текстуры к модели + unlit-настройка
// DBP: set object specular n,0
//      set object n,1,0,0,0,0,0,0   (unlit, no fog)
//      set object texture n,0,0      (no mipmapping)
// ============================================================
void BulletSystem::bindTexture(Model &m, const Texture2D &tex)
{
    for (int j = 0; j < m.materialCount; j++)
    {
        m.materials[j].maps[MATERIAL_MAP_DIFFUSE].texture = tex;
        m.materials[j].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
        m.materials[j].maps[MATERIAL_MAP_EMISSION].texture = tex;
        m.materials[j].maps[MATERIAL_MAP_EMISSION].color = WHITE;
        m.materials[j].maps[MATERIAL_MAP_SPECULAR].color = BLACK;
        SetTextureFilter(tex, TEXTURE_FILTER_POINT);
    }
}

// ============================================================
// Загрузка текстуры с DBP color key (чёрный → прозрачный)
// DBP: load image "file.bmp", N, 1
// ============================================================
Texture2D BulletSystem::loadTextureColorKey(const char *path)
{
    Image img = LoadImage(path);
    if (img.data == nullptr)
    {
        TraceLog(LOG_WARNING, "Texture not found: %s", path);
        return LoadTextureFromImage(img); // вернёт пустую
    }

    // DBP хранит как 32-bit RGBA после color key
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    // Color key: тёмные пиксели → alpha = 0
    Color *pixels = static_cast<Color *>(img.data);
    int count = img.width * img.height;
    for (int i = 0; i < count; i++)
    {
        if (pixels[i].r < 16 && pixels[i].g < 16 && pixels[i].b < 16)
            pixels[i].a = 0;
    }

    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

void BulletSystem::loadAssets()
{
    bulletModel1 = LoadModel("data/bullets/bullet.glb");
    bulletModel2 = LoadModel("data/bullets/bullet2.glb");
    bulletModelsLoaded = true;

    hitModel1 = LoadModel("data/bullets/hit.glb");
    hitModel2 = LoadModel("data/bullets/hit2.glb");
    hitModelsLoaded = true;

    ringModel = LoadModel("data/bullets/ring.glb");
    ballModel = LoadModel("data/bullets/ball.glb");
    explosionModelsLoaded = true;

    bulletTex1 = LoadTexture("data/bullets/bullet.png");
    bulletTex2 = LoadTexture("data/bullets/bullet2.png");
    hitTex1 = LoadTexture("data/bullets/hit.png");
    hitTex2 = LoadTexture("data/bullets/hit2.png");
    ringTex = loadTextureColorKey("data/bullets/ring.png");
    exploTex = LoadTexture("data/bullets/expo.png");
    texturesLoaded = true;

    bindTexture(bulletModel1, bulletTex1);
    bindTexture(bulletModel2, bulletTex2);
    bindTexture(hitModel1, hitTex1);
    bindTexture(hitModel2, hitTex2);

    // DBP для ring/ball:
    //   set object emissive 401,rgb(255,255,20)
    //   set object diffuse 401,rgb(255,255,20)
    //   set object 401,1,1,0,0,1,0,0  (ghost)
    // Текстура ring.png привязывается, но тонирована жёлтым
    auto setupExplosionMaterial = [](Model &m)
    {
        for (int j = 0; j < m.materialCount; j++)
        {
            m.materials[j].maps[MATERIAL_MAP_DIFFUSE].color =
                Color{255, 255, 20, 255};
            m.materials[j].maps[MATERIAL_MAP_SPECULAR].color = BLACK;
        }
    };

    bindTexture(ringModel, ringTex);
    setupExplosionMaterial(ringModel);

    bindTexture(ballModel, exploTex);
    setupExplosionMaterial(ballModel);
}

// ============================================================
// Позиция дула:
//   1) Используется muzzleLocal из TankSystem (реальный меш)
//   2) Порядок вращений: Rz → Rx → Ry (совпадает с рендером)
// ============================================================
void BulletSystem::getMuzzlePosition(int n, float &mx, float &my, float &mz) const
{
    const TankData &tk = tankSystem->getTank(n);
    int type = tk.type;
    if (type < 1 || type > 8)
        type = 1;

    // === Локальная позиция дула = центр меша fireLimb ===
    // Вычислен из реальной геометрии модели при загрузке
    Vector3 local = tankSystem->getMuzzleLocal(type);

    // Применяем масштаб танка
    float lx = local.x * tk.scaleX;
    float ly = local.y * tk.scaleY;
    float lz = local.z * tk.scaleZ;

    // Углы танка (DBP: pitch object up = tk#(n,4)+sin(walk)*2)
    float yaw = tk.yaw;
    float pitch = tk.pitch + sinDeg(tk.walkSine) * 2.0f;
    float roll = tk.roll;

    // Порядок: Scale → Rz(roll) → Rx(pitch) → Ry(yaw+180) → Translate
    // Это совпадает с матрицей рендера: T * Ry * Rx * Rz * S

    // 1) Rz (roll)
    float cr = cosDeg(roll), sr = sinDeg(roll);
    float x1 = lx * cr - ly * sr;
    float y1 = lx * sr + ly * cr;
    float z1 = lz;

    // 2) Rx (pitch)
    float pitchRad = pitch * DEG2RAD;
    float cp = cosf(pitchRad), sp = sinf(pitchRad);
    float x2 = x1;
    float y2 = y1 * cp - z1 * sp;
    float z2 = y1 * sp + z1 * cp;

    // 3) Ry (yaw + 180 для OpenGL)
    float yawRad = (yaw + 180.0f) * DEG2RAD;
    float cy = cosf(yawRad), sy = sinf(yawRad);
    float x3 = x2 * cy + z2 * sy;
    float y3 = y2;
    float z3 = -x2 * sy + z2 * cy;

    // 4) Translate
    mx = tk.x + x3;
    my = tk.y + 3.3f + y3; // DBP: position object n,...,tk#(n,2)+3.3,...
    mz = tk.z + z3;
}

// ============================================================
// Направление выстрела
// ============================================================
void BulletSystem::getMuzzleDirection(int n, float &dx, float &dy, float &dz) const
{
    const TankData &tk = tankSystem->getTank(n);

    float yaw = tk.yaw;
    // ИСПРАВЛЕНО: shotDegree → shotAngle
    float pitch = tk.pitch + sinDeg(tk.walkSine) * 2.0f + tk.shotAngle;

    float pitchRad = pitch * DEG2RAD;
    float cosP = cosf(pitchRad);
    float sinP = sinf(pitchRad);

    dx = sinDeg(yaw) * cosP;
    dy = sinP;
    dz = cosDeg(yaw) * cosP;

    float len = sqrtf(dx * dx + dy * dy + dz * dz);
    if (len > 0.0001f)
    {
        dx /= len;
        dy /= len;
        dz /= len;
    }
}

// ============================================================
// Выстрел
// ============================================================
void BulletSystem::fireBullet(int n)
{
    if (n < 1 || n > MAX_BULLETS || !tankSystem)
        return;

    TankData &tk = tankSystem->getTankMut(n);

    // DBP: if fire=1 and tk#(n,18)=0 and tk#(n,17)=0
    if (tk.reloadCounter > 0 || tk.bulletCounter > 0)
        return;
    if (!tk.canFire)
        return;

    float mx, my, mz;
    getMuzzlePosition(n, mx, my, mz);

    float dx, dy, dz;
    getMuzzleDirection(n, dx, dy, dz);

    // DBP: move object b,6
    mx += dx * 6.0f;
    my += dy * 6.0f;
    mz += dz * 6.0f;

    BulletData &b = bullets[n];
    b.active = true;
    b.owner = n;
    b.x = mx;
    b.y = my;
    b.z = mz;
    b.dirX = dx;
    b.dirY = dy;
    b.dirZ = dz;
    b.gravCounter = 0;
    b.lifeCounter = tk.bulletLength;
    b.bulletPower = tk.bulletPower;
    b.bulletGravity = tk.bulletGravity;
    // ИСПРАВЛЕНО: colorId → squadId
    b.ownerSquadId = tk.squadId;
    // ИСПРАВЛЕНО: superBullet → bulletFlag
    b.superBullet = (tk.bulletFlag > 0);
    // ИСПРАВЛЕНО: hitMagnifier → hitScale
    b.hitScale = tk.hitScale;
    b.bulletScale = tk.bulletScale; // ← НОВОЕ
    b.bulletModelType = tk.hitModelType;
    b.hitModelType = tk.hitModelType; // ← НОВОЕ

    // DBP: tk#(n,17)=tk#(n,20) : tk#(n,18)=tk#(n,19)
    tk.bulletCounter = tk.bulletLength;
    tk.reloadCounter = tk.reloadTime;
    tk.hitCounter = 0;
}

// ============================================================
// Обновление — фиксированный тик 100 Гц
// ============================================================
void BulletSystem::update()
{
    if (!terrain || !tankSystem)
        return;

    // === Перезарядка танков ===
    for (int n = 1; n < MAX_TANKS; n++)
    {
        TankData &tk = tankSystem->getTankMut(n);
        if (tk.type == 0)
            continue;

        if (tk.reloadCounter > 0)
        {
            tk.reloadCounter--;
            tk.hitCounter++;
        }
    }

    // === Движение и коллизии пуль ===
    for (int n = 1; n <= MAX_BULLETS; n++)
    {
        BulletData &b = bullets[n];
        if (!b.active)
            continue;

        b.lifeCounter--;

        // DBP: move object b,8.5
        b.x += b.dirX * 8.5f;
        b.y += b.dirY * 8.5f;
        b.z += b.dirZ * 8.5f;

        // DBP: гравитация
        b.gravCounter += b.bulletGravity;
        b.y -= b.gravCounter;

        int dead = 0;
        int treeIdx = 0;

        if (b.lifeCounter <= 0)
            dead = 1;

        if (dead == 0 && checkGroundCollision(b))
            dead = 1;

        if (dead == 0 && checkTreeCollision(b, treeIdx))
            dead = 1;

        if (dead == 0 && checkMapBounds(b))
            dead = 1;

        int hitTank = 0;
        if (dead == 0)
        {
            hitTank = checkTankCollision(b);
            if (hitTank > 0)
                dead = 2;
        }

        if (dead > 0)
        {
            b.active = false;

            // ★ DBP: tk#(n,17)=0 — обнуляем, чтобы можно было стрелять снова
            tankSystem->getTankMut(b.owner).bulletCounter = 0;

            spawnHitEffect(b.x, b.y, b.z, b.hitScale, b.hitModelType);

            if (dead == 2 && hitTank > 0)
            {
                float ddx = tankSystem->getTank(hitTank).x - b.x;
                float ddz = tankSystem->getTank(hitTank).z - b.z;
                float collAngle = atan2f(ddx, ddz) * RAD2DEG;
                if (collAngle < 0)
                    collAngle += 360.0f;

                applyDamage(hitTank, b, collAngle);
            }
        }
    }

    // === Эффекты попадания ===
    for (int i = 1; i <= MAX_HIT_EFFECTS; i++)
    {
        HitEffect &h = hits[i];
        if (!h.active)
            continue;

        h.counter -= 3;
        h.angleY = wrapValue(h.angleY + 3.0f);

        if (h.counter < 4)
        {
            h.counter = 0;
            h.active = false;
        }
    }

    // === Взрывы ===
    for (int i = 0; i < MAX_EXPLOSIONS; i++)
    {
        ExplosionData &e = explosions[i];
        if (!e.active)
            continue;

        e.counter += e.direction;

        if (e.counter < 1)
            e.direction = 2;

        if (e.counter >= 80)
            e.active = false;
    }
}

// ============================================================
// Коллизии
// ============================================================

bool BulletSystem::checkGroundCollision(const BulletData &b) const
{
    float h = terrain->getHeight(b.x, b.z);
    return h > b.y;
}

bool BulletSystem::checkTreeCollision(BulletData &b, int &treeIndex) const
{
    int xm = (int)(b.x / 100.0f);
    int zm = (int)(b.z / 100.0f);

    if (xm < 0 || xm >= HEIGHTMAP_SIZE || zm < 0 || zm >= HEIGHTMAP_SIZE)
        return false;

    if (terrain->getCell(xm, zm).objectType != 1)
        return false;

    float h = terrain->getHeight(b.x, b.z);
    float dh = b.y - h;

    if (dh >= 90.0f)
        return false;

    float cex = xm * 100.0f + 50.0f;
    float cez = zm * 100.0f + 50.0f;

    bool col = false;
    if (dh > 37.0f)
    {
        if (fabsf(b.x - cex) < 40.0f && fabsf(b.z - cez) < 40.0f)
            col = true;
    }
    else
    {
        if (fabsf(b.x - cex) < 8.0f && fabsf(b.z - cez) < 8.0f)
            col = true;
    }

    if (col)
        treeIndex = terrain->getCell(xm, zm).objectValue;

    return col;
}

bool BulletSystem::checkMapBounds(const BulletData &b) const
{
    return b.x < 20.0f || b.x > 4980.0f ||
           b.z < 20.0f || b.z > 4980.0f;
}

int BulletSystem::checkTankCollision(const BulletData &b) const
{
    for (int c = 1; c < MAX_TANKS; c++)
    {
        if (c == b.owner)
            continue;

        const TankData &tk = tankSystem->getTank(c);
        if (tk.type == 0)
            continue;

        float dx = b.x - tk.x;
        float dz = b.z - tk.z;
        // ИСПРАВЛЕНО: collHeight → collisionHeight
        float dy = b.y - (tk.y + tk.collisionHeight);

        float r = sqrtf(dx * dx + dz * dz);

        // ИСПРАВЛЕНО: collRadius → collisionRange, collHeight → collisionHeight
        if (r < (tk.collisionRange + 2.0f) &&
            fabsf(dy) < (tk.collisionHeight + 2.0f))
            return c;
    }
    return 0;
}

// ============================================================
// Урон
// ============================================================
void BulletSystem::applyDamage(int targetIdx, const BulletData &b, float collAngle)
{
    TankData &tk = tankSystem->getTankMut(targetIdx);
    if (tk.type == 0)
        return;

    // ИСПРАВЛЕНО: collAngle → bounceAngle, collForce → bounceForce
    tk.bounceAngle = collAngle;
    tk.bounceForce = 2.0f;
    tk.accel /= 2.0f;

    if (tk.type < 0)
        tk.bounceForce = 1.0f;

    if (tk.type <= 0)
        return;

    // DBP: abullet#=abs(tk#(c,14)-tk#(c,5))
    float abullet = fabsf(collAngle - tk.yaw);
    if (abullet > 180.0f)
        abullet = 360.0f - abullet;

    float khit = 0.21f + (180.0f - abullet) / 180.0f;

    if (khit < 0.355f)
        khit *= 0.665f;

    // ИСПРАВЛЕНО: colorId → squadId
    if (b.ownerSquadId == tk.squadId)
        khit /= 4.0f;

    // ИСПРАВЛЕНО: barrier → barrierCounter
    if (tk.barrierCounter > 0)
        khit = 0.0f;

    if (b.superBullet)
        khit *= 2.0f;

    tk.energy -= b.bulletPower * khit;

    if (khit > 0.94f)
        tk.hitCounter = 10;

    // Смена текстуры при 2/3 урона
    if (tk.energy < tk.originalEnergy / 3.0f && !tk.damaged)
    {
        tk.damaged = true;
        // TODO: сменить текстуру на td<squadId>.png
    }

    // Уничтожение
    if (tk.energy < 0.0f)
    {
        tk.type = -1;
        // ИСПРАВЛЕНО: collForce → bounceForce
        tk.bounceForce = 0;
        tk.accel = 0;
        tk.fallForce = 0;

        // TODO: сменить текстуру на tw<squadId>.png

        float h = terrain->getHeight(tk.x, tk.z);
        float range = 35.0f + tk.collisionRange * 3.0f;

        // DBP: gam(4)=55, gam(5)=range, gam(6)=-1
        // Кольцо ориентируется по танку, шар — случайный Y
        spawnExplosion(tk.x, h + 10.0f, tk.z, range,
                       tk.yaw, tk.pitch, tk.roll);
    }
}

// ============================================================
// Эффекты
// ============================================================

void BulletSystem::spawnHitEffect(float x, float y, float z,
                                  float magnifier, int modelType)
{
    for (int i = 1; i <= MAX_HIT_EFFECTS; i++)
    {
        if (!hits[i].active)
        {
            hits[i].active = true;
            hits[i].x = x;
            hits[i].y = y;
            hits[i].z = z;
            hits[i].counter = 100;
            hits[i].angleY = (float)rnd(360);
            hits[i].magnifier = magnifier;
            hits[i].modelType = modelType;
            return;
        }
    }
}

void BulletSystem::spawnExplosion(float x, float y, float z, float range,
                                  float yaw, float pitch, float roll)
{
    for (int i = 0; i < MAX_EXPLOSIONS; i++)
    {
        if (!explosions[i].active)
        {
            explosions[i].active = true;
            explosions[i].x = x;
            explosions[i].y = y;
            explosions[i].z = z;
            explosions[i].counter = 55;
            explosions[i].direction = -1;
            explosions[i].range = range;

            // DBP: ring ориентируется по танку
            explosions[i].ringYaw = yaw;
            explosions[i].ringPitch = pitch;
            explosions[i].ringRoll = roll + 4.0f - (float)rnd(8);

            // DBP: rotate object 402,0,rnd(359),0
            explosions[i].ballYaw = (float)rnd(360);
            return;
        }
    }
}

// ============================================================
// Отрисовка
// ============================================================

void BulletSystem::render() const
{
    // === Пули ===
    for (int n = 1; n <= MAX_BULLETS; n++)
    {
        const BulletData &b = bullets[n];
        if (!b.active)
            continue;

        // Выбор модели: типы 7-8 → bullet2
        const Model &mdl = (b.hitModelType == 2) ? bulletModel2 : bulletModel1;

        // Направление → углы для рендера
        float yaw = atan2f(b.dirX, b.dirZ) * RAD2DEG;
        float pitch = asinf(b.dirY) * RAD2DEG;
        float sc = b.bulletScale;

        rlPushMatrix();
        rlTranslatef(b.x, b.y, b.z);
        rlRotatef(yaw + 180.0f, 0, 1, 0); // DBP→OpenGL
        rlRotatef(pitch, 1, 0, 0);        // +pitch = нос вверх
        rlScalef(sc, sc, sc);
        DrawModel(mdl, {0, 0, 0}, 1.0f, WHITE);
        rlPopMatrix();
    }

    // === Эффекты попадания ===
    for (int i = 1; i <= MAX_HIT_EFFECTS; i++)
    {
        const HitEffect &h = hits[i];
        if (!h.active)
            continue;

        // DBP: sc=(20+(100-hit(n))/1.25)*tk#(n,39)
        float sc = (20.0f + (100.0f - h.counter) / 1.25f) * h.magnifier / 100.0f;

        const Model &mdl = (h.modelType == 2) ? hitModel2 : hitModel1;

        // tint: hit1 — белая вспышка, hit2 — цветное свечение
        // (их текстура hit2 синяя → белый tint выбеливает, цветной tint
        //  ограничивает добавку по каналам и возвращает оттенок)
        Color tint = (h.modelType == 2)
                         ? Color{100, 80, 230, 255}   // ← hit2: сине-фиолетовый, стартовый
                         : Color{255, 255, 255, 255}; // ← hit1: как в оригинале

        BeginBlendMode(BLEND_ADDITIVE);

        rlPushMatrix();
        rlTranslatef(h.x, h.y, h.z);
        rlRotatef(h.angleY, 0, 1, 0);
        rlScalef(sc, sc, sc);
        // DrawModel(mdl, {0, 0, 0}, 1.0f, WHITE);
        //  Alpha 128 = 50% — DBP ghost по умолчанию
        DrawModel(mdl, {0, 0, 0}, 1.0f, tint);
        rlPopMatrix();

        EndBlendMode();
    }

    // === Взрывы ===
    for (int i = 0; i < MAX_EXPLOSIONS; i++)
    {
        const ExplosionData &e = explosions[i];
        if (!e.active)
            continue;

        float r1 = (1.8f * (95.0f - e.counter * 1.1f) * e.range) / 100.0f;
        float r2 = (1.9f * (95.0f - e.counter / 1.1f) * e.range) / 100.0f;

        if (r1 < 0.1f)
            r1 = 0.1f;
        if (r2 < 0.1f)
            r2 = 0.1f;

        // ★ DBP scale object = проценты → делим на 100
        r1 /= 100.0f;
        r2 /= 100.0f;

        // Включаем аддитивное смешивание (DBP: ghost object on)
        BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);

        // Кольцо — ориентация по уничтоженному танку
        rlPushMatrix();
        rlTranslatef(e.x, e.y, e.z);
        rlRotatef(e.ringYaw + 180.0f, 0, 1, 0);
        rlRotatef(e.ringPitch, 1, 0, 0);
        rlRotatef(e.ringRoll, 0, 0, 1);
        rlScalef(r1, r1, r1);
        DrawModel(ringModel, {0, 0, 0}, 1.0f, Color{255, 255, 20, 180});
        rlPopMatrix();

        // Шар — случайный Y-поворот
        rlPushMatrix();
        rlTranslatef(e.x, e.y, e.z);
        rlRotatef(e.ballYaw, 0, 1, 0);
        rlScalef(r2, r2, r2);
        DrawModel(ballModel, {0, 0, 0}, 1.0f, Color{255, 255, 20, 180});
        rlPopMatrix();

        // Возвращаем стандартный alpha blending
        EndBlendMode();
    }
}