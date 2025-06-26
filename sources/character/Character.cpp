//
// Created by bison on 09-01-25.
//

#include <cmath>
#include "Character.h"
#include "util/Random.h"
#include "game/Items.h"

// Simple function to display character info
/*
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
*/


bool IsAlive(CharacterData &data, int characterIdx) {
    return data.stats[characterIdx].health > 0;
}

int CreateCharacter(CharacterData &data, CharacterClass characterClass, CharacterFaction faction, const std::string& name, const std::string& ai) {
    data.name.push_back(name);
    data.ai.push_back(ai);
    data.faction.emplace_back(faction);

    CharacterStats stats{};
    switch(characterClass) {
        case CharacterClass::Warrior: stats.maxHealth = 16; break;
        case CharacterClass::Mage: stats.maxHealth = 12; break;
        case CharacterClass::Rogue: stats.maxHealth = 14; break;
        default: stats.maxHealth = 16; break;
    }


    //character.maxHealth = 16;
    stats.health = stats.maxHealth;
    stats.attack = 5;
    stats.defense = 3;
    stats.speed = 4;
    stats.hunger = 0;
    stats.thirst = 0;
    stats.movePoints = 0;
    stats.level = 1;
    data.stats.emplace_back(stats);
    data.characterClass.emplace_back(characterClass);
    data.orientation.emplace_back(Orientation::Right);
    data.equippedItemIdx.emplace_back();
    data.equippedItemIdx.back().fill(-1);
    data.statusEffects.emplace_back();
    data.skills.emplace_back();
    data.sprite.emplace_back();
    return (int) data.name.size()-1;
}

void DeleteCharacter(CharacterData& data, int id) {
    data.stats.erase(data.stats.begin() + id);
    data.sprite.erase(data.sprite.begin() + id);
    data.orientation.erase(data.orientation.begin() + id);
    data.equippedItemIdx.erase(data.equippedItemIdx.begin() + id);
    data.characterClass.erase(data.characterClass.begin() + id);
    data.name.erase(data.name.begin() + id);
    data.ai.erase(data.ai.begin() + id);
    data.faction.erase(data.faction.begin() + id);
    data.statusEffects.erase(data.statusEffects.begin() + id);
    data.skills.erase(data.skills.begin() + id);
}

void ClearAllCharacters(CharacterData& data) {
    data.stats.clear();
    data.sprite.clear();
    data.orientation.clear();
    data.equippedItemIdx.clear();
    data.characterClass.clear();
    data.name.clear();
    data.ai.clear();
    data.faction.clear();
    data.statusEffects.clear();
    data.skills.clear();
}

void GiveWeapon(GameData& data, int characterIdx, const std::string& itemTemplate, ItemEquipSlot slot) {
    int weaponId = CreateItem(data, itemTemplate, 1);
    SetEquippedItem(data, characterIdx, slot, weaponId);
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
void LevelUp(CharacterData &charData, int cid, bool autoDistributePoints) {
    charData.stats[cid].level++;
    int healthIncrease = 0;
    int attackIncrease = 0;
    int defenseIncrease = 0;
    int speedIncrease = 0;

    // Automatic stat increases per level
    switch(charData.characterClass[cid]) {
        case CharacterClass::Warrior:
            healthIncrease = 6;
            attackIncrease = 1;
            defenseIncrease = 1;
            break;
        case CharacterClass::Mage:
            healthIncrease = 2;
            defenseIncrease = 1;
            speedIncrease = 1;
            break;
        case CharacterClass::Rogue:
            healthIncrease = 3;
            defenseIncrease = 1;
            speedIncrease = 1;
            break;
    }

    // Apply automatic increases
    charData.stats[cid].maxHealth += healthIncrease;
    charData.stats[cid].attack += attackIncrease;
    charData.stats[cid].defense += defenseIncrease;
    charData.stats[cid].speed += speedIncrease;

    if (charData.stats[cid].level % 2 == 0 && autoDistributePoints) {
        int pointsToDistribute = 5;
        int pointsPerStat = pointsToDistribute / 3;  // Divide points evenly
        int remainder = pointsToDistribute % 3;      // The leftover points

        // Distribute the evenly divided points
        switch(charData.characterClass[cid]) {
            case CharacterClass::Warrior: {
                charData.stats[cid].maxHealth += pointsPerStat;
                charData.stats[cid].attack += pointsPerStat;
                charData.stats[cid].defense += pointsPerStat;

                // Now distribute the remaining points
                if (remainder > 0) {
                    charData.stats[cid].attack++;  // Next point goes to attack
                    remainder--;
                }
                if (remainder > 0) {
                    charData.stats[cid].speed++;  // Extra point goes to health (for example)
                    remainder--;
                }
                if (remainder > 0) {
                    charData.stats[cid].defense++;  // Remaining point goes to defense
                    remainder--;
                }
                break;
            }
            case CharacterClass::Mage: {
                charData.stats[cid].maxHealth += pointsPerStat;
                charData.stats[cid].attack += pointsPerStat;
                charData.stats[cid].defense += pointsPerStat;

                // Distribute the remaining points
                if (remainder > 0) {
                    charData.stats[cid].attack++;  // Extra point goes to attack
                    remainder--;
                }
                if (remainder > 0) {
                    charData.stats[cid].maxHealth++;  // Next point goes to health
                    remainder--;
                }
                if (remainder > 0) {
                    charData.stats[cid].defense++;  // Last point goes to defense
                    remainder--;
                }
                break;
            }
            case CharacterClass::Rogue: {
                charData.stats[cid].maxHealth += pointsPerStat;
                charData.stats[cid].attack += pointsPerStat;
                charData.stats[cid].speed += pointsPerStat;

                // Distribute the remaining points
                if (remainder > 0) {
                    charData.stats[cid].attack++;  // Extra point goes to attack (for example)
                    remainder--;
                }
                if (remainder > 0) {
                    charData.stats[cid].defense++;  // Next point goes to defense
                    remainder--;
                }
                if (remainder > 0) {
                    charData.stats[cid].maxHealth++;  // Last point goes to health
                    remainder--;
                }
                break;
            }
        }
    }
    charData.stats[cid].health = charData.stats[cid].maxHealth;
}

int GetAttack(GameData& data, int cid) {
    int weaponItemId = GetEquippedItem(data, cid, ItemEquipSlot::Weapon1);
    if(weaponItemId != -1) {
        int tplIdx = GetItemTypeTemplateId(data, weaponItemId);
        return data.charData.stats[cid].attack + data.weaponData.templateData.stats[tplIdx].baseAttack;
    }
    return data.charData.stats[cid].attack;
}

void FaceCharacter(SpriteData& spriteData, CharacterData &charData, int attackerId, int defenderId) {
    // Determine the direction of movement and set the appropriate animation
    Vector2 start = GetCharacterSpritePos(spriteData, charData.sprite[attackerId]);
    Vector2 end = GetCharacterSpritePos(spriteData, charData.sprite[defenderId]);
    if (fabs(end.x - start.x) > fabs(end.y - start.y)) {
        // Horizontal movement
        if (end.x > start.x) {
            StartPausedCharacterSpriteAnim(spriteData, charData.sprite[attackerId], SpriteAnimationType::WalkRight, true);
            charData.orientation[attackerId] = Orientation::Right;
        } else {
            StartPausedCharacterSpriteAnim(spriteData, charData.sprite[attackerId], SpriteAnimationType::WalkLeft, true);
            charData.orientation[attackerId] = Orientation::Left;
        }
    } else {
        // Vertical movement
        if (end.y > start.y) {
            StartPausedCharacterSpriteAnim(spriteData, charData.sprite[attackerId], SpriteAnimationType::WalkDown, true);
            charData.orientation[attackerId] = Orientation::Down;
        } else {
            StartPausedCharacterSpriteAnim(spriteData, charData.sprite[attackerId], SpriteAnimationType::WalkUp, true);
            charData.orientation[attackerId] = Orientation::Up;
        }
    }
}

CharacterClass StringToClass(const std::string &className) {
    return CharacterClass::Warrior;
}

CharacterFaction StringToFaction(const std::string &factionName) {
    if(factionName == "Player") return CharacterFaction::Player;
    if(factionName == "Npc") return CharacterFaction::Npc;
    return CharacterFaction::Enemy;
}

int GetEquippedItem(const GameData& data, int charIdx, ItemEquipSlot slot) {
    return data.charData.equippedItemIdx[charIdx][static_cast<size_t>(slot)];
}

void SetEquippedItem(GameData& data, int charIdx, ItemEquipSlot slot, int itemIdx) {
    data.charData.equippedItemIdx[charIdx][static_cast<size_t>(slot)] = itemIdx;
    if(slot == ItemEquipSlot::Weapon1) {
        int tplIdx = GetItemTypeTemplateId(data, itemIdx);
        SetCharacterSpriteWeaponAnimation(data.spriteData, data.charData.sprite[charIdx], data.weaponData.templateData.animationTemplate[tplIdx]);
    }
}