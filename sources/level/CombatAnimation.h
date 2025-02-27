//
// Created by bison on 24-01-25.
//

#ifndef SANDBOX_COMBATANIMATION_H
#define SANDBOX_COMBATANIMATION_H

#include "character/Character.h"
#include "Level.h"

void PlayAttackAnimation(CharacterData& charData, Level &level, int attacker, int defender);
void PlayDefendAnimation(CharacterData& charData, Level &level, int attacker, int defender);
void PlayAttackDefendAnimation(CharacterData& charData, Level &level, int attacker, int defender);
void PlayEnemyVictoryAnimation(CharacterData& charData, Level &level);
void PlayPlayerVictoryAnimation(CharacterData& charData, Level& level);
void RemoveAttackAnimations(Level &level);
int GetBloodIntensity(int dmg, int attackerAttack);

#endif //SANDBOX_COMBATANIMATION_H
