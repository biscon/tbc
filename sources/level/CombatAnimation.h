//
// Created by bison on 24-01-25.
//

#ifndef SANDBOX_COMBATANIMATION_H
#define SANDBOX_COMBATANIMATION_H

#include "character/Character.h"
#include "LevelState.h"

void PlayAttackAnimation(LevelState &combat, Character &attacker, Character &defender);
void PlayDefendAnimation(LevelState &combat, Character &attacker, Character &defender);
void PlayAttackDefendAnimation(LevelState& combat, Character& attacker, Character& defender);
void PlayEnemyVictoryAnimation(LevelState& combat);
void PlayPlayerVictoryAnimation(LevelState& combat);
void RemoveAttackAnimations(LevelState &combat);
int GetBloodIntensity(int dmg, int attackerAttack);

#endif //SANDBOX_COMBATANIMATION_H
