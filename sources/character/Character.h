//
// Created by bison on 09-01-25.
//

#ifndef SANDBOX_CHARACTER_H
#define SANDBOX_CHARACTER_H

#include <iostream>
#include <utility>
#include <vector>
#include "graphics/Sprite.h"
#include "Skill.h"
#include "StatusEffect.h"

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
};

void CreateCharacter(Character &character, std::string name, int maxHealth, int attack, int defense, int speed);
void DisplayCharacterInfo(const Character &character);
Character GenerateRandomCharacter(std::string name, bool isEnemy = false);
bool IsAlive(const Character &character);
Vector2 GetOrientationVector(Orientation orientation);
void LevelUp(Character &character, bool autoDistributePoints);

#endif //SANDBOX_CHARACTER_H
