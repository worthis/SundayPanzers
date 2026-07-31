#include "Terrain.h"
#include "Utils.h"
#include <cmath>
#include <cstdlib>
#include <cstdio>

Terrain::Terrain() : currentBiome(1)
{
    reset();
    model = {0};
}

Terrain::~Terrain()
{
    if (model.meshCount > 0)
        UnloadModel(model);
    if (terrainTexture.id != 0)
        UnloadTexture(terrainTexture);
}

void Terrain::reset()
{
    for (int z = 0; z < HEIGHTMAP_SIZE; z++)
    {
        for (int x = 0; x < HEIGHTMAP_SIZE; x++)
        {
            heights[z][x] = 0.0f;
            ter[z][x].objectType = 0;
            ter[z][x].energy = 0;
            ter[z][x].objectValue = 0;
        }
    }

    for (int z = 0; z < HEIGHTMAP_SIZE - 1; z++)
    {
        for (int x = 0; x < HEIGHTMAP_SIZE - 1; x++)
        {
            tileIndices[z][x] = 1;
        }
    }
}

// Изменение тайла, строго до buildMesh
void Terrain::setTile(int x, int z, int tileIndex)
{
    if (x >= 0 && x < 50 && z >= 0 && z < 50)
    {
        tileIndices[z][x] = tileIndex;
    }
}

// === ГЕНЕРАЦИЯ БИОМОВ (ТОЧНО ПО DBP) ===

void Terrain::generateGrass()
{
    // case 1: scenario grass country

    // making the great hills "Crest tipe"
    if (rnd(100) > 65)
    {
        int nseq = rnd(2) + 1;
        for (int s = 1; s <= nseq; s++)
        {
            int stx = rnd(38) + 1;
            int stz = rnd(38) + 1;
            float hm = 20.0f + rnd(200);

            for (int x = stx; x <= stx + 11; x++)
            {
                float ang = 16.3f * (x - stx);
                float sa = sinDeg(ang);

                // DBP: 12 строк set matrix height для stz..stz+11
                heights[stz][x] += sa * (hm / 10.0f);
                heights[stz + 1][x] += sa * (hm / 8.0f);
                heights[stz + 2][x] += sa * (hm / 6.0f);
                heights[stz + 3][x] += sa * (hm / 3.5f);
                heights[stz + 4][x] += sa * (hm / 1.5f);
                heights[stz + 5][x] += sa * hm;
                heights[stz + 6][x] += sa * hm;
                heights[stz + 7][x] += sa * (hm / 1.5f);
                heights[stz + 8][x] += sa * (hm / 3.5f);
                heights[stz + 9][x] += sa * (hm / 6.0f);
                heights[stz + 10][x] += sa * (hm / 8.0f);
                heights[stz + 11][x] += sa * (hm / 10.0f);
            }
        }
    }

    // making the great hills "breast" tipe
    int nseq = rnd(15) + 2;
    for (int s = 1; s <= nseq; s++)
    {
        int stx = rnd(33) + 3;
        int stz = rnd(33) + 3;
        float hm = 120.0f + rnd(100);
        if (rnd(100) > 93)
            hm += 100.0f;

        for (int z = stz; z <= stz + 11; z++)
        {
            float ang = 16.3f * (z - stz);
            float sa = sinDeg(ang);

            for (int xatt = 0; xatt <= 11; xatt++)
            {
                float ang2 = xatt * 16.3f;
                float sa2 = sinDeg(ang2);
                heights[z][stx + xatt] += sa * sa2 * hm;
            }
        }
    }

    // making the flat hills "breast" tipe
    nseq = rnd(23) + 9;
    for (int s = 1; s <= nseq; s++)
    {
        int stx = rnd(32) + 3;
        int stz = rnd(32) + 3;
        float hm = 20.0f + rnd(30);

        for (int z = stz; z <= stz + 11; z++)
        {
            float ang = 16.3f * (z - stz);
            float sa = sinDeg(ang);

            for (int xatt = 0; xatt <= 11; xatt++)
            {
                float ang2 = xatt * 16.3f;
                float sa2 = sinDeg(ang2);
                heights[z][stx + xatt] += sa * sa2 * hm;
            }
        }
    }
}

void Terrain::generateMountains()
{
    // case 2: scenario mountains

    // making the short "Crest tipe"
    int nseq = rnd(3) + 3;
    for (int s = 1; s <= nseq; s++)
    {
        int stx = rnd(38) + 1;
        int stz = rnd(38) + 1;
        float hm = 10.0f + rnd(20);

        for (int x = stx; x <= stx + 11; x++)
        {
            float ang = 16.3f * (x - stx);
            float sa = sinDeg(ang);

            heights[stz][x] += sa * (hm / 15.0f);
            heights[stz + 1][x] += sa * (hm / 10.0f);
            heights[stz + 2][x] += sa * (hm / 8.0f);
            heights[stz + 3][x] += sa * (hm / 3.5f);
            heights[stz + 4][x] += sa * (hm / 1.5f);
            heights[stz + 5][x] += sa * hm;
            heights[stz + 6][x] += sa * hm;
            heights[stz + 7][x] += sa * (hm / 1.5f);
            heights[stz + 8][x] += sa * (hm / 3.5f);
            heights[stz + 9][x] += sa * (hm / 8.0f);
            heights[stz + 10][x] += sa * (hm / 10.0f);
            heights[stz + 11][x] += sa * (hm / 15.0f);
        }
    }

    // making the great mountains
    nseq = rnd(15) + 2;
    for (int s = 1; s <= nseq; s++)
    {
        int stx = rnd(33) + 3;
        int stz = rnd(33) + 3;
        float hm = 80.0f + rnd(80);
        if (rnd(100) > 93)
            hm += 35.0f;

        for (int z = stz; z <= stz + 11; z++)
        {
            float ang = 16.3f * (z - stz);
            float sa = sinDeg(ang);

            for (int xatt = 0; xatt <= 11; xatt++)
            {
                float ang2 = xatt * 16.3f;
                float sa2 = sinDeg(ang2);
                int vh = xatt;
                if (xatt > 5)
                    vh = abs(xatt - 11);
                heights[z][stx + xatt] += sa * sa * sa2 * hm + vh * 10.0f;
            }
        }
    }

    // making the flat hills "breast" tipe
    nseq = rnd(23) + 9;
    for (int s = 1; s <= nseq; s++)
    {
        int stx = rnd(32) + 3;
        int stz = rnd(32) + 3;
        float hm = 20.0f + rnd(30);

        for (int z = stz; z <= stz + 11; z++)
        {
            float ang = 16.3f * (z - stz);
            float sa = sinDeg(ang);

            for (int xatt = 0; xatt <= 11; xatt++)
            {
                float ang2 = xatt * 16.3f;
                float sa2 = sinDeg(ang2);
                int vh = xatt;
                if (xatt > 5)
                    vh = abs(xatt - 11);
                heights[z][stx + xatt] += sa * sa2 * hm + vh * 2.0f;
            }
        }
    }
}

void Terrain::generateDesert()
{
    // case 3: scenario desert

    // making the great dunes
    int nseq = rnd(13) + 4;
    for (int s = 1; s <= nseq; s++)
    {
        int stx = rnd(33) + 3;
        int stz = rnd(33) + 3;
        float hm = 100.0f + rnd(90);
        if (rnd(100) > 93)
            hm += 60.0f;

        for (int z = stz; z <= stz + 11; z++)
        {
            float ang = 16.3f * (z - stz);
            float sa = sinDeg(ang);

            for (int xatt = 0; xatt <= 11; xatt++)
            {
                float ang2 = xatt * 16.3f;
                float sa2 = sinDeg(ang2);
                heights[z][stx + xatt] += sa * sa2 * hm;
            }
        }
    }

    // making the flat sand hills
    nseq = rnd(23) + 15;
    for (int s = 1; s <= nseq; s++)
    {
        int stx = rnd(32) + 3;
        int stz = rnd(32) + 3;
        float hm = 10.0f + rnd(30);

        for (int z = stz; z <= stz + 11; z++)
        {
            float ang = 16.3f * (z - stz);
            float sa = sinDeg(ang);

            for (int xatt = 0; xatt <= 11; xatt++)
            {
                float ang2 = xatt * 16.3f;
                float sa2 = sinDeg(ang2);
                heights[z][stx + xatt] += sa * sa2 * hm;
            }
        }
    }
}

void Terrain::generateFrozen()
{
    // case 4: scenario frozed country

    // making the great hills "Crest tipe"
    int nseq = rnd(2) + 5;
    for (int s = 1; s <= nseq; s++)
    {
        int stx = rnd(38) + 1;
        int stz = rnd(38) + 1;
        float hm = 20.0f + rnd(20);

        for (int x = stx; x <= stx + 11; x++)
        {
            float ang = 16.3f * (x - stx);
            float sa = sinDeg(ang);

            heights[stz][x] += sa * (hm / 10.0f);
            heights[stz + 1][x] += sa * (hm / 8.0f);
            heights[stz + 2][x] += sa * (hm / 6.0f);
            heights[stz + 3][x] += sa * (hm / 3.5f);
            heights[stz + 4][x] += sa * (hm / 1.5f);
            heights[stz + 5][x] += sa * hm;
            heights[stz + 6][x] += sa * hm;
            heights[stz + 7][x] += sa * (hm / 1.5f);
            heights[stz + 8][x] += sa * (hm / 3.5f);
            heights[stz + 9][x] += sa * (hm / 6.0f);
            heights[stz + 10][x] += sa * (hm / 8.0f);
            heights[stz + 11][x] += sa * (hm / 10.0f);
        }
    }

    // making the great holes
    nseq = rnd(7) + 5;
    for (int s = 1; s <= nseq; s++)
    {
        int stx = rnd(33) + 3;
        int stz = rnd(33) + 3;
        float hm = 20.0f + rnd(100);
        if (rnd(100) > 90)
            hm += 120.0f;
        hm = -hm;

        for (int z = stz; z <= stz + 11; z++)
        {
            float ang = 16.3f * (z - stz);
            float sa = sinDeg(ang);

            for (int xatt = 0; xatt <= 11; xatt++)
            {
                float ang2 = xatt * 16.3f;
                float sa2 = sinDeg(ang2);
                heights[z][stx + xatt] += sa * sa2 * hm;
            }
        }
    }

    // making the hills
    nseq = rnd(20) + 11;
    for (int s = 1; s <= nseq; s++)
    {
        int stx = rnd(32) + 3;
        int stz = rnd(32) + 3;
        float hm = 20.0f + rnd(140);
        float gg = rnd(10);

        for (int z = stz; z <= stz + 11; z++)
        {
            float ang = 16.3f * (z - stz);
            float sa = sinDeg(ang);

            for (int xatt = 0; xatt <= 11; xatt++)
            {
                float ang2 = xatt * 16.3f;
                float sa2 = sinDeg(ang2);
                heights[z][stx + xatt] += sa * sa2 * hm + gg;
            }
        }
    }
}

void Terrain::generateTundra()
{
    // case 5: scenario tundra

    // making the great hills
    int nseq = rnd(10) + 9;
    for (int s = 1; s <= nseq; s++)
    {
        int stx = rnd(33) + 3;
        int stz = rnd(33) + 3;
        float hm = 105.0f + rnd(90);
        if (rnd(100) > 93)
            hm += 10.0f;

        for (int z = stz; z <= stz + 11; z++)
        {
            float ang = 16.3f * (z - stz);
            float sa = sinDeg(ang);

            for (int xatt = 0; xatt <= 11; xatt++)
            {
                float ang2 = xatt * 16.3f;
                float sa2 = sinDeg(ang2);
                heights[z][stx + xatt] += sa * sa2 * hm;
            }
        }
    }

    // making the flat hills
    nseq = rnd(23) + 15;
    for (int s = 1; s <= nseq; s++)
    {
        int stx = rnd(32) + 3;
        int stz = rnd(32) + 3;
        float hm = 10.0f + rnd(30);

        for (int z = stz; z <= stz + 11; z++)
        {
            float ang = 16.3f * (z - stz);
            float sa = sinDeg(ang);

            for (int xatt = 0; xatt <= 11; xatt++)
            {
                float ang2 = xatt * 16.3f;
                float sa2 = sinDeg(ang2);
                heights[z][stx + xatt] += sa * sa2 * hm;
            }
        }
    }
}

void Terrain::generateMoon()
{
    // case 6: scenario the moon

    // making the great holes
    int nseq = rnd(7) + 5;
    for (int s = 1; s <= nseq; s++)
    {
        int stx = rnd(33) + 3;
        int stz = rnd(33) + 3;
        float hm = 70.0f + rnd(120);
        if (rnd(100) > 90)
            hm += 120.0f;
        hm = -hm;

        for (int z = stz; z <= stz + 11; z++)
        {
            float ang = 16.3f * (z - stz);
            float sa = sinDeg(ang);

            for (int xatt = 0; xatt <= 11; xatt++)
            {
                float ang2 = xatt * 16.3f;
                float sa2 = sinDeg(ang2);
                heights[z][stx + xatt] += sa * sa2 * hm;
            }
        }
    }

    // making the hills
    nseq = rnd(20) + 5;
    for (int s = 1; s <= nseq; s++)
    {
        int stx = rnd(32) + 3;
        int stz = rnd(32) + 3;
        float hm = 20.0f + rnd(40);
        if (rnd(100) > 90)
            hm += 100.0f;
        float gg = rnd(5);

        for (int z = stz; z <= stz + 11; z++)
        {
            float ang = 16.3f * (z - stz);
            float sa = sinDeg(ang);

            for (int xatt = 0; xatt <= 11; xatt++)
            {
                float ang2 = xatt * 16.3f;
                float sa2 = sinDeg(ang2);
                heights[z][stx + xatt] += sa * sa2 * hm + gg;
            }
        }
    }
}

// === ВЫЧИСЛЕНИЕ ТАЙЛОВ (ТОЧНО ПО DBP) ===
// В DBP:
//   for xm=0 to 49
//     for zm=0 to 49
//       h=get ground height(1,100*xm+50,100*zm+50)+10
//       ... формула зависит от биома ...
//       set matrix tile 1,xm,zm,nt
//   Затем отдельно устанавливаются границы (borders)
void Terrain::calculateTileIndices()
{
    // === ФАЗА 1: Основной цикл по всем клеткам ===
    for (int xm = 0; xm < 50; xm++)
    {
        for (int zm = 0; zm < 50; zm++)
        {
            // Высота в ЦЕНТРЕ клетки (как в DBP: 100*xm+50, 100*zm+50)
            float worldX = 100.0f * xm + 50.0f;
            float worldZ = 100.0f * zm + 50.0f;
            float h = getHeight(worldX, worldZ) + 10.0f;

            // Для биомов 4 (frozen) и 6 (moon) высота берётся по модулю
            float hForCalc = h;
            if (currentBiome == 4 || currentBiome == 6)
            {
                hForCalc = fabsf(h);
            }

            // Формула dv зависит от биома (точно по DBP)
            float dv;
            switch (currentBiome)
            {
            case 1:
                dv = 45.0f + (hForCalc / 22.0f);
                break; // grass
            case 2:
                dv = 45.0f + (hForCalc / 25.0f);
                break; // mountains
            case 3:
                dv = 45.0f + (hForCalc / 30.0f);
                break; // desert
            case 4:
                dv = 31.0f + (hForCalc / 60.0f);
                break; // frozen
            case 5:
                dv = 43.0f + (hForCalc / 28.0f);
                break; // tundra
            case 6:
                dv = 31.0f + (hForCalc / 60.0f);
                break; // moon
            default:
                dv = 45.0f + (hForCalc / 22.0f);
                break;
            }

            // Вычисление номера тайла
            // DBP: nt = 1 + h/dv  (nt — integer, truncation)
            int nt = (int)(1.0f + hForCalc / dv);
            
            // DBP: if nt > 9 then nt = 9
            if (nt > 9) nt = 9;

            // DBP: if nt = 8 then nt = 23
            if (nt == 8) nt = 23;

            // DBP: if nt = 9 then nt = 24
            if (nt == 9) nt = 24;

            // 20% шанс получить "альтернативный" тайл (ряд 2)
            if (rnd(100) > 80 && nt < 8)
                nt = nt + 8;

            if (nt < 1) nt = 1;
            if (nt > 24) nt = 24;

            // ВАЖНО: в DBP set matrix tile 1,xm,zm,nt
            // где xm - это колонка (X), zm - это строка (Z)
            tileIndices[zm][xm] = nt;
        }
    }

    // === ФАЗА 2: Границы карты (borders) ===
    // В DBP:
    //   for z=3 to 46
    //     v1=22:v2=22
    //     if z=3 then v1=20:v2=19
    //     if z=46 then v1=17:v2=18
    //     set matrix tile 1,3,z,v1:set matrix tile 1,46,z,v2
    //   next z
    //   for x=4 to 45
    //     set matrix tile 1,x,3,21:set matrix tile 1,x,46,21
    //   next x

    // Левая и правая границы (x=3 и x=46)
    for (int z = 3; z <= 46; z++)
    {
        // стандартный тайл границы
        int v1 = 22;
        int v2 = 22;

        // Углы имеют специальные тайлы
        // верхние углы
        if (z == 3)
        {
            v1 = 20;
            v2 = 19;
        }
        // нижние углы
        if (z == 46)
        {
            v1 = 17;
            v2 = 18;
        }

        tileIndices[z][3] = v1;  // левая граница
        tileIndices[z][46] = v2; // правая граница
    }

    // Верхняя и нижняя границы (z=3 и z=46)
    for (int x = 4; x <= 45; x++)
    {
        tileIndices[3][x] = 21;  // верхняя граница
        tileIndices[46][x] = 21; // нижняя граница
    }
}

Mesh Terrain::createTerrainMesh()
{
    Mesh mesh = {0};

    int segments = 50;
    int quadCount = segments * segments;
    int vertexCount = quadCount * 4;
    int triangleCount = quadCount * 2;

    mesh.vertexCount = vertexCount;
    mesh.triangleCount = triangleCount;

    mesh.vertices = (float *)RL_CALLOC(vertexCount * 3, sizeof(float));
    mesh.texcoords = (float *)RL_CALLOC(vertexCount * 2, sizeof(float));
    mesh.normals = (float *)RL_CALLOC(vertexCount * 3, sizeof(float));
    mesh.indices = (unsigned short *)RL_CALLOC(triangleCount * 3, sizeof(unsigned short));

    float cellSize = MAP_SIZE / segments;
    int vIdx = 0;
    int iIdx = 0;

    for (int zm = 0; zm < segments; zm++)
    {
        for (int xm = 0; xm < segments; xm++)
        {
            int tileIdx = tileIndices[zm][xm];

            // Вычисляем UV для этого тайла
            // Тайл 1-8: ряд 0 (верх текстуры)
            // Тайл 9-16: ряд 1 (середина)
            // Тайл 17-24: ряд 2 (низ)
            int tileCol = (tileIdx - 1) % 8;
            int tileRow = (tileIdx - 1) / 8;

            // UV координаты в OpenGL (V инвертирована относительно BMP)
            float u0 = tileCol / 8.0f;
            float u1 = (tileCol + 1) / 8.0f;
            float v0 = (tileRow + 1) / 3.0f;
            float v1 = tileRow / 3.0f;

            // Позиции вершин
            float x0 = xm * cellSize;
            float x1 = (xm + 1) * cellSize;
            float z0 = zm * cellSize;
            float z1 = (zm + 1) * cellSize;

            // Высоты углов
            float h00 = heights[zm][xm];
            float h10 = heights[zm][xm + 1];
            float h01 = heights[zm + 1][xm];
            float h11 = heights[zm + 1][xm + 1];

            // Вершина 0: (x0, z0) - UV: (u0, v0)
            mesh.vertices[vIdx * 3 + 0] = x0;
            mesh.vertices[vIdx * 3 + 1] = h00;
            mesh.vertices[vIdx * 3 + 2] = z0;
            mesh.texcoords[vIdx * 2 + 0] = u0;
            mesh.texcoords[vIdx * 2 + 1] = v0;
            mesh.normals[vIdx * 3 + 0] = 0.0f;
            mesh.normals[vIdx * 3 + 1] = 1.0f;
            mesh.normals[vIdx * 3 + 2] = 0.0f;
            vIdx++;

            // Вершина 1: (x1, z0) - UV: (u1, v0)
            mesh.vertices[vIdx * 3 + 0] = x1;
            mesh.vertices[vIdx * 3 + 1] = h10;
            mesh.vertices[vIdx * 3 + 2] = z0;
            mesh.texcoords[vIdx * 2 + 0] = u1;
            mesh.texcoords[vIdx * 2 + 1] = v0;
            mesh.normals[vIdx * 3 + 0] = 0.0f;
            mesh.normals[vIdx * 3 + 1] = 1.0f;
            mesh.normals[vIdx * 3 + 2] = 0.0f;
            vIdx++;

            // Вершина 2: (x0, z1) - UV: (u0, v1)
            mesh.vertices[vIdx * 3 + 0] = x0;
            mesh.vertices[vIdx * 3 + 1] = h01;
            mesh.vertices[vIdx * 3 + 2] = z1;
            mesh.texcoords[vIdx * 2 + 0] = u0;
            mesh.texcoords[vIdx * 2 + 1] = v1;
            mesh.normals[vIdx * 3 + 0] = 0.0f;
            mesh.normals[vIdx * 3 + 1] = 1.0f;
            mesh.normals[vIdx * 3 + 2] = 0.0f;
            vIdx++;

            // Вершина 3: (x1, z1) - UV: (u1, v1)
            mesh.vertices[vIdx * 3 + 0] = x1;
            mesh.vertices[vIdx * 3 + 1] = h11;
            mesh.vertices[vIdx * 3 + 2] = z1;
            mesh.texcoords[vIdx * 2 + 0] = u1;
            mesh.texcoords[vIdx * 2 + 1] = v1;
            mesh.normals[vIdx * 3 + 0] = 0.0f;
            mesh.normals[vIdx * 3 + 1] = 1.0f;
            mesh.normals[vIdx * 3 + 2] = 0.0f;
            vIdx++;

            // Индексы треугольников
            int base = vIdx - 4;
            mesh.indices[iIdx++] = base + 0;
            mesh.indices[iIdx++] = base + 2;
            mesh.indices[iIdx++] = base + 1;
            mesh.indices[iIdx++] = base + 1;
            mesh.indices[iIdx++] = base + 2;
            mesh.indices[iIdx++] = base + 3;
        }
    }

    UploadMesh(&mesh, false);
    return mesh;
}

// === ГЛАВНАЯ ФУНКЦИЯ ГЕНЕРАЦИИ ===
void Terrain::generate(int biome)
{
    currentBiome = biome;
    reset();

    // Выбор биома (аналог select l в DBP)
    switch (biome)
    {
    case 1:
        generateGrass();
        break;
    case 2:
        generateMountains();
        break;
    case 3:
        generateDesert();
        break;
    case 4:
        generateFrozen();
        break;
    case 5:
        generateTundra();
        break;
    case 6:
        generateMoon();
        break;
    default:
        generateGrass();
        break;
    }

    calculateTileIndices();
}

// === ПОСТРОЕНИЕ MESH ===
void Terrain::buildMesh()
{
    // Выгрузка старого mesh
    if (model.meshCount > 0)
    {
        UnloadModel(model);
        model = {0};
    }

    // Нормализация высот для heightmap image
    float minH = heights[0][0];
    float maxH = heights[0][0];
    for (int z = 0; z < HEIGHTMAP_SIZE; z++)
    {
        for (int x = 0; x < HEIGHTMAP_SIZE; x++)
        {
            if (heights[z][x] < minH)
                minH = heights[z][x];
            if (heights[z][x] > maxH)
                maxH = heights[z][x];
        }
    }

    float range = maxH - minH;
    if (range == 0.0f)
        range = 1.0f;

    // Создаем кастомный mesh с правильными UV-координатами
    Mesh mesh = createTerrainMesh();
    model = LoadModelFromMesh(mesh);

    // Загружаем текстуру для текущего биома
    char texturePath[64];
    snprintf(texturePath, sizeof(texturePath), "data/terrain/terrain%d.png", currentBiome);

    if (terrainTexture.id != 0)
    {
        UnloadTexture(terrainTexture);
    }

    // Загружаем изображение
    Image terrainImage = LoadImage(texturePath);
    if (terrainImage.data == NULL)
    {
        TraceLog(LOG_WARNING, "Texture not found: %s, using fallback", texturePath);
        terrainImage = GenImageChecked(256, 256, 32, 32, RED, BLUE);
    }

    terrainTexture = LoadTextureFromImage(terrainImage);

    // Оставляем POINT, чтобы пиксели были четкими (это стандарт для тайловых карт)
    SetTextureFilter(terrainTexture, TEXTURE_FILTER_POINT);

    UnloadImage(terrainImage);

    // Применяем текстуру к материалу
    if (model.materials != NULL)
    {
        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = terrainTexture;
    }
}

// === ПОЛУЧЕНИЕ ВЫСОТЫ (аналог get ground height) ===
float Terrain::getHeight(float x, float z) const
{
    // Мировые координаты (0..5000) -> индексы сетки (0..50)
    float gridX = x / 100.0f;
    float gridZ = z / 100.0f;

    int x0 = (int)gridX;
    int z0 = (int)gridZ;
    int x1 = x0 + 1;
    int z1 = z0 + 1;

    // Ограничение границ
    if (x0 < 0)
        x0 = 0;
    if (z0 < 0)
        z0 = 0;
    if (x1 >= HEIGHTMAP_SIZE)
        x1 = HEIGHTMAP_SIZE - 1;
    if (z1 >= HEIGHTMAP_SIZE)
        z1 = HEIGHTMAP_SIZE - 1;

    // Билинейная интерполяция
    float fx = gridX - x0;
    float fz = gridZ - z0;

    float h00 = heights[z0][x0];
    float h10 = heights[z0][x1];
    float h01 = heights[z1][x0];
    float h11 = heights[z1][x1];

    float h0 = h00 * (1.0f - fx) + h10 * fx;
    float h1 = h01 * (1.0f - fx) + h11 * fx;

    return h0 * (1.0f - fz) + h1 * fz;
}

// === ЦВЕТ НЕБА (аналог color backdrop rgb) ===
Color Terrain::getBackdropColor() const
{
    switch (currentBiome)
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

void Terrain::render() const
{
    if (model.meshCount == 0)
        return;

    DrawModel(model, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
}