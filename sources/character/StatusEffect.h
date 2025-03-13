//
// Created by bison on 23-01-25.
//

#ifndef SANDBOX_STATUSEFFECT_H
#define SANDBOX_STATUSEFFECT_H

#include <string>
#include "data/StatusEffectData.h"

bool CheckStatusEffectByType(std::vector<StatusEffect>& statusEffects, StatusEffectType type);
StatusEffect* GetStatusEffectByType(std::vector<StatusEffect>& statusEffects, StatusEffectType type);
std::vector<StatusEffect*> GetStatusEffectsByType(std::vector<StatusEffect>& statusEffects, StatusEffectType type);
std::string GetStatusEffectName(StatusEffectType type);
void AssignStatusEffect(std::vector<StatusEffect>& statusEffects, StatusEffectType type, int roundsLeft, float value);
void AssignStatusEffectAllowStacking(std::vector<StatusEffect>& statusEffects, StatusEffectType type, int roundsLeft, float value);

#endif //SANDBOX_STATUSEFFECT_H
