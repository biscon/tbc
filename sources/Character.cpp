//
// Created by bison on 09-01-25.
//

#include "Character.h"
#include "Random.h"

// Simple function to display character info
void DisplayCharacterInfo(const Character &character) {
    std::cout << "Name: " << character.name << "\n"
              << "Health: " << character.health << "/" << character.maxHealth << "\n"
              << "Attack: " << character.attack << "\n"
              << "Defense: " << character.defense << "\n"
              << "Speed: " << character.speed << "\n"
              << "Hunger: " << character.hunger << "\n"
              << "Thirst: " << character.thirst << "\n";

    // Display skills
    if (!character.skills.empty()) {
        std::cout << "Skills:\n";
        for (const Skill &skill : character.skills) {
            std::cout << "  " << skill.name << " (Rank " << skill.rank << ")\n";
        }
    } else {
        std::cout << "No skills.\n";
    }

    std::cout << "\n";  // Adding a line break for better readability
}



// Function to generate a random character
Character GenerateRandomCharacter(std::string name, bool isEnemy) {
    Character c;
    c.name = std::move(name);
    c.maxHealth = RandomInRange(50, 100);
    c.health = c.maxHealth;  // New characters start at full health
    c.attack = RandomInRange(isEnemy ? 15 : 5, isEnemy ? 25 : 10);  // Enemies are stronger
    c.defense = RandomInRange(0, 10);
    c.speed = RandomInRange(1, 5);
    c.hunger = RandomInRange(0, 100);
    c.thirst = RandomInRange(0, 100);
    return c;
}

// Helper function to get the rank of a skill, returns -1 if the skill is not found
int GetSkillRank(const Character &character, SkillType type) {
    for (const Skill &skill : character.skills) {
        if (skill.type == type) {
            return skill.rank;
        }
    }
    return -1;  // Return -1 if skill not found
}

Skill* GetSkill(Character &character, SkillType type) {
    for (Skill &skill : character.skills) {
        if (skill.type == type) {
            return &skill;
        }
    }
    return nullptr;  // Return -1 if skill not found
}

std::vector<Skill*> GetActiveSkills(Character &character) {
    std::vector<Skill*> activeSkills;
    for (Skill &skill : character.skills) {
        if (!skill.isPassive) {
            activeSkills.push_back(&skill);
        }
    }
    return activeSkills;
}

bool IsAlive(const Character &character) {
    return character.health > 0;
}

StatusEffect* GetStatusEffectByType(Character &character, StatusEffectType type) {
    for (StatusEffect &effect : character.statusEffects) {
        if (effect.type == type) {
            return &effect;
        }
    }
    return nullptr;
}

bool CheckStatusEffectByType(Character &character, StatusEffectType type) {
    for (StatusEffect &effect : character.statusEffects) {
        if (effect.type == type) {
            return true;
        }
    }
    return false;
}


std::vector<StatusEffect*> GetStatusEffectsByType(Character &character, StatusEffectType type) {
    std::vector<StatusEffect*> effects;
    for (StatusEffect &effect : character.statusEffects) {
        if (effect.type == type) {
            effects.push_back(&effect);
        }
    }
    return effects;
}

std::string GetStatusEffectName(StatusEffectType type) {
    switch(type) {
        case StatusEffectType::DamageReduction: return "Damage-";
        case StatusEffectType::ThreatModifier: return "Threat";
        case StatusEffectType::Stun: return "Stunned";
        case StatusEffectType::Poison: return "Poison";
        case StatusEffectType::Regeneration: return "Regeneration";
        case StatusEffectType::Bleed: return "Bleed";
        case StatusEffectType::Confusion: return "Confusion";
        case StatusEffectType::Fear: return "Fear";
        default: return "Unknown";
    }
}

void CreateCharacter(Character &character, std::string name, int maxHealth, int attack, int defense, int speed) {
    character.name = std::move(name);
    character.maxHealth = maxHealth;
    character.health = maxHealth;
    character.attack = attack;
    character.defense = defense;
    character.speed = speed;
    character.hunger = 0;
    character.thirst = 0;
    character.movePoints = speed;
}

