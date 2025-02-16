//
// Created by bison on 09-01-25.
//

#ifndef SANDBOX_COMBAT_H
#define SANDBOX_COMBAT_H

#include <map>
#include "character/Character.h"
#include "graphics/Animation.h"
#include "LevelState.h"

AttackResult Attack(LevelState &combat, Character &attacker, Character &defender);
int DealDamage(LevelState& combat, Character &attacker, Character &defender, int damage);
int DealDamageStatusEffect(LevelState& combat, Character &target, int damage);
void KillCharacter(LevelState &combat, Character &character);
void InitCombat(LevelState &combat, std::vector<Character> &playerCharacters, std::vector<Character> &enemyCharacters);
bool IsPlayerCharacter(LevelState &combat, Character &character);
Character* GetFirstLivingEnemy(LevelState &combat);
bool IsIncapacitated(Character* character);
void DecayThreat(LevelState& combat, int amount);
void SetTaunt(LevelState& combat, Character* target);
void NextCharacter(LevelState &combat);

#endif //SANDBOX_COMBAT_H
