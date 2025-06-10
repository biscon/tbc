//
// Created by bison on 28-05-25.
//

#ifndef SANDBOX_SAVEDATA_H
#define SANDBOX_SAVEDATA_H

#include <string>
#include <vector>
#include <unordered_map>
#include <list>
#include <unordered_set>
#include "util/json.hpp"
#include "CharacterData.h"

struct LevelSaveState {
    std::unordered_set<std::string> defeatedGroups;
};

void to_json(nlohmann::json& j, const LevelSaveState& m);
void from_json(const nlohmann::json& j, LevelSaveState& m);

struct PartyCharacter {
    std::string name;
    std::string ai;
    std::string weaponTemplate;
    std::string spriteTemplate;
    int tilePosX = 0;
    int tilePosY = 0;
    CharacterClass characterClass;
    CharacterFaction faction;
    CharacterStats stats;
};

void to_json(nlohmann::json& j, const PartyCharacter& c);
void from_json(const nlohmann::json& j, PartyCharacter& c);

struct SaveData {
    std::string currentLevel;
    std::unordered_map<std::string, LevelSaveState> levels; // key = map name
    std::list<PartyCharacter> party;
};

bool SaveGameData(SaveData& data, const std::string& filename);
bool LoadGameData(SaveData& data, const std::string& filename);

#endif //SANDBOX_SAVEDATA_H
