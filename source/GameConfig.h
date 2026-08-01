#pragma once

// === Экран ===
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

// === Карта (DBP: make matrix 1,5000,5000,50,50) ===
#define MAP_SIZE 5000.0f
#define MAP_SEGMENTS 50
#define HEIGHTMAP_SIZE 51
#define CELL_SIZE (MAP_SIZE / MAP_SEGMENTS) // 100.0f
#define MAP_CENTER (MAP_SIZE / 2.0f)        // 2500.0f

// === Лимиты карты (DBP) ===
#define MAP_LIMIT_MIN 470.0f // облака
#define MAP_LIMIT_MAX 4530.0f
#define TANK_LIMIT_MIN 370.0f // танки
#define TANK_LIMIT_MAX 4630.0f

// === Объекты (DBP) ===
#define MAX_TREES 75
#define MAX_CLOUDS 10
#define NUM_BIOMES 6

// === Камера (DBP: set camera range 6,7450) ===
#define CAMERA_NEAR 0.1f
#define CAMERA_FAR 15000.0f

// === Танки ===
#define MAX_TANKS 56
#define MAX_TANK_TYPES 8

#define MAX_BULLETS     55
#define MAX_HIT_EFFECTS 55
#define MAX_EXPLOSIONS  4