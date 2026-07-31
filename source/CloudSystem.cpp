#include "CloudSystem.h"
#include "Utils.h"
#include <cstdio>
#include <cmath>

CloudSystem::CloudSystem() : terrain(nullptr), cloudCount(0)
{
    for (int i = 0; i < 4; i++)
    {
        cloudModels[i] = {0};
        cloudTextures[i] = {0};
        modelsLoaded[i] = false;
        texturesLoaded[i] = false;
    }
    reset();
}

CloudSystem::~CloudSystem()
{
    unloadModels();
}

void CloudSystem::reset()
{
    for (int i = 0; i < MAX_CLOUDS; i++)
    {
        clouds[i].active = false;
    }
    cloudCount = 0;
}

void CloudSystem::init(Terrain *t)
{
    terrain = t;
    loadModels();
}

void CloudSystem::unloadModels()
{
    for (int i = 1; i <= 3; i++)
    {
        if (modelsLoaded[i] && cloudModels[i].meshCount > 0)
        {
            UnloadModel(cloudModels[i]);
            cloudModels[i] = {0};
            modelsLoaded[i] = false;
        }
        if (texturesLoaded[i] && cloudTextures[i].id != 0)
        {
            UnloadTexture(cloudTextures[i]);
            cloudTextures[i] = {0};
            texturesLoaded[i] = false;
        }
    }
}

void CloudSystem::loadModels()
{
    unloadModels();

    const char *modelFiles[] = {
        "",                       // 0 не используется
        "data/clouds/cloud1.glb", // биомы 1, 2, 5
        "data/clouds/cloud2.glb", // биомы 3, 4
        "data/clouds/cloud3.glb"  // биом 6
    };

    const char *textureFiles[] = {
        "",                       // 0 не используется
        "data/clouds/cloud1.png", // биомы 1, 2, 5
        "data/clouds/cloud2.png", // биомы 3, 4
        "data/clouds/cloud3.png"  // биом 6
    };

    for (int i = 1; i <= 3; i++)
    {
        // Загрузка модели
        Model m = LoadModel(modelFiles[i]);
        if (m.meshCount > 0)
        {
            cloudModels[i] = m;
            modelsLoaded[i] = true;
            TraceLog(LOG_INFO, "Cloud model %d loaded: %s", i, modelFiles[i]);
        }
        else
        {
            TraceLog(LOG_WARNING, "Cloud model %d NOT found: %s", i, modelFiles[i]);
        }

        // Загрузка текстуры
        Texture2D tex = LoadTexture(textureFiles[i]);
        if (tex.id != 0)
        {
            cloudTextures[i] = tex;
            texturesLoaded[i] = true;

            // DBP: set object texture ob,0,0 — отключаем mipmapping
            SetTextureFilter(cloudTextures[i], TEXTURE_FILTER_POINT);
            TraceLog(LOG_INFO, "Cloud texture %d loaded: %s (%dx%d)",
                     i, textureFiles[i], tex.width, tex.height);

            // Применяем текстуру к модели
            if (modelsLoaded[i])
            {
                for (int j = 0; j < cloudModels[i].materialCount; j++)
                {
                    cloudModels[i].materials[j].maps[MATERIAL_MAP_DIFFUSE].texture = cloudTextures[i];
                    cloudModels[i].materials[j].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
                    SetTextureFilter(cloudModels[i].materials[j].maps[MATERIAL_MAP_DIFFUSE].texture, TEXTURE_FILTER_POINT);
                }
            }
        }
        else
        {
            TraceLog(LOG_WARNING, "Cloud texture %d NOT found: %s", i, textureFiles[i]);
        }
    }
}

void CloudSystem::updateCloudPosition(int index)
{
    Cloud &c = clouds[index];
    float rad = c.angle * DEG2RAD;

    // DBP: xc#=2500+cl#(o,1)*cos(cl#(o,2))
    float xc = MAP_CENTER + c.radius * cosf(rad);
    float zc = MAP_CENTER + c.radius * sinf(rad);

    // DBP: limit map
    if (xc < MAP_LIMIT_MIN)
        xc = MAP_LIMIT_MIN;
    if (xc > MAP_LIMIT_MAX)
        xc = MAP_LIMIT_MAX;
    if (zc < MAP_LIMIT_MIN)
        zc = MAP_LIMIT_MIN;
    if (zc > MAP_LIMIT_MAX)
        zc = MAP_LIMIT_MAX;

    // DBP: hc#=get ground height(1,xc#,zc#)+450+cl#(o,2)/50
    float h = terrain->getHeight(xc, zc);
    float hc = h + 450.0f + (c.angle / 50.0f);

    c.position = {xc, hc, zc};
}

void CloudSystem::generate(int biome)
{
    reset();
    if (!terrain)
        return;

    // DBP: cln=3+rnd(3)
    cloudCount = 3 + rnd(4);

    // DBP: выбор модели
    // биомы 1,2,5 → cloud1 (индекс 1)
    // биомы 3,4   → cloud2 (индекс 2)
    // биом 6      → cloud3 (индекс 3)
    int modelIdx = 1;
    if (biome == 3 || biome == 4)
        modelIdx = 2;
    else if (biome == 6)
        modelIdx = 3;

    if (!modelsLoaded[modelIdx])
    {
        TraceLog(LOG_WARNING, "Cannot generate clouds: model %d not loaded", modelIdx);
        return;
    }

    // DBP: for ob=1076 to 1076+cln (включительный цикл → cln+1 итераций)
    for (int i = 0; i <= cloudCount; i++)
    {
        if (i >= MAX_CLOUDS)
            break;

        clouds[i].active = true;
        clouds[i].model = cloudModels[modelIdx];

        // DBP: cl#(o,1)=rnd(2400)+400
        clouds[i].radius = 400.0f + (float)rnd(2401);

        // DBP: cl#(o,2)=rnd(359)
        clouds[i].angle = (float)rnd(360);

        // DBP: cl#(o,4)=rnd(10)/900.0 при ~100 FPS
        // Переводим в градусы/сек: * 100 (эмуляция 100 FPS из DBP)
        clouds[i].speed = ((float)rnd(11) / 900.0f) * 100.0f;

        // DBP: масштаб
        if (biome == 6)
        {
            // DBP: sc=190+rnd(55) : scale object ob,sc,sc,sc
            float sc = (190.0f + (float)rnd(56)) / 100.0f;
            clouds[i].scale = {sc, sc, sc};
        }
        else
        {
            // DBP: sc=230+rnd(85) : scale object ob,sc+rnd(10),sc+rnd(10),sc+rnd(10)
            // ВАЖНО: каждый rnd(10) — отдельный вызов, значения разные!
            float sc = (230.0f + (float)rnd(86)) / 100.0f;
            float sx = sc + (float)rnd(11) / 100.0f;
            float sy = sc + (float)rnd(11) / 100.0f;
            float sz = sc + (float)rnd(11) / 100.0f;
            clouds[i].scale = {sx, sy, sz};
        }

        // Начальная позиция
        updateCloudPosition(i);
    }

    TraceLog(LOG_INFO, "Generated %d clouds for biome %d (model %d)",
             cloudCount + 1, biome, modelIdx);
}

void CloudSystem::update(float deltaTime)
{
    for (int i = 0; i < MAX_CLOUDS; i++)
    {
        if (!clouds[i].active)
            continue;

        // DBP: cl#(o,2)=wrapvalue(cl#(o,2)+cl#(o,4))
        // speed в градусах/сек, умножаем на deltaTime
        clouds[i].angle = wrapValue(clouds[i].angle + clouds[i].speed * deltaTime);

        updateCloudPosition(i);
    }
}

void CloudSystem::render() const
{
    for (int i = 0; i < MAX_CLOUDS; i++)
    {
        if (!clouds[i].active)
            continue;

        // DBP: yrotate object ob,wrapvalue(cl#(o,2)+90)
        float rotY = wrapValue(clouds[i].angle + 90.0f);

        DrawModelEx(
            clouds[i].model,
            clouds[i].position,
            (Vector3){0.0f, 1.0f, 0.0f},
            rotY,
            clouds[i].scale,
            WHITE);
    }
}