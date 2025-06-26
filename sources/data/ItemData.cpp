//
// Created by bison on 25-06-25.
//

#include "ItemData.h"
#include <stdexcept>

// used to access lookup tables from serialization functions where I cannot change the function signature
static ItemData* itemData = nullptr;
static WeaponData* weaponData = nullptr;

static int GetTypeTemplateIdFromString(const ItemType& type, const std::string& strId) {
    switch (type) {
        case ItemType::Weapon: return weaponData->templateIdToIndex.at(strId);
        case ItemType::Consumable: return -1;
        case ItemType::Grenade:    return -1;
        case ItemType::KeyItem:    return -1;
        case ItemType::Armor:      return -1;
        default: throw std::runtime_error("Invalid ItemType");
    }
}

// -- Enum ↔ String Helpers --
const char* ItemTypeToString(ItemType type) {
    switch (type) {
        case ItemType::Weapon:     return "Weapon";
        case ItemType::Consumable: return "Consumable";
        case ItemType::Grenade:    return "Grenade";
        case ItemType::KeyItem:    return "KeyItem";
        case ItemType::Armor:      return "Armor";
        default:                   return "Unknown";
    }
}

ItemType ItemTypeFromString(const std::string& str) {
    if (str == "Weapon")      return ItemType::Weapon;
    if (str == "Consumable")  return ItemType::Consumable;
    if (str == "Grenade")     return ItemType::Grenade;
    if (str == "KeyItem")     return ItemType::KeyItem;
    if (str == "Armor")       return ItemType::Armor;
    throw std::runtime_error("Invalid ItemType: " + str);
}

// -- JSON Conversion --

void to_json(nlohmann::json& j, const ItemType& type) {
    j = ItemTypeToString(type);
}

void from_json(const nlohmann::json& j, ItemType& type) {
    type = ItemTypeFromString(j.get<std::string>());
}

void to_json(nlohmann::json& j, const ItemTemplate& item) {
    std::string strId = itemData->indexToTemplateId[item.id];
    std::string strTypeTemplateId = "Unknown";
    j = nlohmann::json{
            {"id", strId},
            {"name", item.name},
            {"type", item.type},
            {"typeTemplateId", strTypeTemplateId},
            {"value", item.value},
            {"stackable", item.stackable}
    };
}

void from_json(const nlohmann::json& j, ItemTemplate& item) {
    std::string strId;
    j.at("id").get_to(strId);

    item.id = static_cast<int>(itemData->templateData.size()); // Assign next ID
    itemData->templateIdToIndex[strId] = item.id;
    itemData->indexToTemplateId.push_back(strId);

    j.at("name").get_to(item.name);
    j.at("type").get_to(item.type);

    std::string typeTemplateId;
    j.at("typeTemplateId").get_to(typeTemplateId);
    // Do the same registration logic here for typeTemplateId if needed
    item.typeTemplateId = GetTypeTemplateIdFromString(item.type, typeTemplateId);

    j.at("value").get_to(item.value);
    j.at("stackable").get_to(item.stackable);
}

void to_json(nlohmann::json& j, const ItemInstance& inst) {
    const std::string& templateStrId = itemData->indexToTemplateId[inst.templateId];
    j = nlohmann::json{
            {"id", inst.id},
            {"templateId", templateStrId},
            {"quantity", inst.quantity}
    };
}

void from_json(const nlohmann::json& j, ItemInstance& inst) {
    j.at("id").get_to(inst.id);

    std::string templateStrId;
    j.at("templateId").get_to(templateStrId);

    auto it = itemData->templateIdToIndex.find(templateStrId);
    if (it == itemData->templateIdToIndex.end())
        throw std::runtime_error("Unknown ItemTemplate ID: " + templateStrId);

    inst.templateId = it->second;

    j.at("quantity").get_to(inst.quantity);
}

void SetDataInstancePtrs(ItemData* itemDataPtr, WeaponData* weaponDataPtr) {
    itemData = itemDataPtr;
    weaponData = weaponDataPtr;
}


