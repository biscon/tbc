//
// Created by bison on 09-01-25.
//

#include "Character.h"
#include "util/Random.h"

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

bool IsAlive(const Character &character) {
    return character.health > 0;
}

void CreateCharacter(Character &character, CharacterClass characterClass, std::string name, std::string ai) {
    character.name = std::move(name);
    character.ai = std::move(ai);
    character.maxHealth = 20;
    character.health = 20;
    character.attack = 5;
    character.defense = 3;
    character.speed = 4;
    character.hunger = 0;
    character.thirst = 0;
    character.movePoints = 0;
    character.level = 1;
    character.characterClass = characterClass;
    character.orientation = Orientation::Right;
}

Vector2 GetOrientationVector(Orientation orientation) {
    switch (orientation) {
        case Orientation::Up:
            return {0, -1};
        case Orientation::Down:
            return {0, 1};
        case Orientation::Left:
            return {-1, 0};
        case Orientation::Right:
            return {1, 0};
    }
    return {0, 0};
}

// This could be in your Character struct or class
void LevelUp(Character &character, bool autoDistributePoints) {
    character.level++;
    int healthIncrease = 0;
    int attackIncrease = 0;
    int defenseIncrease = 0;
    int speedIncrease = 0;

    // Automatic stat increases per level
    switch(character.characterClass) {
        case CharacterClass::Warrior:
            healthIncrease = 4;
            attackIncrease = 1;
            defenseIncrease = 1;
            break;
        case CharacterClass::Mage:
            healthIncrease = 2;
            defenseIncrease = 1;
            speedIncrease = 1;
            break;
        case CharacterClass::Rogue:
            healthIncrease = 2;
            defenseIncrease = 1;
            speedIncrease = 1;
            break;
    }

    // Apply automatic increases
    character.maxHealth += healthIncrease;
    character.attack += attackIncrease;
    character.defense += defenseIncrease;
    character.speed += speedIncrease;

    if (character.level % 2 == 0 && autoDistributePoints) {
        int pointsToDistribute = 5;
        int pointsPerStat = pointsToDistribute / 3;  // Divide points evenly
        int remainder = pointsToDistribute % 3;      // The leftover points

        // Distribute the evenly divided points
        switch(character.characterClass) {
            case CharacterClass::Warrior: {
                character.maxHealth += pointsPerStat;
                character.attack += pointsPerStat;
                character.defense += pointsPerStat;

                // Now distribute the remaining points
                if (remainder > 0) {
                    character.attack++;  // Next point goes to attack
                    remainder--;
                }
                if (remainder > 0) {
                    character.speed++;  // Extra point goes to health (for example)
                    remainder--;
                }
                if (remainder > 0) {
                    character.defense++;  // Remaining point goes to defense
                    remainder--;
                }
                break;
            }
            case CharacterClass::Mage: {
                character.maxHealth += pointsPerStat;
                character.attack += pointsPerStat;
                character.defense += pointsPerStat;

                // Distribute the remaining points
                if (remainder > 0) {
                    character.attack++;  // Extra point goes to attack
                    remainder--;
                }
                if (remainder > 0) {
                    character.maxHealth++;  // Next point goes to health
                    remainder--;
                }
                if (remainder > 0) {
                    character.defense++;  // Last point goes to defense
                    remainder--;
                }
                break;
            }
            case CharacterClass::Rogue: {
                character.maxHealth += pointsPerStat;
                character.attack += pointsPerStat;
                character.speed += pointsPerStat;

                // Distribute the remaining points
                if (remainder > 0) {
                    character.attack++;  // Extra point goes to attack (for example)
                    remainder--;
                }
                if (remainder > 0) {
                    character.defense++;  // Next point goes to defense
                    remainder--;
                }
                if (remainder > 0) {
                    character.maxHealth++;  // Last point goes to health
                    remainder--;
                }
                break;
            }
        }
    }

    // Recalculate max health if you have a max health stat
    // Optionally, you could add max health directly as a function of level or health
    character.health = character.maxHealth;
}
