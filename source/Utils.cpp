#include "Utils.h"
#include <cmath>
#include <cfloat>

int rnd(int n)
{
    if (n <= 0)
        return 0;
    return GetRandomValue(0, n - 1);
}

float sinDeg(float ang)
{
    return sinf(ang * DEG2RAD);
}

float cosDeg(float ang)
{
    return cosf(ang * DEG2RAD);
}

float atanDeg(float v)
{
    return atanf(v) * RAD2DEG;
}

float wrapValue(float angle)
{
    angle = fmodf(angle, 360.0f);
    if (angle < 0.0f)
        angle += 360.0f;
    return angle;
}

float newXValue(float x, float angle, float dist)
{
    return x + sinDeg(angle) * dist;
}

float newZValue(float z, float angle, float dist)
{
    return z + cosDeg(angle) * dist;
}

Vector3 ComputeMeshCenter(const Mesh &mesh)
{
    if (mesh.vertexCount == 0)
        return {0, 0, 0};

    float minX = FLT_MAX, minY = FLT_MAX, minZ = FLT_MAX;
    float maxX = -FLT_MAX, maxY = -FLT_MAX, maxZ = -FLT_MAX;

    for (int i = 0; i < mesh.vertexCount; i++)
    {
        float x = mesh.vertices[i * 3 + 0];
        float y = mesh.vertices[i * 3 + 1];
        float z = mesh.vertices[i * 3 + 2];

        if (x < minX)
            minX = x;
        if (x > maxX)
            maxX = x;
        if (y < minY)
            minY = y;
        if (y > maxY)
            maxY = y;
        if (z < minZ)
            minZ = z;
        if (z > maxZ)
            maxZ = z;
    }

    return {(minX + maxX) * 0.5f, (minY + maxY) * 0.5f, (minZ + maxZ) * 0.5f};
}

// ============================================================
// Загрузка текстуры с DBP color key (чёрный → прозрачный)
// DBP: load image "file.bmp", N, 1
// ============================================================
Texture2D LoadTextureColorKey(const char *path)
{
    Image img = LoadImage(path);
    if (img.data == nullptr)
    {
        TraceLog(LOG_WARNING, "Texture not found: %s", path);
        return LoadTextureFromImage(img); // вернёт пустую
    }

    // DBP хранит как 32-bit RGBA после color key
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    // Color key: тёмные пиксели → alpha = 0
    Color *pixels = static_cast<Color *>(img.data);
    int count = img.width * img.height;
    for (int i = 0; i < count; i++)
    {
        if (pixels[i].r < 16 && pixels[i].g < 16 && pixels[i].b < 16)
            pixels[i].a = 0;
    }

    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}