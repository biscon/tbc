//
// Created by bison on 24-01-25.
//

#ifndef SANDBOX_COMBATANIMATION_H
#define SANDBOX_COMBATANIMATION_H

#include "character/Character.h"
#include "Level.h"

void PlayAttackAnimation(Level &level, Character &attacker, Character &defender);
void PlayDefendAnimation(Level &level, Character &attacker, Character &defender);
void PlayAttackDefendAnimation(Level& level, Character& attacker, Character& defender);
void PlayEnemyVictoryAnimation(Level& level);
void PlayPlayerVictoryAnimation(Level& level);
void RemoveAttackAnimations(Level &level);
int GetBloodIntensity(int dmg, int attackerAttack);

#endif //SANDBOX_COMBATANIMATION_H
