//
// Created by bison on 09-01-25.
//

#ifndef SANDBOX_COMBAT_H
#define SANDBOX_COMBAT_H

#include <map>
#include "character/Character.h"
#include "graphics/Animation.h"
#include "CombatState.h"

AttackResult Attack(CombatState &combat, Character &attacker, Character &defender);
int DealDamage(CombatState& combat, Character &attacker, Character &defender, int damage);
int DealDamageStatusEffect(CombatState& combat, Character &target, int damage);
void KillCharacter(CombatState &combat, Character &character);
void InitCombat(CombatState &combat, std::vector<Character> &playerCharacters, std::vector<Character> &enemyCharacters);
bool IsPlayerCharacter(CombatState &combat, Character &character);
Character* GetFirstLivingEnemy(CombatState &combat);
bool IsIncapacitated(Character* character);
void DecayThreat(CombatState& combat, int amount);
void SetTaunt(CombatState& combat, Character* target);
void NextCharacter(CombatState &combat);

#endif //SANDBOX_COMBAT_H
