//
// Created by bison on 09-09-25.
//

#ifndef SANDBOX_SOUND_H
#define SANDBOX_SOUND_H

#include "data/SoundData.h"

#include "raylib.h"
#include <string>
#include <unordered_map>
#include <vector>

//----------------------------------------------------
// Core management
//----------------------------------------------------
void InitSoundData(SoundData& sd, const std::string &filename);
bool AddSoundToGroup(SoundData& sd, const std::string& groupId, const std::string& filename,
                     float volume, float restartDelay, const std::string& tag = "");

//----------------------------------------------------
// Playback
//----------------------------------------------------
int PlaySfx(SoundData& sd, const std::string& groupId, bool loop = false, float startDelay = 0.0f, int index = -1);

//----------------------------------------------------
// Control
//----------------------------------------------------
void StopSfx(SoundData& sd, int handle);
void SetSfxVolume(SoundData& sd, int handle, float volume);

//----------------------------------------------------
// Update
//----------------------------------------------------
void UpdateSoundData(SoundData& sd, float dt);
void StopAllSounds(SoundData& sd, const std::string& tag = "");
void StopAllLooping(SoundData& sd);

//----------------------------------------------------
// Unload
//----------------------------------------------------
void UnloadTaggedSounds(SoundData& sd, const std::string& tag);
void UnloadAllSounds(SoundData& sd);

#endif //SANDBOX_SOUND_H
