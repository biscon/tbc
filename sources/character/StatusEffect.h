//
// Created by bison on 23-01-25.
//

#ifndef SANDBOX_STATUSEFFECT_H
#define SANDBOX_STATUSEFFECT_H

#include <vector>
#include <string>

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

bool CheckStatusEffectByType(std::vector<StatusEffect>& statusEffects, StatusEffectType type);
StatusEffect* GetStatusEffectByType(std::vector<StatusEffect>& statusEffects, StatusEffectType type);
std::vector<StatusEffect*> GetStatusEffectsByType(std::vector<StatusEffect>& statusEffects, StatusEffectType type);
std::string GetStatusEffectName(StatusEffectType type);
void AssignStatusEffect(std::vector<StatusEffect>& statusEffects, StatusEffectType type, int roundsLeft, float value);
void AssignStatusEffectAllowStacking(std::vector<StatusEffect>& statusEffects, StatusEffectType type, int roundsLeft, float value);

#endif //SANDBOX_STATUSEFFECT_H
