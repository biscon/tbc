//
// Created by bison on 13-03-25.
//

#ifndef SANDBOX_STATUSEFFECTDATA_H
#define SANDBOX_STATUSEFFECTDATA_H

#include <vector>

enum class StatusEffectType {
    DamageReduction,
    ThreatModifier,
    Stun,
    Poison,
    Regeneration,
    Bleed,
    Confusion,
    Fear,
    Burning,
};

struct StatusEffect {
    StatusEffectType type;
    int roundsLeft; // How many rounds the effect lasts, -1 for infinite
    float value;
};

#endif //SANDBOX_STATUSEFFECTDATA_H
