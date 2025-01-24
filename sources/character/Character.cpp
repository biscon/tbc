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
