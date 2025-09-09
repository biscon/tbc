#include "Sound.h"
#include "raylib.h"
#include "util/json.hpp"
#include "util/FileUtil.h"
#include <string>
#include <unordered_map>
#include <fstream>
#include <cstdlib>

#define MAX_ALIASES 64

//----------------------------------------------------
// Core management
//----------------------------------------------------
void InitSoundData(SoundData& sd, const std::string &filename) {
    sd.groups.clear();
    for (int i = 0; i < MAX_ALIASES; i++) {
        sd.aliases[i].inUse = false;
    }

    std::ifstream file(filename);
    if (!file) {
        TraceLog(LOG_ERROR, "No sound data file found: %s", filename.c_str());
        std::abort();
    }

    nlohmann::json j;
    file >> j;

    if (!j.is_array()) {
        TraceLog(LOG_ERROR, "Invalid sound JSON format: expected array at root");
        std::abort();
    }

    for (const auto& entry : j) {
        if (!entry.contains("groupId") || !entry.contains("file")) {
            TraceLog(LOG_WARNING, "Skipping invalid sound entry (missing groupId/file)");
            continue;
        }

        std::string groupId = entry.value("groupId", "");
        std::string filePath = entry.value("file", "");
        std::string tag = entry.value("tag", ""); // optional tag
        std::string fullPath = ResolveRelativeToCwd(filename, filePath).string();
        float volume = entry.value("volume", 1.0f);
        float restartDelay = entry.value("restartDelay", 0.0f);

        if (groupId.empty() || filePath.empty()) {
            TraceLog(LOG_WARNING, "Skipping invalid sound entry (empty groupId/file)");
            continue;
        }

        // Load the actual sound
        Sound snd = LoadSound(fullPath.c_str());
        if (snd.frameCount == 0) {
            TraceLog(LOG_ERROR, "Failed to load sound file: %s", fullPath.c_str());
            continue;
        }

        SoundEntry se{};
        se.sound = snd;
        se.volume = volume;
        se.restartDelay = restartDelay;
        se.tag = tag;

        sd.groups[groupId].entries.push_back(se);

        TraceLog(LOG_INFO, "Loaded sound %s into group %s (vol=%.2f, delay=%.3f, tag=%s)",
                 filePath.c_str(), groupId.c_str(), volume, restartDelay, tag.c_str());
    }
}

// Add a sound file into a named group with optional tag
bool AddSoundToGroup(SoundData& sd, const std::string& groupId, const std::string& filename,
                     float volume, float restartDelay, const std::string& tag) {
    SoundEntry entry{};
    entry.sound = LoadSound(filename.c_str());
    entry.volume = volume;
    entry.restartDelay = restartDelay;
    entry.tag = tag;

    sd.groups[groupId].entries.push_back(entry);
    return true;
}

//----------------------------------------------------
// Playback
//----------------------------------------------------
int PlaySfx(SoundData& sd, const std::string& groupId, bool loop, float startDelay, int index) {
    auto it = sd.groups.find(groupId);
    if (it == sd.groups.end() || it->second.entries.empty()) return -1;

    const auto& group = it->second;
    int chosenIndex = index;
    if (chosenIndex < 0 || chosenIndex >= (int)group.entries.size()) {
        chosenIndex = GetRandomValue(0, (int)group.entries.size() - 1);
    }
    const SoundEntry& entry = group.entries[chosenIndex];

    for (int i = 0; i < MAX_ALIASES; i++) {
        if (!sd.aliases[i].inUse || !IsSoundPlaying(sd.aliases[i].alias)) {
            if (!sd.aliases[i].inUse) {
                sd.aliases[i].alias = LoadSoundAlias(entry.sound);
                sd.aliases[i].inUse = true;
            }

            sd.aliases[i].looping = loop;
            sd.aliases[i].volume = entry.volume;
            sd.aliases[i].restartDelay = entry.restartDelay;
            sd.aliases[i].restartTimer = startDelay;
            sd.aliases[i].tag = entry.tag; // store tag for later stopping/unloading

            if (startDelay <= 0.0f) {
                PlaySound(sd.aliases[i].alias);
                SetSoundVolume(sd.aliases[i].alias, entry.volume);
                sd.aliases[i].restartTimer = entry.restartDelay;
            }

            return i;
        }
    }

    return -1;
}

//----------------------------------------------------
// Control
//----------------------------------------------------
void StopSfx(SoundData& sd, int handle) {
    if (handle < 0 || handle >= MAX_ALIASES) return;
    if (sd.aliases[handle].inUse) {
        StopSound(sd.aliases[handle].alias);
        sd.aliases[handle].inUse = false;
    }
}

void SetSfxVolume(SoundData& sd, int handle, float volume) {
    if (handle < 0 || handle >= MAX_ALIASES) return;
    if (sd.aliases[handle].inUse) {
        sd.aliases[handle].volume = volume;
        SetSoundVolume(sd.aliases[handle].alias, volume);
    }
}

//----------------------------------------------------
// Update (handle looping with restart delay)
//----------------------------------------------------
void UpdateSoundData(SoundData& sd, float dt) {
    for (int i = 0; i < MAX_ALIASES; i++) {
        if (sd.aliases[i].inUse) {
            sd.aliases[i].restartTimer -= dt;

            if (!IsSoundPlaying(sd.aliases[i].alias)) {
                if (sd.aliases[i].looping) {
                    if (sd.aliases[i].restartTimer <= 0.0f) {
                        PlaySound(sd.aliases[i].alias);
                        SetSoundVolume(sd.aliases[i].alias, sd.aliases[i].volume);
                        sd.aliases[i].restartTimer = sd.aliases[i].restartDelay;
                    }
                } else {
                    if (sd.aliases[i].restartTimer <= 0.0f) {
                        PlaySound(sd.aliases[i].alias);
                        SetSoundVolume(sd.aliases[i].alias, sd.aliases[i].volume);
                        sd.aliases[i].restartTimer = -1.0f;
                    }
                }
            }
        }
    }
}

//----------------------------------------------------
// Stop all sounds (optionally by tag)
//----------------------------------------------------
void StopAllSounds(SoundData& sd, const std::string& tag) {
    for (int i = 0; i < MAX_ALIASES; i++) {
        if (sd.aliases[i].inUse) {
            if (tag.empty() || sd.aliases[i].tag == tag) {
                StopSound(sd.aliases[i].alias);
                sd.aliases[i].inUse = false;
            }
        }
    }
}

void StopAllLooping(SoundData& sd) {
    for (int i = 0; i < MAX_ALIASES; i++) {
        if (sd.aliases[i].inUse && sd.aliases[i].looping) {
            StopSound(sd.aliases[i].alias);
            sd.aliases[i].inUse = false;
        }
    }
}

//----------------------------------------------------
// Unload all sounds with a given tag
//----------------------------------------------------
void UnloadTaggedSounds(SoundData& sd, const std::string& tag) {
    for (auto& kv : sd.groups) {
        auto& vec = kv.second.entries;
        for (auto it = vec.begin(); it != vec.end();) {
            if (it->tag == tag) {
                UnloadSound(it->sound);
                it = vec.erase(it);
            } else {
                ++it;
            }
        }
    }
    // Stop any active aliases with the tag
    StopAllSounds(sd, tag);
}

//----------------------------------------------------
// Cleanup all
//----------------------------------------------------
void UnloadAllSounds(SoundData& sd) {
    for (int i = 0; i < MAX_ALIASES; i++) {
        if (sd.aliases[i].inUse) {
            UnloadSoundAlias(sd.aliases[i].alias);
            sd.aliases[i].inUse = false;
        }
    }
    for (auto& kv : sd.groups) {
        for (SoundEntry& e : kv.second.entries) {
            UnloadSound(e.sound);
        }
    }
    sd.groups.clear();
}
