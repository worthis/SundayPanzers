#pragma once
#include "raylib.h"
#include "GameConfig.h"
#include <string>
#include <vector>
#include <array>

/**
 * AudioSystem - точный порт звуковой системы из DBPro.
 *
 * В оригинале DBPro использует до 30 каналов звука одновременно.
 * Raylib ограничен, поэтому мы эмулируем это через пул Sound каналов.
 *
 * Ключевые фичи оригинала:
 * - Двигатель танка меняет PITCH в зависимости от RPM
 * - Двигатель ближайшего танка меняет VOLUME по дистанции
 * - Пулинг каналов для повторяющихся звуков (выстрелы, попадания)
 * - Отдельный трек для музыки с переключением (клавиша M)
 */
class AudioSystem
{
public:
    // Музыкальные треки
    enum class MusicTrack
    {
        NONE,
        INTRO,
        MENU1,
        MENU2,
        GAME1,
        GAME2,
        GAME3,
        GAME4,
        END
    };

    // Игровые звуки
    enum class SoundList
    {
        ENGINE,
        CANNON,
        HIT,
        HIT2,
        COLLISION,
        EXPLOSION,
        BARRIER,
        SUPERBULLET,
        REPAIR,
        DESTROYED,
        TURBO,
        MENU
    };

    AudioSystem();
    ~AudioSystem();

    void init();
    void shutdown();

    // Музыка
    void playBattleMusic(); // ms=1+rnd(399)/100, volume по таблице
    void playMenuMusic();   // ms=1+rnd(199)/100
    void playIntroMusic();  // intro.ogg
    void playEndMusic();    // end.ogg
    void stopMusic();
    void updateMusic(float dt); // UpdateMusicStream + обработка Mute
    void toggleMusicMute();     // Клавиша M (keystate(50))
    bool isMusicMuted() const { return musicMuted; }

    // Звуки двигателя (2 канала)
    void updatePlayerEngine(float playerTankRPM, float playerTankEnergy, float playerTankSoundStart, bool isChangingCamera);
    void updateNearbyEngine(float nearbyTankRPM, float nearbyTankSoundStart, float distanceToNearby);

    // Одноразовые звуки с пуллингом каналов
    void playCannonShot(const Vector3 &position);
    void playGroundHit(const Vector3 &position);
    void playTankHit(const Vector3 &position);
    void playCollision(const Vector3 &position, bool isTank = false);
    void playTankExplosion(const Vector3 &position);
    void playPlayerDestroyed();              // Звук 27 (elim.wav)
    void playTurbo(const Vector3 &position); // Звук 26 (turbo.wav)
    void playRepairPickup(const Vector3 &position);
    void playBarrierPickup(const Vector3 &position);
    void playSuperBulletPickup(const Vector3 &position);
    void playMenuClick();  // Звук 29 (menu.wav)
    void playMenuCancel(); // Звук 18 (coll.wav)

    void update(float dt);                           // Главный метод обновления (вызывать в UpdateBattle)
    void updatePlayerPos(float x, float y, float z); // Обновления позиции танка игрока для обработки звуков в пространстве

private:
    struct SoundChannel
    {
        Sound sound;
        bool active = false;
        float volume = 1.0f;
    };

    float playerPosX = 0.0f;
    float playerPosY = 0.0f;
    float playerPosZ = 0.0f;

    // Вспомогательные методы
    float calculateDistanceVolume(float distance, float maxDistance = 1500.0f) const;
    float calculatePitch(float rpm, float basePitch, bool isPlayer = false) const;
    int findFreeChannel(std::vector<SoundChannel> &pool) const;

    // Музыка
    Music currentMusic;
    MusicTrack currentTrack;
    float musicVolume; // vom из DBPro (73, 95, 84, 86)
    bool musicMuted;

    Sound poolSounds[MAX_SOUNDS]; // оригиналы всех звуков

    // Пулы звуков (эмуляция каналов DBPro)
    std::vector<SoundChannel> poolCannon;    // 4 канала, DBPro каналы 3-6: выстрелы (cannon)
    std::vector<SoundChannel> poolBulletHit; // 5 каналов, DBPro каналы 7-11: пуля в землю (hit)
    std::vector<SoundChannel> poolTankHit;   // 4 канала, DBPro каналы 14-17: попадание в танк (can)
    std::vector<SoundChannel> poolCollision; // 4 канала, DBPro каналы 19-22: столкновения (coll)

    // Звуки двигателя (всегда играют)
    Sound sndEnginePlayer; // Звук 1 и 12 (tank.wav)
    Sound sndEngineNearby; // Звук 1 и 12 (tank.wav)
    
    // Одноразовые звуки
    Sound sndExplosion;         // explo.wav (канал 23)
    Sound sndBarrierPickup;     // pup.wav (канал 24)
    Sound sndSuperBulletPickup; // pup2.wav (канал 25)
    Sound sndTurbo;             // turbo.wav (канал 26)
    Sound sndPlayerDestroyed;   // elim.wav (канал 27)
    Sound sndRepairPickup;      // repair.wav (канал 28)
    Sound sndMenuClick;         // menu.wav (канал 29)
    Sound sndMenuCancel;        // coll.wav (канал 18)

    bool initialized;
};