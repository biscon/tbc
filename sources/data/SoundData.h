//
// Created by bison on 09-09-25.
//

#ifndef SANDBOX_SOUNDDATA_H
#define SANDBOX_SOUNDDATA_H

#include <unordered_map>
#include <vector>
#include <string>
#include "raylib.h"

#define MAX_ALIASES 64

struct SoundAlias {
    Sound alias;
    bool inUse;
    bool looping;
    float volume;
    float restartDelay;
    float restartTimer;
    std::string tag;
};

struct SoundEntry {
    Sound sound;        // owns audio buffer
    float volume;       // default volume
    float restartDelay; // default restart delay
    std::string tag; // optional, empty = global
};

struct SoundGroup {
    std::vector<SoundEntry> entries; // variations
};

struct SoundData {
    std::unordered_map<std::string, SoundGroup> groups;
    SoundAlias aliases[MAX_ALIASES];
};

#endif //SANDBOX_SOUNDDATA_H
