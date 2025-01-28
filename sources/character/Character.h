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

struct Character {
    CharacterClass characterClass;
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
    Weapon weapon;
    Weapon* equippedWeapon;
};

void CreateCharacter(Character &character, CharacterClass characterClass, std::string name, std::string ai);
void DisplayCharacterInfo(const Character &character);
Character GenerateRandomCharacter(std::string name, bool isEnemy = false);
bool IsAlive(const Character &character);
void GiveWeapon(Character &character, Weapon &weapon);
void GiveWeapon(Character &character, const std::string& weaponTemplate);
Vector2 GetOrientationVector(Orientation orientation);
void LevelUp(Character &character, bool autoDistributePoints);
int GetAttack(const Character &character);

#endif //SANDBOX_CHARACTER_H
