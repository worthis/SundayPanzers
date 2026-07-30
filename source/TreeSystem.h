#pragma once
#include "raylib.h"
#include "Terrain.h"

struct Tree
{
    bool active;
    Vector3 position;
    float rotationY;
    Vector3 scale;
    int biome;
};

class TreeSystem
{
public:
    TreeSystem();
    ~TreeSystem();

    void init(Terrain *terrain);
    void placeTrees(int biome);
    void render() const;
    void reset();

private:
    static const int MAX_TREES = 75;
    Tree trees[MAX_TREES];
    Terrain *terrain;

    Model treeModels[7]; // индексы 1-6 для биомов
    Texture2D treeTextures[7];
    bool modelLoaded[7];
    bool textureLoaded[7];

    void loadTreeModels();
    void unloadTreeModels();
};