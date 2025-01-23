//
// Created by bison on 09-01-25.
//

#ifndef SANDBOX_COMBAT_H
#define SANDBOX_COMBAT_H

#include <map>
#include "Character.h"
#include "Animation.h"
#include "CombatState.h"

//int CalculateDamage(const Character &attacker, const Character &defender);
int Attack(CombatState &combat, Character &attacker, Character &defender);
int DealDamage(CombatState& combat, Character &attacker, Character &defender, int damage);
void InitCombat(CombatState &combat, std::vector<Character> &playerCharacters, std::vector<Character> &enemyCharacters);
bool IsPlayerCharacter(CombatState &combat, Character &character);
Character* GetFirstLivingEnemy(CombatState &combat);
bool IsIncapacitated(Character* character);
void DecayThreat(CombatState& combat, int amount);
void SetTaunt(CombatState& combat, Character* target);
void UpdateStatusEffects(CombatState &combat);
void UpdateSkillCooldown(CombatState &combat);
void NextCharacter(CombatState &combat);

#endif //SANDBOX_COMBAT_H
