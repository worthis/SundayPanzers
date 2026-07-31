#pragma once
#include "raylib.h"
#include "Terrain.h"

struct Cloud
{
    bool active;
    Model model;
    Vector3 position;
    float angle;  // cl#(o,2) — текущий угол
    float radius; // cl#(o,1) — радиус от центра карты
    float speed;  // cl#(o,4) — скорость (градусы/сек)
    Vector3 scale;
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
    static const int MAX_CLOUDS = 10;
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