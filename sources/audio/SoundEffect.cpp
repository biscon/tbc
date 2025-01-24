//
// Created by bison on 24-01-25.
//

#include "SoundEffect.h"
#include <cstdlib>  // For rand()
#include <ctime>    // For seeding random generator

struct SoundEffect {
    Sound sound;
    float restartDelay;
    float restartTimer;
    bool loop;
    bool playing;
    float delayTimer; // Timer for initial delay
};

struct SoundEffectGroup {
    SoundEffectType type;
    std::vector<std::unique_ptr<SoundEffect>> effects; // Group of effects for the same type
};

struct SoundEffectManager {
    std::vector<std::unique_ptr<SoundEffectGroup>> groups;
};

static SoundEffectManager manager;

void InitSoundEffectManager() {
    std::srand(static_cast<unsigned>(std::time(nullptr))); // Seed random generator
}

void DestroySoundEffectManager() {
    for (auto &group : manager.groups) {
        for (auto &effect : group->effects) {
            UnloadSound(effect->sound);
        }
        group->effects.clear();
    }
    manager.groups.clear();
}

void LoadSoundEffect(SoundEffectType type, const char* filename, bool loop, float restartDelay) {
    // Find the group for the specified type or create a new one
    SoundEffectGroup* group = nullptr;
    for (auto &existingGroup : manager.groups) {
        if (existingGroup->type == type) {
            group = existingGroup.get();
            break;
        }
    }
    if (!group) {
        group = new SoundEffectGroup();
        group->type = type;
        manager.groups.push_back(std::unique_ptr<SoundEffectGroup>(group));
    }

    // Create a new SoundEffect and add it to the group
    SoundEffect* effect = new SoundEffect();
    effect->sound = LoadSound(filename);
    effect->loop = loop;
    effect->restartDelay = restartDelay;
    effect->restartTimer = restartDelay;
    effect->delayTimer = 0.0f; // Initialize delay timer to 0
    effect->playing = false;
    group->effects.push_back(std::unique_ptr<SoundEffect>(effect));
}

void PlaySoundEffect(SoundEffectType type, float delay) {
    // Find the group for the specified type
    for (auto &group : manager.groups) {
        if (group->type == type) {
            if (group->effects.empty()) {
                return; // No effects loaded for this type
            }

            // Pick a random effect from the group
            int randomIndex = rand() % group->effects.size();
            SoundEffect* effect = group->effects[randomIndex].get();

            if (!IsSoundPlaying(effect->sound)) {
                effect->delayTimer = delay; // Set the delay timer
                effect->playing = true;
                if (delay <= 0.0f) {
                    PlaySound(effect->sound); // Play sound immediately
                }
            }
            break;
        }
    }
}

void StopSoundEffect(SoundEffectType type) {
    // Find the group for the specified type
    for (auto &group : manager.groups) {
        if (group->type == type) {
            for (auto &effect : group->effects) {
                StopSound(effect->sound);
                effect->playing = false;
                effect->delayTimer = 0.0f; // Reset delay timer
            }
            break;
        }
    }
}

void SetLoopSoundEffect(SoundEffectType type, bool loop) {
    // Find the group for the specified type
    for (auto &group : manager.groups) {
        if (group->type == type) {
            for (auto &effect : group->effects) {
                effect->loop = loop;
            }
            break;
        }
    }
}

void UpdateSoundEffects(float dt) {
    for (auto &group : manager.groups) {
        for (auto &effect : group->effects) {
            if (effect->playing) {
                // Handle delay timer
                if (effect->delayTimer > 0.0f) {
                    effect->delayTimer -= dt;
                    if (effect->delayTimer <= 0.0f) {
                        PlaySound(effect->sound); // Play sound after delay
                        effect->restartTimer = effect->restartDelay; // Reset restart timer
                    }
                    continue; // Skip further processing until delay finishes
                }

                // Handle looping sounds
                if (effect->loop) {
                    if (!IsSoundPlaying(effect->sound)) {
                        effect->restartTimer -= dt;
                        if (effect->restartTimer <= 0.0f) {
                            PlaySound(effect->sound);
                            effect->restartTimer = effect->restartDelay;
                        }
                    }
                }
            }
        }
    }
}

void SetVolumeSoundEffect(SoundEffectType type, float volume) {
    for (auto &group : manager.groups) {
        if (group->type == type) {
            for (auto &effect : group->effects) {
                SetSoundVolume(effect->sound, volume);
            }
            break;
        }
    }
}
