//
// Created by bison on 09-01-25.
//

#ifndef SANDBOX_COMBAT_H
#define SANDBOX_COMBAT_H

#include <map>
#include "Level.h"

AttackResult Attack(CharacterData& charData, WeaponData& weaponData, Level& level, int attacker, int defender);
int DealDamage(SpriteData& spriteData, CharacterData& charData, WeaponData& weaponData, Level& level, int attacker, int defender, int damage);
int DealDamageStatusEffect(SpriteData& spriteData, CharacterData& charData, WeaponData& weaponData, Level& level, int target, int damage);
void KillCharacter(SpriteData& spriteData, CharacterData& charData, Level &level, int character);
bool IsPlayerCharacter(CharacterData& charData, int character);
bool IsIncapacitated(CharacterData& charData, int character);
void DecayThreat(CharacterData& charData, Level& level, int amount);
void SetTaunt(CharacterData& charData, Level& level, int target);
void NextCharacter(CharacterData& charData, Level &level);
void StartCombat(SpriteData& spriteData, CharacterData& charData, Level &level, int spotter, int maxDist);

#endif //SANDBOX_COMBAT_H
