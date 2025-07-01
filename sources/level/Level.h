//
// Created by bison on 20-01-25.
//

#ifndef SANDBOX_LEVEL_H
#define SANDBOX_LEVEL_H

#include <string>
#include <unordered_map>
#include "data/GameData.h"

void CreateLevel(Level &level);
void LoadLevel(GameData& data, Level &level, const std::string &filename);
void DestroyLevel(SpriteSheetData& sheetData, Level &level);
void WaitTurnState(Level &level, TurnState state, float waitTime);
void AddPartyToLevelNoPositioning(SpriteData& spriteData, CharacterData& charData, Level &level, std::vector<int> &party);
void AddPartyToLevel(SpriteData& spriteData, CharacterData& charData, Level &level, std::vector<int> &party, const std::string& spawnPoint);
void AddEnemiesToLevel(SpriteData& spriteData, CharacterData& charData, Level &level, std::vector<int> &enemies, const std::string& spawnPoint);
void UpdateVisibilityMap(GameData& data, Level& level);


#endif //SANDBOX_LEVEL_H
