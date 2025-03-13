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

enum class CharacterFaction {
    Player,
    Npc,
    Enemy,
};

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
    std::vector<int> weaponIdx;
    std::vector<bool> isWeaponEquipped;
};


#endif //SANDBOX_CHARACTERDATA_H
