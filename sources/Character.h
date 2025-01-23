//
// Created by bison on 09-01-25.
//

#ifndef SANDBOX_CHARACTER_H
#define SANDBOX_CHARACTER_H

#include <iostream>
#include <utility>
#include <vector>
#include "Sprite.h"

enum class SkillType {
    Dodge,
    Stun,
    Taunt,
    FlameJet,
    Bleed
};

// Skill struct to represent individual skills like Dodge
struct Skill {
    SkillType type;
    char name[64];
    int rank;  // Rank 1, Rank 2, Rank 3
    bool isPassive;  // Passive skills are always active
    bool noTarget;  // Skills that don't require a target
    int cooldown; // How many rounds before the skill can be used again
    int maxCooldown; // The maximum cooldown for the skill
    int range;  // How far the skill can reach
};

enum class StatusEffectType {
    DamageReduction,
    ThreatModifier,
    Stun,
    Poison,
    Regeneration,
    Bleed,
    Confusion,
    Fear,
    Burning,
};

struct StatusEffect {
    StatusEffectType type;
    int roundsLeft; // How many rounds the effect lasts, -1 for infinite
    float value;
};

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
Skill* GetSkill(Character &character, SkillType type);
int GetSkillRank(const Character &character, SkillType type);
std::vector<Skill*> GetActiveSkills(Character &character);
bool IsAlive(const Character &character);
bool CheckStatusEffectByType(Character &character, StatusEffectType type);
StatusEffect* GetStatusEffectByType(Character &character, StatusEffectType type);
std::vector<StatusEffect*> GetStatusEffectsByType(Character &character, StatusEffectType type);
std::string GetStatusEffectName(StatusEffectType type);
void DecreaseSkillCooldown(Character &character);

#endif //SANDBOX_CHARACTER_H
