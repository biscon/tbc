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

struct Character {
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
    CharacterSprite sprite;
    std::vector<Skill> skills;  // List of skills the character possesses
    std::vector<StatusEffect> statusEffects;
};

void CreateCharacter(Character &character, std::string name, int maxHealth, int attack, int defense, int speed);
void DisplayCharacterInfo(const Character &character);
Character GenerateRandomCharacter(std::string name, bool isEnemy = false);
bool IsAlive(const Character &character);

#endif //SANDBOX_CHARACTER_H
