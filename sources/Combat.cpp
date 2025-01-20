//
// Created by bison on 09-01-25.
//

#include <algorithm>
#include <cmath>
#include <limits.h>
#include "Combat.h"
#include "Random.h"
#include "Animation.h"
#include "raylib.h"
#include "Skills.h"

static void InitializeThreatTable(CombatState& combat) {
    combat.threatTable.clear();
    for (const auto& player : combat.playerCharacters) {
        combat.threatTable[player] = 0;
    }
}

void DecayThreat(CombatState& combat, int amount) {
    for (auto& entry : combat.threatTable) {
        if (entry.first->health > 0) {
            entry.second = std::max(0, entry.second - amount); // Reduce threat but not below 0
        }
    }
}

Character* SelectTargetBasedOnThreat(CombatState& combat) {
    // Find the highest threat value among alive characters
    auto highestThreat = std::max_element(
            combat.threatTable.begin(), combat.threatTable.end(),
            [](const std::pair<Character*, int>& a, const std::pair<Character*, int>& b) {
                // Ensure both characters are alive, otherwise treat their threat as minimal
                int threatA = (a.first->health > 0) ? a.second : INT_MIN;
                int threatB = (b.first->health > 0) ? b.second : INT_MIN;
                return threatA < threatB;
            }
    );

    // If all threats are zero or no valid targets exist, choose a random alive target
    if (highestThreat == combat.threatTable.end() || highestThreat->second <= 0 || highestThreat->first->health <= 0) {
        std::vector<Character*> aliveTargets;
        for (const auto& entry : combat.threatTable) {
            if (entry.first->health > 0) {
                aliveTargets.push_back(entry.first);
            }
        }

        if (!aliveTargets.empty()) {
            int randomIndex = RandomInRange(0, static_cast<int>(aliveTargets.size() - 1));
            return aliveTargets[randomIndex];
        }

        // If no alive targets exist (shouldn't happen in normal flow), return nullptr
        return nullptr;
    }

    // Otherwise, return the character with the highest threat
    return highestThreat->first;
}

void IncreaseThreat(CombatState& combat, Character* target, int amount) {
    if (target->health <= 0) return; // Skip dead characters

    // Separate additive and multiplicative modifiers
    float additiveModifier = 0.0f;
    float multiplicativeModifier = 1.0f;

    auto effects = GetStatusEffectsByType(*target, StatusEffectType::ThreatModifier);
    for (auto& effect : effects) {
        if (effect->value >= 1.0f) {
            // Treat values >= 1 as multiplicative modifiers
            multiplicativeModifier *= effect->value;
        } else {
            // Treat values < 1 as additive modifiers
            additiveModifier += effect->value;
        }
    }
    if(additiveModifier <= 0.0f) additiveModifier = 1.0f;
    // Apply modifiers: amount is adjusted by (1 + additive) * multiplicative
    float modifiedAmount = additiveModifier * multiplicativeModifier * static_cast<float>(amount);
    TraceLog(LOG_INFO, "Threat modified amount: %f", modifiedAmount);

    // Update threatTable
    if (combat.threatTable.find(target) != combat.threatTable.end()) {
        combat.threatTable[target] += static_cast<int>(modifiedAmount);
    }
}

void SetTaunt(CombatState& combat, Character* target) {
    if (combat.threatTable.find(target) != combat.threatTable.end() && target->health > 0) { // Skip dead characters
        int highestThreat = 0;
        for (const auto& entry : combat.threatTable) {
            if (entry.second > highestThreat && entry.first->health > 0) { // Skip dead characters
                highestThreat = entry.second;
            }
        }
        combat.threatTable[target] = highestThreat + 1; // Taunt sets the target as the highest threat.
    }
}

bool IsIncapacitated(Character* character) {
    // Check if the character is stunned
    if (GetStatusEffectByType(*character, StatusEffectType::Stun) != nullptr) {
        return true;
    }
    // Check if the character is dead
    if (character->health <= 0) {
        return true;
    }
    return false;
}


Character* GetFirstLivingEnemy(CombatState &combat) {
    for (auto &enemy : combat.enemyCharacters) {
        if (enemy->health > 0) {
            return enemy;
        }
    }
    return nullptr;
}

static int CalculateMissChance(Character &attacker, Character &defender) {
    int baseMissChance = 50; // Base miss chance starts at 50%
    int missChance = baseMissChance - (attacker.speed * 3); // Higher speed reduces miss chance

    // Use helper function to get the rank of the Dodge skill
    int dodgeRank = GetSkillRank(defender, SkillType::Dodge);

    // Apply dodge bonus if the character has the Dodge skill
    if (dodgeRank == 1) missChance += 5;  // +5% miss chance for Dodge Rank 1
    else if (dodgeRank == 2) missChance += 10;  // +10% miss chance for Dodge Rank 2
    else if (dodgeRank == 3) missChance += 15;  // +15% miss chance for Dodge Rank 3

    // defender is stunned, miss chance is zero
    if (IsIncapacitated(&defender)) {
        missChance = 0;
    }

    // Ensure miss chance doesn't go below 0% or above 100%
    if (missChance < 0) missChance = 0;
    if (missChance > 100) missChance = 100;

    return missChance;
}

static int CalculateDamageReduction(Character &defender, int baseDamage) {
    // loop through all status effects and calculate damage reduction
    float damageReductionPct = 0;
    auto effects = GetStatusEffectsByType(defender, StatusEffectType::DamageReduction);
    for (auto &effect : effects) {
        damageReductionPct += effect->value;
    }
    return static_cast<int>((float) baseDamage * damageReductionPct);
}

// Function to calculate damage dealt in combat, with chance to miss and critical hit
static int CalculateDamage(CombatState& combat, Character &attacker, Character &defender) {
    float defenderX = defender.sprite.player.position.x;
    float defenderY = defender.sprite.player.position.y;
    float attackerX = attacker.sprite.player.position.x;
    float attackerY = attacker.sprite.player.position.y;

    // Calculate miss chance based on defender's dodge skill and speed
    int missChance = CalculateMissChance(attacker, defender);
    int missRoll = RandomInRange(1, 100);  // Random roll between 1 and 100

    // If roll is less than missChance, the attack misses
    if (missRoll <= missChance) {
        //std::cout << attacker.name << " misses the attack!\n";
        std::string logMessage = attacker.name + " misses the attack!";
        combat.log.push_back(logMessage);
        Animation damageNumberAnim{};
        SetupDamageNumberAnimation(damageNumberAnim, "MISS", defenderX, defenderY-25, WHITE, 10);
        combat.animations.push_back(damageNumberAnim);
        return 0;  // No damage if the attack misses
    }

    // Base damage calculation
    int baseDamage = attacker.attack - defender.defense;
    int damageReduction = CalculateDamageReduction(defender, baseDamage);
    TraceLog(LOG_INFO, "Damage reduction: %i", damageReduction);
    baseDamage -= damageReduction;
    if (baseDamage < 0) baseDamage = 0;  // No negative damage
    //int critChance = 1 + (attacker.speed * 1);  // 1% base + 1% per speed point
    // Base critical hit chance (1%), diminishing returns
    int critChance = 1 + static_cast<int>(sqrt(attacker.speed) * 5);  // Scales slower
    if (critChance > 25) critChance = 25;  // Max 25% crit chance
    int roll = RandomInRange(1, 100);  // Random roll between 1 and 100
    bool isCritical = roll <= critChance;

    // Apply critical multiplier
    if (isCritical) {
        baseDamage *= 2;  // Double the damage for critical hit
        //std::cout << "**Critical Hit!** ";
        std::string logMessage = "**Critical Hit!** ";
        combat.log.push_back(logMessage);
        Animation damageNumberAnim{};
        SetupDamageNumberAnimation(damageNumberAnim, "CRITICAL!!!", defenderX, defenderY, WHITE, 10);
        combat.animations.push_back(damageNumberAnim);
    }
    Animation damageNumberAnim{};
    SetupDamageNumberAnimation(damageNumberAnim, TextFormat("%d", baseDamage), attackerX, attackerY-25, YELLOW, 20);
    combat.animations.push_back(damageNumberAnim);

    return baseDamage;
}

// Function for a character to attack another
void Attack(CombatState& combat, Character &attacker, Character &defender) {
    int damage = CalculateDamage(combat, attacker, defender);
    if (damage > 0) {
        defender.health -= damage;
        if(IsPlayerCharacter(combat, attacker)) {
            // If the attacker is a player character, increase their threat
            IncreaseThreat(combat, &attacker, damage);
        }

        // Ensure health does not drop below 0
        if (defender.health < 0) defender.health = 0;

        // Log the attack and damage dealt
        std::string logMessage = attacker.name + " attacks " + defender.name + " for " + std::to_string(damage) + " damage!";
        combat.log.push_back(logMessage);

        std::string logMessage2 = defender.name + " has " + std::to_string(defender.health) + " health left.";
        combat.log.push_back(logMessage2);
    }
}

bool IsPlayerCharacter(CombatState &combat, Character &character) {
    return (std::find(combat.playerCharacters.begin(), combat.playerCharacters.end(), &character) != combat.playerCharacters.end());
}



void InitCombat(CombatState &combat, std::vector<Character> &playerCharacters, std::vector<Character> &enemyCharacters) {
    std::vector<std::pair<int, Character*>> allCharacters;
    for (auto & playerCharacter : playerCharacters) {
        allCharacters.emplace_back(playerCharacter.speed, &playerCharacter);
        combat.playerCharacters.emplace_back(&playerCharacter);
    }
    for (auto & enemyCharacter : enemyCharacters) {
        allCharacters.emplace_back(enemyCharacter.speed, &enemyCharacter);
        combat.enemyCharacters.emplace_back(&enemyCharacter);
    }

    // Sort by speed, then randomize in case of tie
    std::sort(allCharacters.begin(), allCharacters.end(), [](std::pair<int, Character*> &left, std::pair<int, Character*> &right) {
        if (left.first != right.first) return left.first > right.first;
        return rand() % 2 == 0;  // Randomize tie-breaking
    });

    // Now set the current order
    for (auto &pair : allCharacters) {
        combat.turnOrder.push_back(pair.second);
    }

    InitializeThreatTable(combat);

    combat.currentCharacter = combat.turnOrder[0];
    combat.currentCharacterIdx = 0;
    combat.nextState = TurnState::StartTurn;
    combat.waitTime = 3.0f;
    combat.turnState = TurnState::Waiting;
    Animation textAnim{};
    SetupTextAnimation(textAnim, "First round!", 125, 2.0f, 0.0f);
    combat.animations.push_back(textAnim);
}

