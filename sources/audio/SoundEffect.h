//
// Created by bison on 24-01-25.
//

#ifndef SANDBOX_SOUNDEFFECT_H
#define SANDBOX_SOUNDEFFECT_H

#include <vector>
#include <memory>
#include "raylib.h"

enum class SoundEffectType {
    MeleeHit,
    MeleeCrit,
    MeleeMiss,
    Footstep,
    Select,
    HumanPain,
    HumanDeath,
    Ambience,
    Victory,
    Defeat,
    Burning,
    StartRound,
};

void InitSoundEffectManager();
void DestroySoundEffectManager();
void LoadSoundEffect(SoundEffectType type, const char* filename, bool loop, float restartDelay = 0.0f);
void PlaySoundEffect(SoundEffectType type, float delay = 0.0f);
void StopSoundEffect(SoundEffectType type);
void SetLoopSoundEffect(SoundEffectType type, bool loop);
void UpdateSoundEffects(float dt);
void SetVolumeSoundEffect(SoundEffectType type, float volume);

#endif //SANDBOX_SOUNDEFFECT_H
