//
// Created by bison on 23-01-25.
//

#include <algorithm>
#include "StatusEffect.h"

StatusEffect* GetStatusEffectByType(std::vector<StatusEffect>& statusEffects, StatusEffectType type) {
    for (StatusEffect &effect : statusEffects) {
        if (effect.type == type) {
            return &effect;
        }
    }
    return nullptr;
}

bool CheckStatusEffectByType(std::vector<StatusEffect>& statusEffects, StatusEffectType type) {
    for (StatusEffect &effect : statusEffects) {
        if (effect.type == type) {
            return true;
        }
    }
    return false;
}


std::vector<StatusEffect*> GetStatusEffectsByType(std::vector<StatusEffect>& statusEffects, StatusEffectType type) {
    std::vector<StatusEffect*> effects;
    for (StatusEffect &effect : statusEffects) {
        if (effect.type == type) {
            effects.push_back(&effect);
        }
    }
    return effects;
}

std::string GetStatusEffectName(StatusEffectType type) {
    switch(type) {
        case StatusEffectType::DamageReduction: return "Damage-";
        case StatusEffectType::ThreatModifier: return "Threat";
        case StatusEffectType::Stun: return "Stunned";
        case StatusEffectType::Poison: return "Poison";
        case StatusEffectType::Regeneration: return "Regeneration";
        case StatusEffectType::Bleed: return "Bleed";
        case StatusEffectType::Confusion: return "Confusion";
        case StatusEffectType::Fear: return "Fear";
        case StatusEffectType::Burning: return "Burning";
        default: return "Unknown";
    }
}

void AssignStatusEffectAllowStacking(std::vector<StatusEffect> &statusEffects, StatusEffectType type, int roundsLeft,
                                     float value) {
    StatusEffect effect{};
    effect.type = type;
    effect.roundsLeft = roundsLeft;
    effect.value = value;
    statusEffects.push_back(effect);
}

void AssignStatusEffect(std::vector<StatusEffect> &statusEffects, StatusEffectType type, int roundsLeft, float value) {
    // check if a similar effect is already applied, update it if so
    for (StatusEffect &effect : statusEffects) {
        if (effect.type == type) {
            effect.roundsLeft = roundsLeft;
            effect.value = value;
            return;
        }
    }
    AssignStatusEffectAllowStacking(statusEffects, type, roundsLeft, value);
}


