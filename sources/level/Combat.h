//
// Created by bison on 09-01-25.
//

#ifndef SANDBOX_COMBAT_H
#define SANDBOX_COMBAT_H

#include <map>
#include "character/Character.h"
#include "graphics/Animation.h"
#include "Level.h"

AttackResult Attack(CharacterData& charData, WeaponData& weaponData, Level& level, int attacker, int defender);
int DealDamage(CharacterData& charData, WeaponData& weaponData, Level& level, int attacker, int defender, int damage);
int DealDamageStatusEffect(CharacterData& charData, WeaponData& weaponData, Level& level, int target, int damage);
void KillCharacter(CharacterData& charData, Level &level, int character);
bool IsPlayerCharacter(CharacterData& charData, int character);
bool IsIncapacitated(CharacterData& charData, int character);
void DecayThreat(CharacterData& charData, Level& level, int amount);
void SetTaunt(CharacterData& charData, Level& level, int target);
void NextCharacter(CharacterData& charData, Level &level);
void StartCombat(CharacterData& charData, Level &level, int spotter, int maxDist);

#endif //SANDBOX_COMBAT_H
