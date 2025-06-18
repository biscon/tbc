//
// Created by bison on 09-01-25.
//

#ifndef SANDBOX_CHARACTER_H
#define SANDBOX_CHARACTER_H

#include <iostream>
#include <utility>
#include <vector>
#include "graphics/CharacterSprite.h"
#include "Skill.h"
#include "StatusEffect.h"
#include "Weapon.h"
#include "data/CharacterData.h"


int CreateCharacter(CharacterData &data, CharacterClass characterClass, CharacterFaction faction, const std::string& name, const std::string& ai);
void ClearAllCharacters(CharacterData& data);
void DeleteCharacter(CharacterData& data, int id);
bool IsAlive(CharacterData &data, int characterIdx);
void GiveWeapon(SpriteData& spriteData, WeaponData& weaponData, CharacterData &data, int characterIdx, const std::string& weaponTemplate);
Vector2 GetOrientationVector(Orientation orientation);
void LevelUp(CharacterData &charData, int cid, bool autoDistributePoints);
int GetAttack(CharacterData &charData, WeaponData& weaponData, int cid);
void FaceCharacter(SpriteData& spriteData, CharacterData &charData, int attackerId, int defenderId);
CharacterClass StringToClass(const std::string& className);
CharacterFaction StringToFaction(const std::string& factionName);

#endif //SANDBOX_CHARACTER_H
