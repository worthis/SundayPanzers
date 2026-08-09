#include "AudioSystem.h"
#include "GameConfig.h"
#include <cmath>
#include <algorithm>

AudioSystem::AudioSystem()
    : currentMusic{},
      currentTrack(MusicTrack::NONE),
      musicVolume(0.0f),
      musicMuted(false),
      sndEnginePlayer{},
      sndEngineNearby{},
      sndExplosion{},
      sndBarrierPickup{},
      sndSuperBulletPickup{},
      sndTurbo{},
      sndPlayerDestroyed{},
      sndRepairPickup{},
      sndMenuClick{},
      sndMenuCancel{},
      initialized(false)
{
    for (int i = 0; i < MAX_SOUNDS; i++)
    {
        poolSounds[i] = {};
    }
}

AudioSystem::~AudioSystem()
{
    if (initialized)
    {
        shutdown();
    }
}

void AudioSystem::init()
{
    if (initialized)
        return;

    const char *soundFiles[MAX_SOUNDS] = {
        "data/sound/tank.wav",   // tank engine
        "data/sound/cannon.wav", // tank cannon
        "data/sound/hit.wav",    // hit ground and etc
        "data/sound/can.wav",    // hit tank
        "data/sound/coll.wav",   // collision
        "data/sound/explo.wav",  // explosion
        "data/sound/pup.wav",    // pup - barrier
        "data/sound/pup2.wav",   // pup - super bullet
        "data/sound/repair.wav", // pup - repair
        "data/sound/turbo.wav",  // turbo
        "data/sound/elim.wav",   // destroyed
        "data/sound/menu.wav"    // menu click
    };

    for (int i = 0; i < MAX_SOUNDS; i++)
    {
        poolSounds[i] = LoadSound(soundFiles[i]);
    }

    // Загрузка звука двигателя
    sndEnginePlayer = LoadSoundAlias(poolSounds[static_cast<int>(SoundList::ENGINE)]);
    sndEngineNearby = LoadSoundAlias(poolSounds[static_cast<int>(SoundList::ENGINE)]);

    // Загрузка пула звуков выстрелов (4 канала, как DBPro 3-6)
    poolCannon.resize(4);
    for (int i = 0; i < poolCannon.size(); i++)
    {
        poolCannon[i].sound = LoadSoundAlias(poolSounds[static_cast<int>(SoundList::CANNON)]);
    }

    // Загрузка пула звуков пуль (5 каналов, DBPro 7-11)
    poolBulletHit.resize(5);
    for (int i = 0; i < poolBulletHit.size(); i++)
    {
        poolBulletHit[i].sound = LoadSoundAlias(poolSounds[static_cast<int>(SoundList::HIT)]);
    }

    // Загрузка пула звуков попаданий в танк (4 канала, DBPro 14-17)
    poolTankHit.resize(4);
    for (int i = 0; i < poolTankHit.size(); i++)
    {
        poolTankHit[i].sound = LoadSoundAlias(poolSounds[static_cast<int>(SoundList::HIT2)]);
    }

    // Загрузка пула звуков столкновений (4 канала, DBPro 19-22)
    poolCollision.resize(4);
    for (int i = 0; i < poolCollision.size(); i++)
    {
        poolCollision[i].sound = LoadSoundAlias(poolSounds[static_cast<int>(SoundList::COLLISION)]);
    }

    // Загрузка одноразовых звуков
    sndExplosion = LoadSoundAlias(poolSounds[static_cast<int>(SoundList::EXPLOSION)]);
    sndBarrierPickup = LoadSoundAlias(poolSounds[static_cast<int>(SoundList::BARRIER)]);
    sndSuperBulletPickup = LoadSoundAlias(poolSounds[static_cast<int>(SoundList::SUPERBULLET)]);
    sndRepairPickup = LoadSoundAlias(poolSounds[static_cast<int>(SoundList::REPAIR)]);
    sndTurbo = LoadSoundAlias(poolSounds[static_cast<int>(SoundList::TURBO)]);
    sndPlayerDestroyed = LoadSoundAlias(poolSounds[static_cast<int>(SoundList::DESTROYED)]);
    sndMenuClick = LoadSoundAlias(poolSounds[static_cast<int>(SoundList::MENU)]);
    sndMenuCancel = LoadSoundAlias(poolSounds[static_cast<int>(SoundList::COLLISION)]);

    SetSoundVolume(sndRepairPickup, 0.99f);    // set sound volume 28,99
    SetSoundVolume(sndPlayerDestroyed, 0.99f); // set sound volume 27,99
    SetSoundVolume(sndMenuClick, 0.99f);       // set sound volume 29,99

    initialized = true;
}

void AudioSystem::shutdown()
{
    stopMusic();

    UnloadSoundAlias(sndEnginePlayer);
    UnloadSoundAlias(sndEngineNearby);
    UnloadSoundAlias(sndExplosion);
    UnloadSoundAlias(sndBarrierPickup);
    UnloadSoundAlias(sndSuperBulletPickup);
    UnloadSoundAlias(sndTurbo);
    UnloadSoundAlias(sndPlayerDestroyed);
    UnloadSoundAlias(sndRepairPickup);
    UnloadSoundAlias(sndMenuClick);
    UnloadSoundAlias(sndMenuCancel);

    // Выгрузка пулов
    if (poolCannon.size() > 0)
    {
        for (int i = 0; i < poolCannon.size(); i++)
        {
            UnloadSoundAlias(poolCannon[i].sound);
        }
    }
    poolCannon.clear();

    if (poolBulletHit.size() > 0)
    {
        for (int i = 0; i < poolBulletHit.size(); i++)
        {
            UnloadSoundAlias(poolBulletHit[i].sound);
        }
    }
    poolBulletHit.clear();

    if (poolTankHit.size() > 0)
    {
        for (int i = 0; i < poolTankHit.size(); i++)
        {
            UnloadSoundAlias(poolTankHit[i].sound);
        }
    }
    poolTankHit.clear();

    if (poolCollision.size() > 0)
    {
        for (int i = 0; i < poolCollision.size(); i++)
        {
            UnloadSoundAlias(poolCollision[i].sound);
        }
    }
    poolCollision.clear();

    for (int i = 0; i < MAX_SOUNDS; i++)
    {
        UnloadSoundAlias(poolSounds[i]);
    }

    initialized = false;
}

// ========================================
// Музыка
// ========================================

void AudioSystem::playBattleMusic()
{
    stopMusic();

    // Оригинальная логика: ms = 1 + rnd(399)/100 (значения 1-4)
    int ms = 1 + GetRandomValue(0, 3);

    std::string filename = "data/music/game" + std::to_string(ms) + ".ogg";
    currentMusic = LoadMusicStream(filename.c_str());

    // Оригинальная таблица громкостей:
    // if ms=1 then vom=73
    // if ms=2 then vom=95
    // if ms=3 then vom=84
    // if ms=4 then vom=86
    float volumes[] = {0.0f, 0.73f, 0.95f, 0.84f, 0.86f};
    musicVolume = volumes[ms];

    SetMusicVolume(currentMusic, musicMuted ? 0.0f : musicVolume);
    PlayMusicStream(currentMusic);
    currentTrack = static_cast<MusicTrack>(static_cast<int>(MusicTrack::GAME1) + ms - 1);
}

void AudioSystem::playMenuMusic()
{
    stopMusic();

    // ms = 1 + rnd(199)/100 (значения 1-2)
    int ms = 1 + GetRandomValue(0, 1);
    std::string filename = "data/music/menu" + std::to_string(ms) + ".ogg";
    currentMusic = LoadMusicStream(filename.c_str());
    musicVolume = 0.95f; // set music volume 1,95

    SetMusicVolume(currentMusic, musicMuted ? 0.0f : musicVolume);
    PlayMusicStream(currentMusic);
    currentTrack = static_cast<MusicTrack>(static_cast<int>(MusicTrack::MENU1) + ms - 1);
}

void AudioSystem::playIntroMusic()
{
    stopMusic();
    currentMusic = LoadMusicStream("data/music/intro.ogg");
    musicVolume = 0.95f;
    SetMusicVolume(currentMusic, musicMuted ? 0.0f : musicVolume);
    PlayMusicStream(currentMusic);
    currentTrack = MusicTrack::INTRO;
}

void AudioSystem::playEndMusic()
{
    stopMusic();
    currentMusic = LoadMusicStream("data/music/end.ogg");
    musicVolume = 1.0f; // set music volume 1,100
    SetMusicVolume(currentMusic, musicMuted ? 0.0f : musicVolume);
    PlayMusicStream(currentMusic);
    currentTrack = MusicTrack::END;
}

void AudioSystem::stopMusic()
{
    if (currentTrack != MusicTrack::NONE)
    {
        StopMusicStream(currentMusic);
        UnloadMusicStream(currentMusic);
        currentMusic = {};
        currentTrack = MusicTrack::NONE;
    }
}

void AudioSystem::updateMusic(float dt)
{
    if (currentTrack != MusicTrack::NONE)
    {
        UpdateMusicStream(currentMusic);
    }
}

void AudioSystem::toggleMusicMute()
{
    musicMuted = !musicMuted;
    if (currentTrack != MusicTrack::NONE)
    {
        // В оригинале: if muxyc=1 then set music volume 1,0
        SetMusicVolume(currentMusic, musicMuted ? 0.0f : musicVolume);
    }
}

// ========================================
// Двигатели танков
// ========================================

// Обновляет звуки двигателя танка игрока.
// Оригинальная формула:
// rpm = abs(tk#(n,38))
// vo = 85 + rpm * 4.5  (clamped to 93)
// pitch = tk#(n,40) + rpm * 3250
void AudioSystem::updatePlayerEngine(float playerTankRPM, float playerTankEnergy, float playerTankSoundStart, bool isChangingCamera)
{
    if (!initialized)
        return;

    if (playerTankEnergy <= 0 || isChangingCamera)
    {
        // if tk#(n,0)<0 then set sound volume 1,0
        // if gam(1)=0 then vo#=0 (changing camera)
        StopSound(sndEnginePlayer);
        return;
    }

    float pitch = calculatePitch(playerTankRPM, playerTankSoundStart, true);

    // vo# = 85 + rpm# * 4.5; if vo# > 93 then vo# = 93
    float volume = ENGINE_VOLUME_BASE + std::fabs(playerTankRPM) * ENGINE_VOLUME_FACTOR;
    if (volume > ENGINE_VOLUME_MAX)
        volume = ENGINE_VOLUME_MAX;
    if (volume < 0)
        volume = 0;
    volume *= DBPRO_VOLUME_SCALE;

    SetSoundVolume(sndEnginePlayer, volume);
    SetSoundPitch(sndEnginePlayer, pitch);

    if (!IsSoundPlaying(sndEnginePlayer))
    {
        PlaySound(sndEnginePlayer);
    }
}

// Обновляет звук ближайшего танка.
// Оригинальная формула:
// sps = tk#(gam(3),40) + tk#(gam(3),38) * 3150
// vol = 105 - distance/15
void AudioSystem::updateNearbyEngine(float nearbyTankRPM, float nearbyTankSoundStart, float distanceToNearby)
{
    if (!initialized)
        return;

    float pitch = calculatePitch(nearbyTankRPM, nearbyTankSoundStart);

    // vol = 105 - gam(2)/15 (gam(2) = дистанция до ближайшего)
    float volume = NEARBY_VOLUME_BASE - distanceToNearby / NEARBY_VOLUME_DIVISOR;
    if (volume > ENGINE_VOLUME_MAX)
        volume = ENGINE_VOLUME_MAX;
    if (volume < 0)
        volume = 0;
    volume *= DBPRO_VOLUME_SCALE;

    SetSoundVolume(sndEngineNearby, volume);
    SetSoundPitch(sndEngineNearby, pitch);

    if (!IsSoundPlaying(sndEngineNearby))
    {
        PlaySound(sndEngineNearby);
    }
}

float AudioSystem::calculatePitch(float rpm, float basePitch, bool isPlayer) const
{
    // В DBPro: set sound speed 1, tk#(n,40) + rpm# * 3250 / 3150
    // sound speed в Hz. Raylib pitch = множитель (1.0 = оригинальный pitch).
    // DBPro default = 22000 Hz = pitch 1.0
    float rpmabs = std::fabs(rpm);
    float hz = basePitch + rpmabs * (isPlayer ? ENGINE_PITCH_FACTOR_PLAYER : ENGINE_PITCH_FACTOR_NEARBY);
    if (hz < ENGINE_PITCH_MIN)
        hz = ENGINE_PITCH_MIN;
    if (hz > ENGINE_PITCH_MAX)
        hz = ENGINE_PITCH_MAX;
    return hz / DBPRO_PITCH_BASE;
}

// ========================================
// Одноразовые звуки
// ========================================

float AudioSystem::calculateDistanceVolume(float distance, float maxDistance) const
{
    // vol = 105 - rl/25
    if (distance >= maxDistance)
        return 0.0f;

    float vol = COMBAT_VOLUME_BASE - distance / COMBAT_VOLUME_DIVISOR;
    vol = std::max(0.0f, std::min(vol, COMBAT_VOLUME_MAX));
    return vol * DBPRO_VOLUME_SCALE;
}

int AudioSystem::findFreeChannel(std::vector<SoundChannel> &pool) const
{
    // В DBPro: for s=3 to 6; if sound playing(s)=0 and so=0 then so=s
    for (int i = 0; i < static_cast<int>(pool.size()); i++)
    {
        if (!IsSoundPlaying(pool[i].sound))
        {
            return i;
        }
    }
    return -1; // Все каналы заняты
}

void AudioSystem::playCannonShot(const Vector3 &position)
{
    if (!initialized)
        return;

    float dx = position.x - playerPosX;
    float dz = position.z - playerPosZ;
    float dy = position.y - playerPosY;
    float distance = std::sqrt(dx * dx + dz * dz + dy * dy);

    // Игрок всегда имеет свободный канал (канал 2 в DBPro)
    // Для других: ищем свободный в пуле (каналы 3-6)
    int idx = findFreeChannel(poolCannon);
    if (idx < 0)
        return;

    float volume = calculateDistanceVolume(distance);
    if (volume <= 0.0f)
        return;

    SetSoundVolume(poolCannon[idx].sound, volume);
    SetSoundPitch(poolCannon[idx].sound, 1.0f + GetRandomValue(-5, 5) * 0.01f);
    PlaySound(poolCannon[idx].sound);
}

void AudioSystem::playGroundHit(const Vector3 &position)
{
    if (!initialized)
        return;

    float dx = position.x - playerPosX;
    float dz = position.z - playerPosZ;
    float dy = position.y - playerPosY;
    float distance = std::sqrt(dx * dx + dz * dz + dy * dy);

    int idx = findFreeChannel(poolBulletHit); // каналы 7-11
    if (idx < 0)
        return;

    float volume = calculateDistanceVolume(distance);
    if (volume <= 0.0f)
        return;

    SetSoundVolume(poolBulletHit[idx].sound, volume);
    PlaySound(poolBulletHit[idx].sound);
}

void AudioSystem::playTankHit(const Vector3 &position)
{
    if (!initialized)
        return;

    float dx = position.x - playerPosX;
    float dz = position.z - playerPosZ;
    float dy = position.y - playerPosY;
    float distance = std::sqrt(dx * dx + dz * dz + dy * dy);

    int idx = findFreeChannel(poolTankHit); // каналы 14-17
    if (idx < 0)
        return;

    float volume = calculateDistanceVolume(distance);
    if (volume <= 0.0f)
        return;

    SetSoundVolume(poolTankHit[idx].sound, volume);
    PlaySound(poolTankHit[idx].sound);
}

void AudioSystem::playCollision(const Vector3 &position, bool isTank)
{
    if (!initialized)
        return;

    float dx = position.x - playerPosX;
    float dz = position.z - playerPosZ;
    float dy = position.y - playerPosY;
    float distance = std::sqrt(dx * dx + dz * dz + dy * dy);

    int idx = findFreeChannel(poolCollision); // каналы 19-22
    if (idx < 0)
        return;

    float volume = calculateDistanceVolume(distance, isTank ? MAX_TANK_COLLISION_DISTANCE : MAX_TREE_COLLISION_DISTANCE);
    if (volume <= 0.0f)
        return;

    SetSoundVolume(poolCollision[idx].sound, volume);
    PlaySound(poolCollision[idx].sound);
}

void AudioSystem::playTankExplosion(const Vector3 &position)
{
    if (!initialized)
        return;

    float dx = position.x - playerPosX;
    float dz = position.z - playerPosZ;
    float dy = position.y - playerPosY;
    float distance = std::sqrt(dx * dx + dz * dz + dy * dy);

    if (distance >= MAX_SOUND_DISTANCE)
        return;

    float volume = calculateDistanceVolume(distance);
    SetSoundVolume(sndExplosion, volume);
    PlaySound(sndExplosion);
}

void AudioSystem::playPlayerDestroyed()
{
    if (!initialized)
        return;

    // if c<13 then play sound 27
    SetSoundVolume(sndPlayerDestroyed, 0.99f);
    PlaySound(sndPlayerDestroyed);
}

void AudioSystem::playRepairPickup(const Vector3 &position)
{
    if (!initialized)
        return;

    float dx = position.x - playerPosX;
    float dz = position.z - playerPosZ;
    float dy = position.y - playerPosY;
    float distance = std::sqrt(dx * dx + dz * dz + dy * dy);

    if (distance >= MAX_SOUND_DISTANCE)
        return;

    float volume = 105.0f - distance / 25.0f;
    volume = std::min(volume, 100.0f);
    SetSoundVolume(sndRepairPickup, volume * DBPRO_VOLUME_SCALE);
    PlaySound(sndRepairPickup);
}

void AudioSystem::playTurbo(const Vector3 &position)
{
    if (!initialized)
        return;

    float dx = position.x - playerPosX;
    float dz = position.z - playerPosZ;
    float dy = position.y - playerPosY;
    float distance = std::sqrt(dx * dx + dz * dz + dy * dy);

    // В оригинале: if rp1 < 1500 then play sound
    if (distance >= MAX_SOUND_DISTANCE)
        return;

    float volume = 105.0f - distance / 25.0f;
    volume = std::min(volume, 100.0f);
    SetSoundVolume(sndTurbo, volume * DBPRO_VOLUME_SCALE);
    PlaySound(sndTurbo);
}

void AudioSystem::playBarrierPickup(const Vector3 &position)
{
    if (!initialized)
        return;

    float dx = position.x - playerPosX;
    float dz = position.z - playerPosZ;
    float dy = position.y - playerPosY;
    float distance = std::sqrt(dx * dx + dz * dz + dy * dy);

    // В оригинале: if rp1 < 1500 then play sound
    if (distance >= MAX_SOUND_DISTANCE)
        return;

    float volume = 105.0f - distance / 25.0f;
    volume = std::min(volume, 100.0f);
    SetSoundVolume(sndBarrierPickup, volume * DBPRO_VOLUME_SCALE);
    PlaySound(sndBarrierPickup);
}

void AudioSystem::playSuperBulletPickup(const Vector3 &position)
{
    if (!initialized)
        return;

    float dx = position.x - playerPosX;
    float dz = position.z - playerPosZ;
    float dy = position.y - playerPosY;
    float distance = std::sqrt(dx * dx + dz * dz + dy * dy);

    if (distance >= MAX_SOUND_DISTANCE)
        return;

    float volume = 105.0f - distance / 25.0f;
    volume = std::min(volume, 100.0f);
    SetSoundVolume(sndSuperBulletPickup, volume * DBPRO_VOLUME_SCALE);
    PlaySound(sndSuperBulletPickup);
}

void AudioSystem::playMenuClick()
{
    if (!initialized)
        return;

    PlaySound(sndMenuClick);
}

void AudioSystem::playMenuCancel()
{
    if (!initialized)
        return;
        
    PlaySound(sndMenuCancel);
}

void AudioSystem::update(float dt)
{
    updateMusic(dt);
}

void AudioSystem::updatePlayerPos(float x, float y, float z)
{
    playerPosX = x;
    playerPosY = y;
    playerPosZ = z;
}