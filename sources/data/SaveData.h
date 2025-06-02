//
// Created by bison on 28-05-25.
//

#ifndef SANDBOX_SAVEDATA_H
#define SANDBOX_SAVEDATA_H

#include <string>
#include <vector>
#include <unordered_map>
#include "util/json.hpp"
#include "CharacterData.h"

struct MapState {
    std::vector<int> deadMobIds;  // e.g. index into monster group or unique IDs
};

void to_json(nlohmann::json& j, const MapState& m);
void from_json(const nlohmann::json& j, MapState& m);

struct PartyCharacter {
    std::string name;
    std::string ai;
    CharacterClass characterClass;
    CharacterFaction faction;
    CharacterStats stats;
};

void to_json(nlohmann::json& j, const PartyCharacter& c);
void from_json(const nlohmann::json& j, PartyCharacter& c);

struct SaveData {
    std::string currentMap;
    std::unordered_map<std::string, MapState> maps; // key = map name
};

bool SaveGameData(SaveData& data, const std::string& filename);
bool LoadGameData(SaveData& data, const std::string& filename);

#endif //SANDBOX_SAVEDATA_H
