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

/*
struct Character {
    CharacterClass characterClass;
    CharacterFaction faction;
    std::string name;
    std::string ai;
    int health;
    int maxHealth;
    int attack;
    int defense;
    int speed;  // How quickly they act in combat and how far they can move
    int hunger;
    int thirst;
    int movePoints;
    int level;
    CharacterSprite sprite;
    std::vector<Skill> skills;  // List of skills the character possesses
    std::vector<StatusEffect> statusEffects;
    Orientation orientation;
    int weaponIdx;
    bool isWeaponEquipped;
};
*/

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


int CreateCharacter(CharacterData &data, CharacterClass characterClass, CharacterFaction faction, const std::string& name, const std::string& ai);
bool IsAlive(CharacterData &data, int characterIdx);
void GiveWeapon(WeaponData& weaponData, CharacterData &data, int characterIdx, const std::string& weaponTemplate);
Vector2 GetOrientationVector(Orientation orientation);
void LevelUp(CharacterData &charData, int cid, bool autoDistributePoints);
int GetAttack(CharacterData &charData, WeaponData& weaponData, int cid);
void FaceCharacter(CharacterData &charData, int attackerId, int defenderId);

#endif //SANDBOX_CHARACTER_H
