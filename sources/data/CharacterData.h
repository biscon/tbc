//
// Created by bison on 13-03-25.
//

#ifndef SANDBOX_CHARACTERDATA_H
#define SANDBOX_CHARACTERDATA_H

#include <vector>
#include <string>
#include "SpriteData.h"
#include "SkillData.h"
#include "StatusEffectData.h"
#include "util/json.hpp"

enum class Orientation {
    Up,
    Down,
    Left,
    Right
};

enum class CharacterClass {
    Warrior,
    Mage,
    Rogue,
};
void to_json(nlohmann::json& j, const CharacterClass& c);
void from_json(const nlohmann::json& j, CharacterClass& c);


enum class CharacterFaction {
    Player,
    Npc,
    Enemy,
};
void to_json(nlohmann::json& j, const CharacterFaction& f);
void from_json(const nlohmann::json& j, CharacterFaction& f);

struct CharacterStats {
    int health;
    int maxHealth;
    int attack;
    int defense;
    int speed;
    int hunger;
    int thirst;
    int movePoints;
    int level;
};
void to_json(nlohmann::json& j, const CharacterStats& m);
void from_json(const nlohmann::json& j, CharacterStats& m);

enum class ItemEquipSlot {
    Weapon1,
    Armor,
    // Add more later
    COUNT
};


struct CharacterData {
    std::vector<CharacterClass> characterClass;
    std::vector<CharacterFaction> faction;
    std::vector<std::string> name;
    std::vector<std::string> ai;
    std::vector<CharacterStats> stats;
    std::vector<CharacterSprite> sprite;
    std::vector<std::vector<Skill>> skills;  // List of skills the character possesses
    std::vector<std::vector<StatusEffect>> statusEffects;
    std::vector<Orientation> orientation;
    std::vector<std::array<int, static_cast<size_t>(ItemEquipSlot::COUNT)>> equippedItemIdx;
};

inline const char* GetEquipSlotName(ItemEquipSlot slot) {
    switch (slot) {
        case ItemEquipSlot::Weapon1: return "weapon";
        case ItemEquipSlot::Armor:  return "armor";
        default: return "unknown";
    }
}

inline int GetEquipSlotIndexByName(const std::string& slotName) {
    if(slotName == "weapon") return 0;
    if(slotName == "armor") return 1;
    TraceLog(LOG_ERROR, "Could not map slot '%s' to index", slotName.c_str());
    std::abort();
    return -1;
}

#endif //SANDBOX_CHARACTERDATA_H
