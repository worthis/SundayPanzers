#pragma once
#include "raylib.h"

class Skybox
{
public:
    Skybox();
    ~Skybox();

    void load(int biome);
    void render() const;
    void unload();
    int getCurrentBiome() const { return currentBiome; }

private:
    Model wall;
    Model ceiling;

    Texture2D wallTexture;
    Texture2D topTexture;
    int currentBiome;

    float sizeX, sizeY, sizeZ;

    Model createPlane(float width, float height);

    // Получение цвета неба для биома (из DBP: color backdrop rgb(...))
    Color getSkyColor(int biome) const;
};