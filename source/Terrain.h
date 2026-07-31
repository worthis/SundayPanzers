#pragma once
#include "raylib.h"
#include "GameConfig.h"

// Точная копия структуры данных из DBP:
// ter(2,50,50) - массив ландшафта
// 0 > тип объекта (0 нет, 1 дерево)
// 1 > энергия объекта
// 2 > значение 3D объекта
struct TerrainCell
{
    int objectType;  // ter(0,x,z)
    int energy;      // ter(1,x,z)
    int objectValue; // ter(2,x,z)
};

class Terrain
{
public:
    Terrain();
    ~Terrain();

    // Аналог maketerrain(l) из DBP
    void generate(int biome);

    // Построение mesh из массива высот
    void buildMesh();

    // Отрисовка ландшафта
    void render() const;

    // Аналог get ground height(1,x,z) из DBP
    // x,z - мировые координаты (0..5000)
    float getHeight(float x, float z) const;

    // Аналог color backdrop rgb(...) из DBP
    Color getBackdropColor() const;

    int getCurrentBiome() const { return currentBiome; }

    // Доступ к массиву ter() для других систем (деревья, танки)
    TerrainCell &getCell(int x, int z) { return ter[x][z]; }
    const TerrainCell &getCell(int x, int z) const { return ter[x][z]; }
    void setTile(int x, int z, int tileIndex);

private:
    // ter(2,50,50) из DBP - массив клеток ландшафта
    TerrainCell ter[HEIGHTMAP_SIZE][HEIGHTMAP_SIZE];

    // Массив высот для интерполяции (аналог matrix heights в DBP)
    float heights[HEIGHTMAP_SIZE][HEIGHTMAP_SIZE];

    // Номер тайла (1-24) для каждой клетки
    int tileIndices[HEIGHTMAP_SIZE - 1][HEIGHTMAP_SIZE - 1];

    Model model;
    Texture2D terrainTexture;
    int currentBiome;

    // Генерация для каждого биома (точно по DBP формулам)
    void generateGrass();     // case 1
    void generateMountains(); // case 2
    void generateDesert();    // case 3
    void generateFrozen();    // case 4
    void generateTundra();    // case 5
    void generateMoon();      // case 6

    void calculateTileIndices();
    Mesh createTerrainMesh();

    // Очистка массивов
    void reset();
};