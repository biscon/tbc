//
// Created by bison on 09-01-25.
//

#ifndef SANDBOX_COMBAT_H
#define SANDBOX_COMBAT_H

#include <map>
#include "Level.h"

#define FULL_AUTO_SHOTS 8

AttackResult Attack(GameData& data, Level& level, int attacker, int defender, int fireMode);
int DealDamage(GameData& data, Level& level, int attacker, int defender, int damage);
int DealDamageStatusEffect(GameData& data, Level& level, int target, int damage);
void KillCharacter(SpriteData& spriteData, CharacterData& charData, Level &level, int character);
bool IsPlayerCharacter(CharacterData& charData, int character);
bool IsIncapacitated(CharacterData& charData, int character);
void NextCharacter(CharacterData& charData, Level &level);
void StartCombat(SpriteData& spriteData, CharacterData& charData, Level &level, int spotter);
void CalcHitChance(GameData& data, int charId, int weaponItemId, int fireModeIdx, AttackInfo& info);
int GetAttackAPCost(GameData& data, int attacker, int fireMode);

#endif //SANDBOX_COMBAT_H
