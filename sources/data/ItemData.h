//
// Created by bison on 25-06-25.
//

#ifndef SANDBOX_ITEMDATA_H
#define SANDBOX_ITEMDATA_H

#include <string>
#include <unordered_map>
#include <vector>
#include "util/json.hpp"
#include "WeaponData.h"

enum class ItemType {
    Weapon,
    Consumable,
    Grenade,
    KeyItem,
    Armor,
};

struct ItemTemplate {
    int id;
    std::string name;
    ItemType type;
    int typeTemplateId;      // Index into type-specific table
    int value;          // Gold or barter value
    bool stackable;
};

struct ItemInstance {
    int id; // Unique ID
    int templateId; // Index into the master item templates table
    int quantity;   // For stackables
};

struct ItemData {
    std::vector<ItemTemplate> templateData;
    std::unordered_map<std::string, int> templateIdToIndex;
    std::vector<std::string> indexToTemplateId;
    std::vector<ItemInstance> instanceData;
};

// Enum conversion helpers
const char* ItemTypeToString(ItemType type);
ItemType ItemTypeFromString(const std::string& str);

// JSON (de)serialization
void to_json(nlohmann::json& j, const ItemTemplate& item);
void from_json(const nlohmann::json& j, ItemTemplate& item);

void to_json(nlohmann::json& j, const ItemInstance& inst);
void from_json(const nlohmann::json& j, ItemInstance& inst);

void to_json(nlohmann::json& j, const ItemType& type);
void from_json(const nlohmann::json& j, ItemType& type);

void SetDataInstancePtrs(ItemData* itemDataPtr, WeaponData* weaponDataPtr);

#endif //SANDBOX_ITEMDATA_H
