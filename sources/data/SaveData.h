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

struct ItemInstanceSaveState {
    std::string templateId; // Index into the master item templates table
    int instanceDataIdx;
};

struct InventorySaveState {
    int capacity;
    std::vector<ItemInstanceSaveState> instances;
};

struct LevelSaveState {
    std::unordered_set<std::string> defeatedGroups;
    std::unordered_map<std::string, DoorSaveState> doors;
    std::unordered_map<std::string, InventorySaveState> objectInventories;
};

void to_json(nlohmann::json& j, const DoorSaveState& m);
void from_json(const nlohmann::json& j, DoorSaveState& m);
void to_json(nlohmann::json& j, const LevelSaveState& m);
void from_json(const nlohmann::json& j, LevelSaveState& m);
void to_json(nlohmann::json& j, const ItemInstanceSaveState& m);
void from_json(const nlohmann::json& j, ItemInstanceSaveState& m);

struct PartyCharacter {
    std::string name;
    std::string ai;
    std::string spriteTemplate;
    int tilePosX = 0;
    int tilePosY = 0;
    CharacterFaction faction;
    CharacterStats stats;
    std::array<ItemInstanceSaveState, static_cast<size_t>(ItemEquipSlot::COUNT)> equippedItems;
    std::array<int, static_cast<size_t>(Skill::Count)> skillValues;
    int selectedWeaponSlot;
};


void to_json(nlohmann::json& j, const InventorySaveState& s);
void from_json(const nlohmann::json& j, InventorySaveState& s);

void to_json(nlohmann::json& j, const PartyCharacter& c);
void from_json(const nlohmann::json& j, PartyCharacter& c);

struct WeaponInstanceSaveState {
    std::string templateId;
    int id;
    int currentAmmo;
    bool jammed;
};

void to_json(nlohmann::json& j, const WeaponInstanceSaveState& s);
void from_json(const nlohmann::json& j, WeaponInstanceSaveState& s);

struct SaveData {
    std::string currentLevel;
    std::unordered_map<std::string, LevelSaveState> levels;
    std::list<PartyCharacter> party;
    std::unordered_map<std::string, QuestSaveState> quests;
    InventorySaveState partyInventory;

    // type specific item instance data
    std::vector<WeaponInstanceSaveState> weaponInstances;
    std::unordered_map<std::string, bool> flags;
};

bool SaveGameData(SaveData& data, const std::string& filename);
bool LoadGameData(SaveData& data, const std::string& filename);

#endif //SANDBOX_SAVEDATA_H
