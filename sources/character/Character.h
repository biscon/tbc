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
#include "data/GameData.h"


int CreateCharacter(CharacterData &data, CharacterFaction faction, const std::string& name, const std::string& ai);
void ClearAllCharacters(CharacterData& data);
void DeleteCharacter(CharacterData& data, int id);
bool IsAlive(CharacterData &data, int characterIdx);
void GiveWeapon(GameData& data, int characterIdx, const std::string& itemTemplate, ItemEquipSlot slot);
Vector2 GetOrientationVector(Orientation orientation);
void FaceCharacter(SpriteData& spriteData, CharacterData &charData, int attackerId, int defenderId);
CharacterFaction StringToFaction(const std::string& factionName);
int GetEquippedItem(const GameData& data, int charIdx, ItemEquipSlot slot);
void SetEquippedItem(GameData& data, int charIdx, ItemEquipSlot slot, int itemIdx);
SpriteAnimationType CharacterOrientationToAnimType(GameData& data, int charId);

int CalculateCharHealth(CharacterStats& stats);
int CalculateCharInitiative(CharacterStats &stats);
int CalculateCharMaxAP(CharacterStats &stats);

#endif //SANDBOX_CHARACTER_H
