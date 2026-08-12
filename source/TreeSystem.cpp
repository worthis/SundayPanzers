#include "TreeSystem.h"
#include "Utils.h"
#include <cstdio>
#include <cmath>
#include "rlgl.h"
#include "raymath.h"

TreeSystem::TreeSystem()
{
    for (int i = 0; i < 7; i++)
    {
        treeModels[i] = {};
        treeTextures[i] = {};
        modelLoaded[i] = false;
        textureLoaded[i] = false;
    }

    reset();
}

TreeSystem::~TreeSystem()
{
    unloadTreeModels();
}

void TreeSystem::reset()
{
    for (int i = 0; i < MAX_TREES; i++)
    {
        trees[i] = Tree{};
    }

    treeCount = 0;
}

void TreeSystem::init(EventSystem *eventSystem, Terrain *terrain)
{
    this->eventSystem = eventSystem;
    this->terrain = terrain;

    eventSystem->subscribe<BulletFlightEvent>(
        [this](const BulletFlightEvent &e)
        { onBulletFlight(e); });

    eventSystem->subscribe<TankTreeCollisionEvent>(
        [this](const TankTreeCollisionEvent &e)
        { onTankTreeCollision(e); });

    loadTreeModels();
}

void TreeSystem::unloadTreeModels()
{
    for (int i = 1; i < 7; i++)
    {
        if (modelLoaded[i] && treeModels[i].meshCount > 0)
        {
            UnloadModel(treeModels[i]);
            treeModels[i] = {};
            modelLoaded[i] = false;
        }
        if (textureLoaded[i] && treeTextures[i].id != 0)
        {
            UnloadTexture(treeTextures[i]);
            treeTextures[i] = {};
            textureLoaded[i] = false;
        }
    }
}

void TreeSystem::loadTreeModels()
{
    unloadTreeModels();

    const char *treeFiles[] = {
        "",                     // индекс 0 не используется
        "data/trees/tree1.glb", // биом 1 - Grass
        "data/trees/tree2.glb", // биом 2 - Mountains
        "data/trees/cac.glb",   // биом 3 - Desert
        "data/trees/froz.glb",  // биом 4 - Frozen
        "data/trees/tree5.glb", // биом 5 - Tundra
        "data/trees/moon.glb"   // биом 6 - Moon
    };

    const char *textureFiles[] = {
        "",                     // индекс 0 не используется
        "data/trees/tree1.png", // биом 1 - Grass
        "data/trees/tree2.png", // биом 2 - Mountains
        "data/trees/cac.png",   // биом 3 - Desert
        "data/trees/froz.png",  // биом 4 - Frozen
        "data/trees/tree5.png", // биом 5 - Tundra
        "data/trees/moon.png"   // биом 6 - Moon
    };

    for (int i = 1; i < 7; i++)
    {
        Model m = LoadModel(treeFiles[i]);
        if (m.meshCount > 0)
        {
            treeModels[i] = m;
            modelLoaded[i] = true;
        }
        else
        {
            TraceLog(LOG_WARNING, "Tree model %d NOT found: %s", i, treeFiles[i]);
            continue;
        }

        Texture2D tex = LoadTexture(textureFiles[i]);
        if (tex.id != 0)
        {
            treeTextures[i] = tex;
            textureLoaded[i] = true;
            SetTextureFilter(tex, TEXTURE_FILTER_POINT);

            for (int m = 0; m < treeModels[i].materialCount; m++)
            {
                Material *mat = &treeModels[i].materials[m];
                mat->maps[MATERIAL_MAP_DIFFUSE].texture = tex;
                mat->maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
            }
        }
        else
        {
            TraceLog(LOG_WARNING, "Tree texture %d NOT found: %s", i, textureFiles[i]);
        }
    }
}

void TreeSystem::placeTrees(int biome)
{
    reset();

    if (!terrain)
        return;

    // TODO. Перенести сюда загрузку дерева для биома и текстуру

    treeCount = 0;

    for (int t = 0; t < MAX_TREES; t++)
    {
        // Координаты клетки (избегая краёв 0-3 и 46-49)
        int x = 4 + rnd(42);
        int z = 4 + rnd(42);

        // Проверка: нет ли уже объекта в этой клетке
        if (terrain->getCell(x, z).objectType != 0)
            continue;

        // Получаем высоту в центре клетки
        float worldX = CELL_SIZE * x + CELL_SIZE / 2.0f;
        float worldZ = CELL_SIZE * z + CELL_SIZE / 2.0f;
        float h = terrain->getHeight(worldX, worldZ);

        // Проверка порога высоты для каждого биома
        bool canPlace = false;
        float hForCheck = h;

        switch (biome)
        {
            // Grass, Mountains, Desert, Tundra
        case 1:
        case 2:
        case 3:
        case 5:
            canPlace = (h < 40.0f);
            break;
        // Frozen, Moon
        case 4:
        case 6:
            hForCheck = fabsf(h);
            canPlace = (hForCheck < 20.0f);
            break;
        }

        if (!canPlace)
            continue;

        // Меняем тайл на 8 или 16 (пятно под деревом)
        int tileIndex = 8;
        if (rnd(100) > 80)
        {
            tileIndex = 16;
        }
        terrain->setTile(x, z, tileIndex);

        // Помечаем клетку как занятую
        TerrainCell &cell = terrain->getCell(x, z);
        cell.objectType = 1;          // 1 - дерево
        cell.energy = 2;              // DBP: ter(1,x,z)=2 (прочность)
        cell.objectValue = treeCount; // индекс в trees[]

        // Масштаб (точно по DBP, конвертируем проценты в множители)
        float sc = (95.0f + rnd(11)) / 100.0f; // 0.95 - 1.05
        float scaleX = sc;
        float scaleY = sc;
        float scaleZ = sc;

        // Специфичные масштабы для разных биомов
        switch (biome)
        {
        case 2:
            sc = (95.0f + rnd(16)) / 100.0f;
            scaleX = sc;
            scaleY = 0.05f + sc;
            scaleZ = sc;
            break;
        case 3:
        case 5:
            scaleX = sc;
            scaleY = sc + (rnd(41) - 20) / 100.0f;
            scaleZ = sc;
            break;
        case 6:
            scaleX = sc;
            scaleY = sc - 0.30f + rnd(61) / 100.0f;
            scaleZ = sc;
            break;
        }

        // Сохраняем дерево в массиве
        trees[treeCount].active = true;
        trees[treeCount].position = {worldX, h - 2.0f, worldZ};
        trees[treeCount].rotationY = (float)rnd(360);
        trees[treeCount].scale = {scaleX, scaleY, scaleZ};
        trees[treeCount].biome = biome;
        trees[treeCount].cellX = x;
        trees[treeCount].cellZ = z;
        trees[treeCount].anim = TreeAnim{};

        treeCount++;
    }

    TraceLog(LOG_INFO, "Placed %d trees for biome %d", treeCount, biome);
}

// ============================================================
// NEW: Попадание в дерево
// DBP:
//   if ter(1,xm,zm)<=0  >  дерево умирает (state=2)
//   else                >  качание (state=1), ter(1)--
//   tree(...,1) = anb#+180   (направление падения)
//   tree(...,2) = 35         (начальная скорость)
// ============================================================
void TreeSystem::hitTree(int cellX, int cellZ, float angleFromSource)
{
    if (!terrain)
        return;

    if (cellX < 0 || cellX >= HEIGHTMAP_SIZE ||
        cellZ < 0 || cellZ >= HEIGHTMAP_SIZE)
        return;

    TerrainCell &cell = terrain->getCell(cellX, cellZ);
    if (cell.objectType != 1)
        return;

    int idx = cell.objectValue;
    if (idx < 0 || idx >= MAX_TREES || !trees[idx].active)
        return;

    TreeAnim &a = trees[idx].anim;
    if (a.state != 0)
        return; // уже анимируется

    // DBP: tree(...,1) = anb#+180  — направление падения
    a.angleY = wrapValue(angleFromSource + 180.0f);
    // DBP: tree(...,2) = 35
    a.swingSpeed = 35.0f;

    if (cell.energy <= 0)
    {
        // Дерево умирает — падает
        a.state = 2;
        a.tilt = 0.0f;
        // DBP: ter(0,xm,zm)=0  — клетка свободна
        cell.objectType = 0;
    }
    else
    {
        // Качание
        a.state = 1;
        a.tilt = 0.0f;
        cell.energy--;
    }
}

// ============================================================
// NEW: Тик анимации (100 Гц)
// DBP:
//   state=1 (bounce):
//     tree(t,2) -= 1
//     tree(t,3) += tree(t,2)
//     if tree(t,3)<=0 > reset
//   state=2 (fall):
//     tree(t,3) += tree(t,2)
//     if tree(t,3)>=1700 > delete
// ============================================================
void TreeSystem::update()
{
    for (int i = 0; i < treeCount; i++)
    {
        Tree &tr = trees[i];
        if (!tr.active)
            continue;

        TreeAnim &a = tr.anim;

        if (a.state == 1) // качание
        {
            a.swingSpeed -= 1.0f;
            a.tilt += a.swingSpeed;

            if (a.tilt <= 0.0f)
            {
                a.tilt = 0.0f;
                a.state = 0;
            }
        }
        else if (a.state == 2) // падение
        {
            a.tilt += a.swingSpeed;

            if (a.tilt >= 1700.0f)
            {
                // DBP: delete object 1000+t
                tr.active = false;
                a.state = 0;
                a.tilt = 0.0f;
            }
        }
    }
}

void TreeSystem::render() const
{
    for (int i = 0; i < MAX_TREES; i++)
    {
        const Tree &tr = trees[i];
        if (!tr.active)
            continue;

        int b = tr.biome;
        if (!modelLoaded[b])
            continue;

        const TreeAnim &a = tr.anim;

        if (a.state == 0)
        {
            // Статичное дерево
            DrawModelEx(
                treeModels[b],
                tr.position,
                (Vector3){0, 1, 0},
                tr.rotationY,
                tr.scale,
                WHITE);
        }
        else
        {
            // Анимированное: Y-поворот (направление) + X-наклон
            // DBP: rotate object, tilt/10, angleY, 0
            // OpenGL: Y-up, модель направлена вверх
            float tiltDeg = a.tilt / 10.0f;

            rlPushMatrix();
            rlTranslatef(tr.position.x, tr.position.y, tr.position.z);
            // Yaw: направление наклона
            // DBP>OpenGL: инвертируем Y-поворот
            rlRotatef(-a.angleY, 0, 1, 0);
            // Pitch: наклон дерева
            rlRotatef(tiltDeg, 1, 0, 0);
            rlScalef(tr.scale.x, tr.scale.y, tr.scale.z);

            // Отрисовка мешей модели
            const Model &mdl = treeModels[b];
            for (int m = 0; m < mdl.meshCount; m++)
            {
                int matIdx = mdl.meshMaterial[m];
                DrawMesh(mdl.meshes[m], mdl.materials[matIdx], MatrixIdentity());
            }
            rlPopMatrix();
        }
    }
}

void TreeSystem::onBulletFlight(const BulletFlightEvent &e)
{
    if (!terrain)
        return;

    if (!e.bullet.active)
        return;

    int xm = (int)(e.bullet.x / 100.0f);
    int zm = (int)(e.bullet.z / 100.0f);

    if (xm < 0 || xm >= HEIGHTMAP_SIZE || zm < 0 || zm >= HEIGHTMAP_SIZE)
        return;

    if (terrain->getCell(xm, zm).objectType != 1)
        return;

    float h = terrain->getHeight(e.bullet.x, e.bullet.z);
    float dh = e.bullet.y - h;

    if (dh >= 90.0f)
        return;

    float cex = xm * 100.0f + 50.0f;
    float cez = zm * 100.0f + 50.0f;

    bool col = false;

    // DBP: контроль — попадание в крону или в ствол
    if (dh > 37.0f)
    {
        if (fabsf(e.bullet.x - cex) < 40.0f && fabsf(e.bullet.z - cez) < 40.0f)
            col = true;
    }
    else
    {
        if (fabsf(e.bullet.x - cex) < 8.0f && fabsf(e.bullet.z - cez) < 8.0f)
            col = true;
    }

    if (col)
    {
        e.bullet.active = false;

        // DBP: position object 65000, cex, 0, cez
        //      point object 65000, bul#(n,1), 0, bul#(n,3)
        //      anb# = object angle y(65000)
        float hitAngle = atan2f(e.bullet.x - cex, e.bullet.z - cez) * RAD2DEG;
        if (hitAngle < 0.0f)
            hitAngle += 360.0f;

        // DBP: damage tree
        hitTree(xm, zm, hitAngle);

        eventSystem->publish(BulletTerrainHitEvent{
            .position = {e.bullet.x, e.bullet.y, e.bullet.z},
            .hitScale = e.bullet.hitScale,
            .bulletType = e.bullet.bulletType});
    }
}

void TreeSystem::onTankTreeCollision(const TankTreeCollisionEvent &e)
{
    hitTree(e.cellX, e.cellZ, e.hitAngle);
}