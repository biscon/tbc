//
// Created by bison on 09-01-25.
//

#ifndef SANDBOX_COMBAT_H
#define SANDBOX_COMBAT_H

#include <map>
#include "character/Character.h"
#include "graphics/Animation.h"
#include "Level.h"

AttackResult Attack(Level &level, Character &attacker, Character &defender);
int DealDamage(Level& level, Character &attacker, Character &defender, int damage);
int DealDamageStatusEffect(Level& level, Character &target, int damage);
void KillCharacter(Level &level, Character &character);
bool IsPlayerCharacter(Character &character);
bool IsIncapacitated(Character* character);
void DecayThreat(Level& level, int amount);
void SetTaunt(Level& level, Character* target);
void NextCharacter(Level &level);

#endif //SANDBOX_COMBAT_H
