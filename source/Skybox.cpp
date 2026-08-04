#include "Skybox.h"
#include "Utils.h"
#include <cstdio>
#include "rlgl.h"
#include "raymath.h"

Skybox::Skybox() : currentBiome(1), sizeX(0.0f), sizeY(0.0f), sizeZ(0.0f)
{
    wall = {0};
    ceiling = {0};
    wallTexture = {0};
    topTexture = {0};
}

Skybox::~Skybox()
{
    unload();
}

void Skybox::unload()
{
    if (wall.meshCount > 0)
        UnloadModel(wall);
    if (ceiling.meshCount > 0)
        UnloadModel(ceiling);
    if (wallTexture.id != 0)
        UnloadTexture(wallTexture);
    if (topTexture.id != 0)
        UnloadTexture(topTexture);

    wall = {0};
    ceiling = {0};
    wallTexture = {0};
    topTexture = {0};
}

Color Skybox::getSkyColor(int biome) const
{
    // Точные цвета из DBP: color backdrop rgb(...)
    switch (biome)
    {
    case 1:
        return (Color){128, 166, 38, 255}; // grass: rgb(128,166,38)
    case 2:
        return (Color){132, 124, 77, 255}; // mountains: rgb(132,124,77)
    case 3:
        return (Color){252, 238, 174, 255}; // desert: rgb(252,238,174)
    case 4:
        return (Color){183, 213, 239, 255}; // frozen: rgb(183,213,239)
    case 5:
        return (Color){191, 149, 124, 255}; // tundra: rgb(191,149,124)
    case 6:
        return (Color){131, 123, 119, 255}; // moon: rgb(131,123,119)
    default:
        return SKYBLUE;
    }
}

Model Skybox::createPlane(float width, float height)
{
    Model plane = LoadModelFromMesh(GenMeshPlane(width, height, 1, 1));
    return plane;
}

void Skybox::load(int biome)
{
    // Освобождаем предыдущий skybox
    unload();

    currentBiome = biome;

    // Загрузка текстур
    char wallPath[64], topPath[64];
    snprintf(wallPath, sizeof(wallPath), "data/sky/sky%d.png", biome);
    snprintf(topPath, sizeof(topPath), "data/sky/sky%dtop.png", biome);

    wallTexture = LoadTexture(wallPath);
    if (wallTexture.id == 0)
    {
        TraceLog(LOG_WARNING, "Wall texture not found: %s", wallPath);
    }
    else
    {
        SetTextureFilter(wallTexture, TEXTURE_FILTER_ANISOTROPIC_8X);
    }

    topTexture = LoadTexture(topPath);
    if (topTexture.id == 0)
    {
        TraceLog(LOG_WARNING, "Top texture not found: %s", topPath);
    }
    else
    {
        SetTextureFilter(topTexture, TEXTURE_FILTER_ANISOTROPIC_8X);
    }

    // Размеры скайбокса (как в DBP)
    sizeX = MAP_SIZE;
    sizeY = (biome == 6) ? 1680.0f : 1960.0f;
    sizeZ = MAP_SIZE;

    wall = createPlane(sizeX, sizeY);
    ceiling = createPlane(sizeX + 4.0f, sizeZ + 4.0f);

    // Назначение текстур материалам
    if (wallTexture.id != 0)
    {
        wall.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = wallTexture;
    }
    else
    {
        wall.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = getSkyColor(biome);
    }

    if (topTexture.id != 0)
    {
        ceiling.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = topTexture;
    }
    else
    {
        ceiling.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = getSkyColor(biome);
    }

    TraceLog(LOG_INFO, "Skybox created for biome %d", biome);
}

void Skybox::render() const
{
    if (wall.meshCount == 0 ||
        ceiling.meshCount == 0)
        return;

    // Позиция из DBP: position object 1100,2500,-2,2500
    Vector3 center = {MAP_CENTER, -2.0f, MAP_CENTER};

    float halfX = sizeX / 2.0f;
    float halfY = sizeY / 2.0f;
    float halfZ = sizeZ / 2.0f;

    // Передняя стенка (Z+) — смотрит на север
    rlPushMatrix();
    rlTranslatef(center.x, center.y + halfY, center.z + halfZ);
    rlRotatef(180.0f, 0, 1, 0);  // Поворот вокруг Y
    rlRotatef(90.0f, 1, 0, 0); // Поворот вокруг X
    DrawModel(wall, (Vector3){0, 0, 0}, 1.0f, WHITE);
    rlPopMatrix();

    // Задняя стенка (Z-) — смотрит на юг
    rlPushMatrix();
    rlTranslatef(center.x, center.y + halfY, center.z - halfZ);
    rlRotatef(0.0f, 0, 1, 0); // Поворот вокруг Y
    rlRotatef(90.0f, 1, 0, 0);  // Поворот вокруг X
    DrawModel(wall, (Vector3){0, 0, 0}, 1.0f, WHITE);
    rlPopMatrix();

    // Левая стенка (X-) — смотрит на запад
    rlPushMatrix();
    rlTranslatef(center.x - halfX, center.y + halfY, center.z);
    rlRotatef(90.0f, 0, 1, 0); // Поворот вокруг Y
    rlRotatef(90.0f, 1, 0, 0); // Поворот вокруг X
    DrawModel(wall, (Vector3){0, 0, 0}, 1.0f, WHITE);
    rlPopMatrix();

    // Правая стенка (X+) — смотрит на восток
    rlPushMatrix();
    rlTranslatef(center.x + halfX, center.y + halfY, center.z);
    rlRotatef(-90.0f, 0, 1, 0); // Поворот вокруг Y
    rlRotatef(90.0f, 1, 0, 0);  // Поворот вокруг X
    DrawModel(wall, (Vector3){0, 0, 0}, 1.0f, WHITE);
    rlPopMatrix();

    // Потолок (Y+)
    DrawModelEx(ceiling,
                (Vector3){center.x - 2.0f, center.y + sizeY - 2.0f, center.z - 2.0f},
                (Vector3){1.0f, 0.0f, 0.0f}, 180.0f,
                (Vector3){1.0f, 1.0f, 1.0f}, WHITE);
}