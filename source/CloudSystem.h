#pragma once
#include "raylib.h"
#include "Terrain.h"
#include "GameConfig.h"

struct Cloud
{
    bool active = false;
    int modelId = 0;
    Vector3 position = {0, 0, 0};
    float angle = 0.0f;  // cl#(o,2) — текущий угол
    float radius = 0.0f; // cl#(o,1) — радиус от центра карты
    float speed = 0.0f;  // cl#(o,4) — скорость (градусы/сек)
    Vector3 scale = {1, 1, 1};
};

class CloudSystem
{
public:
    CloudSystem();
    ~CloudSystem();

    void init(Terrain *terrain);
    void generate(int biome);
    void update(float deltaTime);
    void render() const;
    void reset();

private:
    Cloud clouds[MAX_CLOUDS];
    int cloudCount;
    Terrain *terrain;

    Model cloudModels[4];       // индексы 1, 2, 3
    Texture2D cloudTextures[4]; // текстуры для каждой модели
    bool modelsLoaded[4];
    bool texturesLoaded[4];

    void loadModels();
    void unloadModels();
    void updateCloudPosition(int index);
};