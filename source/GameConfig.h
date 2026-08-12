#pragma once

// === Экран ===
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

// Фиксированный временной шаг для физики (100 FPS, как в оригинальном DBPro)
constexpr float FIXED_DT = 1.0f / 100.0f;

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
#define MAX_TANK_TYPES 8
#define MAX_BULLETS 55
#define MAX_HIT_EFFECTS 55
#define MAX_EXPLOSIONS 4
#define NUM_PUP 5

// === Камера (DBP: set camera range 6,7450) ===
#define CAMERA_NEAR 6.0f
#define CAMERA_FAR 7100.0f

// === Подразделения (DBP: индексация tk#) ===
#define PLAYER_MIN 1
#define PLAYER_MAX 12
#define ENEMY_MIN 13
#define ENEMY_MAX 40
#define GUEST_MIN 41
#define GUEST_MAX 45
#define EXTRA_MIN 46
#define EXTRA_MAX 50
#define TANKS_MAX 45         // PLAYER + ENEMY + GUEST
#define COMBAT_MAX EXTRA_MAX // PLAYER + ENEMY + GUEST + EXTRA
#define POWERUP_MIN 51
#define POWERUP_MAX 55
#define OBJECTS_MAX POWERUP_MAX

// === Аудио система ===
constexpr int MAX_SOUNDS = 12;
constexpr float NEAREST_TANK_DISTANCE_MAX = 1500.0F;   // Максимальная дистанция поиска ближайшего танка
constexpr float DBPRO_PITCH_BASE = 22050.0f;           // В DBPro sound speed = 22050 Hz для нормы. Raylib pitch: 1.0 = оригинал = wav sample rate
constexpr float MAX_SOUND_DISTANCE = 1500.0f;          // Максимальная дистанция для звуков (из DBPro: rl < 1500)
constexpr float MAX_TREE_COLLISION_DISTANCE = 2500.0f; // Максимальная дистанция для столкновений (rp1 < 2500 для дерева, 1500 для танка)
constexpr float MAX_TANK_COLLISION_DISTANCE = 1500.0f; // Максимальная дистанция для столкновений (rp1 < 2500 для дерева, 1500 для танка)
constexpr float ENGINE_VOLUME_BASE = 85.0f;            // Громкость двигателя игрока: vo# = 85 + rpm# * 4.5, cap 93// Громкость двигателя игрока: vo# = 85 + rpm# * 4.5, cap 93
constexpr float ENGINE_VOLUME_FACTOR = 4.5f;           // Громкость двигателя игрока: vo# = 85 + rpm# * 4.5, cap 93
constexpr float ENGINE_VOLUME_MAX = 93.0f;             // Громкость двигателя игрока: vo# = 85 + rpm# * 4.5, cap 93
constexpr float NEARBY_VOLUME_BASE = 105.0f;           // Громкость ближайшего танка: vol = 105 - distance/15
constexpr float NEARBY_VOLUME_DIVISOR = 15.0f;         // Громкость ближайшего танка: vol = 105 - distance/15
constexpr float ENGINE_PITCH_FACTOR_PLAYER = 3250.0f;  // Pitch двигателя игрока: tk#(n,40) + rpm# * 3250
constexpr float ENGINE_PITCH_FACTOR_NEARBY = 3150.0f;  // Pitch ближайшего танка: tk#(gam(3),40) + tk#(gam(3),38) * 3150
constexpr float ENGINE_PITCH_MIN = DBPRO_PITCH_BASE * 0.3f;
constexpr float ENGINE_PITCH_MAX = DBPRO_PITCH_BASE * 2.8f;
constexpr float COMBAT_VOLUME_BASE = 105.0f;   // Громкость выстрелов/попаданий: vol = 105 - rl/25
constexpr float COMBAT_VOLUME_DIVISOR = 25.0f; // Громкость выстрелов/попаданий: vol = 105 - rl/25
constexpr float COMBAT_VOLUME_MAX = 100.0f;
constexpr float DBPRO_VOLUME_SCALE = 1.0f / 100.0f; // Нормализация громкости DBPro (0-100) в Raylib (0.0-1.0)