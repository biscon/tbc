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
#include "QuestData.h"

struct DoorSaveState {
    std::string id;
    bool open;
    bool locked;
};

void to_json(nlohmann::json& j, const DoorSaveState& m);
void from_json(const nlohmann::json& j, DoorSaveState& m);

struct LevelSaveState {
    std::unordered_set<std::string> defeatedGroups;
    std::unordered_map<std::string, DoorSaveState> doors;
    std::unordered_map<std::string, bool> flags;
};

void to_json(nlohmann::json& j, const LevelSaveState& m);
void from_json(const nlohmann::json& j, LevelSaveState& m);

struct PartyCharacter {
    std::string name;
    std::string ai;
    std::string spriteTemplate;
    int tilePosX = 0;
    int tilePosY = 0;
    CharacterFaction faction;
    CharacterStats stats;
    std::array<std::string, static_cast<size_t>(ItemEquipSlot::COUNT)> equippedItems;
    int selectedWeaponSlot;
};

struct InventorySaveState {
    int capacity;
    std::vector<std::string> itemTemplateIds;
};

void to_json(nlohmann::json& j, const InventorySaveState& s);
void from_json(const nlohmann::json& j, InventorySaveState& s);

void to_json(nlohmann::json& j, const PartyCharacter& c);
void from_json(const nlohmann::json& j, PartyCharacter& c);

struct SaveData {
    std::string currentLevel;
    std::unordered_map<std::string, LevelSaveState> levels;
    std::list<PartyCharacter> party;
    std::unordered_map<std::string, QuestSaveState> quests;
    InventorySaveState partyInventory;
};

bool SaveGameData(SaveData& data, const std::string& filename);
bool LoadGameData(SaveData& data, const std::string& filename);

#endif //SANDBOX_SAVEDATA_H
