#include "TreeSystem.h"
#include "Utils.h"
#include <cstdio>
#include <cmath>

TreeSystem::TreeSystem() : terrain(nullptr)
{
    for (int i = 0; i < 7; i++)
    {
        treeModels[i] = {0};
        treeTextures[i] = {0};
        modelLoaded[i] = false;
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
        trees[i].active = false;
    }
}

void TreeSystem::init(Terrain *t)
{
    terrain = t;
    loadTreeModels();
}

void TreeSystem::unloadTreeModels()
{
    for (int i = 1; i <= 6; i++)
    {
        if (modelLoaded[i] && treeModels[i].meshCount > 0)
        {
            UnloadModel(treeModels[i]);
            treeModels[i] = {0};
            modelLoaded[i] = false;
        }
        if (textureLoaded[i] && treeTextures[i].id != 0)
        {
            UnloadTexture(treeTextures[i]);
            treeTextures[i] = {0};
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

    for (int i = 1; i <= 6; i++)
    {
        Model m = LoadModel(treeFiles[i]);
        if (m.meshCount > 0)
        {
            treeModels[i] = m;
            modelLoaded[i] = true;
            TraceLog(LOG_INFO, "Tree model %d loaded: %s", i, treeFiles[i]);
        }
        else
        {
            TraceLog(LOG_WARNING, "Tree model %d NOT found: %s", i, treeFiles[i]);
            continue;
        }

        // Загрузка текстуры
        Texture2D tex = LoadTexture(textureFiles[i]);
        if (tex.id != 0)
        {
            treeTextures[i] = tex;
            textureLoaded[i] = true;

            for (int m = 0; m < treeModels[i].materialCount; m++)
            {
                Material *mat = &treeModels[i].materials[m];
                mat->maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
                mat->maps[MATERIAL_MAP_DIFFUSE].texture = tex;
                SetTextureFilter(mat->maps[MATERIAL_MAP_DIFFUSE].texture,
                                 TEXTURE_FILTER_POINT);
            }

            TraceLog(LOG_INFO, "Tree texture %d loaded: %s", i, textureFiles[i]);
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

    int treeCount = 0;

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
        case 1: // Grass
            canPlace = (h < 40.0f);
            break;
        case 2: // Mountains
            canPlace = (h < 40.0f);
            break;
        case 3: // Desert
            canPlace = (h < 40.0f);
            break;
        case 4: // Frozen
            hForCheck = fabsf(h);
            canPlace = (hForCheck < 20.0f);
            break;
        case 5: // Tundra
            canPlace = (h < 40.0f);
            break;
        case 6: // Moon
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
        terrain->getCell(x, z).objectType = 1; // 1 = дерево

        // Вычисляем позицию дерева (точно по DBP)
        float posX = worldX;
        float posY = h - 2.0f; // на 1 единицу ниже поверхности
        float posZ = worldZ;

        // Поворот (точно по DBP: rnd(359))
        float rotY = (float)rnd(360);

        // Масштаб (точно по DBP, конвертируем проценты в множители)
        float sc = (95.0f + rnd(11)) / 100.0f; // 0.95 - 1.05
        float scaleX = sc;
        float scaleY = sc;
        float scaleZ = sc;

        // Специфичные масштабы для разных биомов
        switch (biome)
        {
        case 2:                                            // Mountains: вытянуты по Y
            sc = (95.0f + rnd(16)) / 100.0f; // 0.95 - 1.10
            scaleX = sc;
            scaleY = 0.05f + sc; // 5 + sc в DBP = 0.05 + sc в Raylib
            scaleZ = sc;
            break;

        case 3: // Desert: случайная высота
            sc = (95.0f + rnd(11)) / 100.0f;
            scaleX = sc;
            scaleY = sc + (rnd(41) - 20) / 100.0f;
            scaleZ = sc;
            break;

        case 5: // Tundra: случайная высота
            sc = (95.0f + rnd(11)) / 100.0f;
            scaleX = sc;
            scaleY = sc + (rnd(41) - 20) / 100.0f;
            scaleZ = sc;
            break;

        case 6: // Moon: сильно варьируется высота
            sc = (95.0f + rnd(11)) / 100.0f;
            scaleX = sc;
            scaleY = sc - 0.30f + rnd(61) / 100.0f; // 0.65 - 1.25
            scaleZ = sc;
            break;
        }

        // Сохраняем дерево в массиве
        trees[treeCount].active = true;
        trees[treeCount].position = {posX, posY, posZ};
        trees[treeCount].rotationY = rotY;
        trees[treeCount].scale = {scaleX, scaleY, scaleZ};
        trees[treeCount].biome = biome;

        treeCount++;
    }

    TraceLog(LOG_INFO, "Placed %d trees for biome %d", treeCount, biome);
}

void TreeSystem::render() const
{
    for (int i = 0; i < MAX_TREES; i++)
    {
        if (!trees[i].active)
            continue;

        int biome = trees[i].biome;
        if (!modelLoaded[biome])
            continue;

        // Отрисовка дерева с поворотом и масштабом
        DrawModelEx(
            treeModels[biome],
            trees[i].position,
            (Vector3){0.0f, 1.0f, 0.0f}, // ось вращения Y
            trees[i].rotationY,
            trees[i].scale,
            WHITE);
    }
}