//
// Created by bison on 24-01-25.
//

#ifndef SANDBOX_COMBATANIMATION_H
#define SANDBOX_COMBATANIMATION_H

#include "character/Character.h"
#include "CombatState.h"

void PlayAttackAnimation(CombatState &combat, Character &attacker, Character &defender);
void PlayDefendAnimation(CombatState &combat, Character &attacker, Character &defender);
void PlayAttackDefendAnimation(CombatState& combat, Character& attacker, Character& defender);
void PlayEnemyVictoryAnimation(CombatState& combat);
void PlayPlayerVictoryAnimation(CombatState& combat);
void RemoveAttackAnimations(CombatState &combat);

#endif //SANDBOX_COMBATANIMATION_H
