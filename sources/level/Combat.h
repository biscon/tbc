//
// Created by bison on 09-01-25.
//

#ifndef SANDBOX_COMBAT_H
#define SANDBOX_COMBAT_H

#include <map>
#include "Level.h"

AttackResult Attack(GameData& data, Level& level, int attacker, int defender);
int DealDamage(GameData& data, Level& level, int attacker, int defender, int damage);
int DealDamageStatusEffect(GameData& data, Level& level, int target, int damage);
void KillCharacter(SpriteData& spriteData, CharacterData& charData, Level &level, int character);
bool IsPlayerCharacter(CharacterData& charData, int character);
bool IsIncapacitated(CharacterData& charData, int character);
void NextCharacter(CharacterData& charData, Level &level);
void StartCombat(SpriteData& spriteData, CharacterData& charData, Level &level, int spotter);

#endif //SANDBOX_COMBAT_H
