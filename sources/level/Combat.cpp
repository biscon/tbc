//
// Created by bison on 09-01-25.
//

#include <algorithm>
#include <cmath>
#include <climits>
#include <cstring>
#include "Combat.h"
#include "util/Random.h"
#include "graphics/Animation.h"
#include "raylib.h"
#include "ui/UI.h"
#include "ui/CombatScreen.h"
#include "audio/SoundEffect.h"


static void InitializeThreatTable(LevelState& combat) {
    combat.threatTable.clear();
    for (const auto& player : combat.playerCharacters) {
        combat.threatTable[player] = 0;
    }
}

void DecayThreat(LevelState& combat, int amount) {
    for (auto& entry : combat.threatTable) {
        if (entry.first->health > 0) {
            entry.second = std::max(0, entry.second - amount); // Reduce threat but not below 0
        }
    }
}

Character* SelectTargetBasedOnThreat(LevelState& combat) {
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

void IncreaseThreat(LevelState& combat, Character* target, int amount) {
    if (target->health <= 0) return; // Skip dead characters

    // Separate additive and multiplicative modifiers
    float additiveModifier = 0.0f;
    float multiplicativeModifier = 1.0f;

    auto effects = GetStatusEffectsByType(target->statusEffects, StatusEffectType::ThreatModifier);
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

void SetTaunt(LevelState& combat, Character* target) {
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
    if (GetStatusEffectByType(character->statusEffects, StatusEffectType::Stun) != nullptr) {
        return true;
    }
    // Check if the character is dead
    if (character->health <= 0) {
        return true;
    }
    return false;
}


Character* GetFirstLivingEnemy(LevelState &combat) {
    for (auto &enemy : combat.enemyCharacters) {
        if (enemy->health > 0) {
            return enemy;
        }
    }
    return nullptr;
}

static int CalculateMissChance(Character &attacker, Character &defender) {
    // Base miss chance starts at 15% at speed = 3 and should scale to 1% at speed = 30
    int baseMissChance = 15;

    // Calculate miss chance adjustment based on speed (higher speed reduces miss chance)
    // For speed = 3 -> miss chance = 15%, for speed = 30 -> miss chance = 1%
    float missChanceReduction = ((float) attacker.speed - 3.0f) * 0.467f; // Adjust this multiplier for the desired scaling

    // Subtract the reduction from the base miss chance
    int missChance = baseMissChance - static_cast<int>(missChanceReduction);

    // Log for debugging
    TraceLog(LOG_INFO, "Base miss chance: %i, Speed: %i, Reduction: %.2f, missChance: %i", baseMissChance, attacker.speed, missChanceReduction, missChance);

    // Use helper function to get the rank of the Dodge skill
    int dodgeRank = GetSkillRank(defender.skills, SkillType::Dodge);

    // Apply dodge bonus if the character has the Dodge skill
    if (dodgeRank == 1) missChance += 5;  // +5% miss chance for Dodge Rank 1
    else if (dodgeRank == 2) missChance += 10;  // +10% miss chance for Dodge Rank 2
    else if (dodgeRank == 3) missChance += 15;  // +15% miss chance for Dodge Rank 3
    if(dodgeRank > 0) {
        TraceLog(LOG_INFO, "Dodge rank: %i, adjusted missChance: %i", dodgeRank, missChance);
    }

    // Defender is stunned, miss chance is zero
    if (IsIncapacitated(&defender)) {
        missChance = 0;
    }

    // Ensure miss chance doesn't go below 0% or above 100%
    if (missChance < 0) missChance = 0;
    if (missChance > 100) missChance = 100;

    return missChance;
}


static int CalculateDamageReduction(Character &defender, int baseDamage) {
    // loop through all status effects and calculate baseAttack reduction
    float damageReductionPct = 0;
    auto effects = GetStatusEffectsByType(defender.statusEffects, StatusEffectType::DamageReduction);
    for (auto &effect : effects) {
        TraceLog(LOG_INFO, "Damage reduction effect: %f", effect->value);
        damageReductionPct += effect->value;
    }
    return static_cast<int>((float) baseDamage * damageReductionPct);
}

// Function to calculate baseAttack dealt in combat, with chance to miss and critical hit
static void CalculateDamage(LevelState& combat, Character &attacker, Character &defender, AttackResult &result) {
    // Calculate miss chance based on defender's dodge skill and speed
    int missChance = CalculateMissChance(attacker, defender);
    int missRoll = RandomInRange(1, 100);  // Random roll between 1 and 100
    TraceLog(LOG_INFO, "Miss chance: %i, roll: %i", missChance, missRoll);

    // If roll is less than missChance, the attack misses
    if (missRoll <= missChance) {
        result.hit = false;
        //std::cout << attacker.name << " misses the attack!\n";
        std::string logMessage = attacker.name + " misses the attack!";
        combat.log.push_back(logMessage);
        result.damage = 0;
        return;  // No baseAttack if the attack misses
    } else {
        TraceLog(LOG_INFO, "Attack hits!");
        result.hit = true;
    }

    // Base baseAttack calculation
    float totalAttack = (float) attacker.attack;
    int effectiveDefense = defender.defense;
    if (attacker.equippedWeapon != nullptr) {
        int scalingStatValue = 0;
        switch(attacker.equippedWeapon->weaponTemplate->scalingStat) {
            case ScalingStat::Attack: scalingStatValue = attacker.attack; break;
            case ScalingStat::Speed: scalingStatValue = attacker.speed; break;
            default: break;
        }
        totalAttack = (float) scalingStatValue * attacker.equippedWeapon->weaponTemplate->attackMultiplier + (float) attacker.equippedWeapon->weaponTemplate->baseAttack;

        effectiveDefense = (int) ((float) defender.defense * (1.0f - attacker.equippedWeapon->weaponTemplate->armorPenetration));
    }
    TraceLog(LOG_INFO, "Total attack: %f", totalAttack);
    TraceLog(LOG_INFO, "Effective defense: %i", effectiveDefense);
    int attackerAttack = (int) totalAttack;

    int randomVariance = GetRandomValue(-2, 2); // Small random range
    int baseDamage = std::max(1, ((attackerAttack * attackerAttack) / (attackerAttack + effectiveDefense)) + randomVariance);
    TraceLog(LOG_INFO, "baseDamage: %i", baseDamage);
    int damageReduction = CalculateDamageReduction(defender, baseDamage);
    TraceLog(LOG_INFO, "Damage reduction: %i", damageReduction);
    baseDamage -= damageReduction;
    if (baseDamage < 0) baseDamage = 0;  // No negative baseAttack
    int critChance = static_cast<int>((2.0f + ((float) attacker.speed / 2.0f)));  // 2% base + 0.5% per speed point
    // Base critical hit chance (1%), diminishing returns
    //int critChance = static_cast<int>(sqrt(attacker.speed) * 3);  // Scales slower
    TraceLog(LOG_INFO, "Crit chance: %i", critChance);
    if (critChance > 25) critChance = 25;  // Max 25% crit chance
    int roll = RandomInRange(1, 100);  // Random roll between 1 and 100
    bool isCritical = roll <= critChance;

    // Apply critical multiplier
    if (isCritical) {
        result.crit = true;
        baseDamage *= 2;  // Double the baseAttack for critical hit
        std::string logMessage = "**Critical Hit!** ";
        combat.log.push_back(logMessage);
    }

    result.damage = baseDamage;
}

// Function for a character to attack another
AttackResult Attack(LevelState& combat, Character &attacker, Character &defender) {
    AttackResult result{};
    result.attacker = &attacker;
    result.defender = &defender;
    result.hit = false;
    result.crit = false;
    result.damage = 0;
    CalculateDamage(combat, attacker, defender, result);
    if (result.damage > 0) {
        if(IsPlayerCharacter(combat, attacker)) {
            // If the attacker is a player character, increase their threat
            IncreaseThreat(combat, &attacker, result.damage);
        }

        // Log the attack and baseAttack dealt
        std::string logMessage = attacker.name + " attacks " + defender.name + " for " + std::to_string(result.damage) + " baseAttack!";
        combat.log.push_back(logMessage);

        std::string logMessage2 = defender.name + " has " + std::to_string(defender.health) + " health left.";
        combat.log.push_back(logMessage2);
    }
    return result;
}

int DealDamage(LevelState& combat, Character &attacker, Character &defender, int damage) {
    float defenderX = GetCharacterSpritePosX(defender.sprite);
    float defenderY = GetCharacterSpritePosY(defender.sprite);
    float attackerX = GetCharacterSpritePosX(attacker.sprite);
    float attackerY = GetCharacterSpritePosY(attacker.sprite);

    // Base baseAttack calculation
    int baseDamage = damage;
    int damageReduction = CalculateDamageReduction(defender, baseDamage);
    TraceLog(LOG_INFO, "Damage reduction: %i", damageReduction);
    baseDamage -= damageReduction;
    if (baseDamage < 0) baseDamage = 0;  // No negative baseAttack
    //int critChance = 1 + (attacker.speed * 1);  // 1% base + 1% per speed point
    // Base critical hit chance (1%), diminishing returns
    int critChance = 1 + static_cast<int>(sqrt(attacker.speed) * 5);  // Scales slower
    if (critChance > 25) critChance = 25;  // Max 25% crit chance
    int roll = RandomInRange(1, 100);  // Random roll between 1 and 100
    bool isCritical = roll <= critChance;

    // Apply critical multiplier
    if (isCritical) {
        baseDamage *= 2;  // Double the baseAttack for critical hit
        //std::cout << "**Critical Hit!** ";
        std::string logMessage = "**Critical Hit!** ";
        combat.log.push_back(logMessage);
        /*
        Animation damageNumberAnim{};
        SetupDamageNumberAnimation(damageNumberAnim, "CRITICAL!!!", defenderX, defenderY-45, WHITE, 10);
        combat.animations.push_back(damageNumberAnim);
         */
    }
    Animation damageNumberAnim{};
    Color dmgColor = GetDamageColor(baseDamage, GetAttack(attacker));
    SetupDamageNumberAnimation(damageNumberAnim, TextFormat("%d", baseDamage), defenderX, defenderY-25, dmgColor, isCritical ? 20 : 10);
    combat.animations.push_back(damageNumberAnim);

    defender.health -= baseDamage;
    if(IsPlayerCharacter(combat, attacker)) {
        // If the attacker is a player character, increase their threat
        IncreaseThreat(combat, &attacker, damage);
    }

    // Ensure health does not drop below 0
    if (defender.health < 0) defender.health = 0;

    // Log the attack and baseAttack dealt
    std::string logMessage = attacker.name + " hits " + defender.name + " for " + std::to_string(damage) + " baseAttack!";
    combat.log.push_back(logMessage);

    std::string logMessage2 = defender.name + " has " + std::to_string(defender.health) + " health left.";
    combat.log.push_back(logMessage2);
    PlaySoundEffect(SoundEffectType::HumanPain, 0.25f);
    return baseDamage;
}

int DealDamageStatusEffect(LevelState& combat, Character &target, int damage) {
    float targetX = GetCharacterSpritePosX(target.sprite);
    float targetY = GetCharacterSpritePosY(target.sprite);

    // Base baseAttack calculation
    int baseDamage = damage;
    int damageReduction = CalculateDamageReduction(target, baseDamage);
    TraceLog(LOG_INFO, "Damage reduction: %i", damageReduction);
    baseDamage -= damageReduction;
    if (baseDamage < 0) baseDamage = 0;  // No negative baseAttack

    Animation damageNumberAnim{};
    Color dmgColor = GetDamageColor(baseDamage, GetAttack(*combat.currentCharacter));
    SetupDamageNumberAnimation(damageNumberAnim, TextFormat("%d", baseDamage), targetX, targetY-25, dmgColor, 10);
    combat.animations.push_back(damageNumberAnim);

    target.health -= baseDamage;

    // Ensure health does not drop below 0
    if (target.health < 0) target.health = 0;

    // Log the attack and baseAttack dealt
    std::string logMessage = target.name + " takes " + std::to_string(damage) + " baseAttack! (status effect)";
    combat.log.push_back(logMessage);

    std::string logMessage2 = target.name + " has " + std::to_string(target.health) + " health left.";
    combat.log.push_back(logMessage2);
    return baseDamage;
}

void KillCharacter(LevelState &combat, Character &character) {
    std::string logMessage = character.name + " is defeated!";
    combat.log.push_back(logMessage);
    Animation deathAnim{};
    SetupDeathAnimation(deathAnim, &character, 0.5f);
    combat.animations.push_back(deathAnim);
    character.health = 0;
    // Remove character from turn order
    //combat.turnOrder.erase(std::remove(combat.turnOrder.begin(), combat.turnOrder.end(), &character), combat.turnOrder.end());
    Animation bloodAnim{};
    SetupBloodPoolAnimation(bloodAnim, GetCharacterSpritePos(character.sprite), 5.0f);
    combat.animations.push_back(bloodAnim);
    PlaySoundEffect(SoundEffectType::HumanDeath, 0.5f);
}

bool IsPlayerCharacter(LevelState &combat, Character &character) {
    return (std::find(combat.playerCharacters.begin(), combat.playerCharacters.end(), &character) != combat.playerCharacters.end());
}



void InitCombat(LevelState &combat, std::vector<Character> &playerCharacters, std::vector<Character> &enemyCharacters) {
    memset(&combat, 0, sizeof(LevelState));
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
        return left.second < right.second;  // Compare by pointer address
    });

    // Now set the current order
    for (auto &pair : allCharacters) {
        combat.turnOrder.push_back(pair.second);
    }

    InitializeThreatTable(combat);

    combat.currentCharacter = combat.turnOrder[0];
    combat.currentCharacterIdx = 0;
    combat.nextState = TurnState::StartRound;
    combat.waitTime = 3.0f;
    combat.turnState = TurnState::Waiting;
    Animation textAnim{};
    SetupTextAnimation(textAnim, "First round!", 125, 2.0f, 0.0f);
    combat.animations.push_back(textAnim);

    InitLevelCamera(combat.camera);
}

void NextCharacter(LevelState &combat) {
    combat.currentCharacterIdx++;
    // skip dead characters
    while (combat.currentCharacterIdx < combat.turnOrder.size() && combat.turnOrder[combat.currentCharacterIdx]->health <= 0) {
        combat.currentCharacterIdx++;
    }
    bool nextRound = combat.currentCharacterIdx >= combat.turnOrder.size();
    // check for end of turn/round
    if (nextRound) {
        combat.currentCharacterIdx = 0;
    }
    // skip dead characters
    while (combat.currentCharacterIdx < combat.turnOrder.size() && combat.turnOrder[combat.currentCharacterIdx]->health <= 0) {
        combat.currentCharacterIdx++;
    }
    combat.selectedCharacter = nullptr;
    combat.currentCharacter = combat.turnOrder[combat.currentCharacterIdx];
    // log current character health
    std::string logMessage = combat.currentCharacter->name + " has " + std::to_string(combat.currentCharacter->health) + " health.";
    TraceLog(LOG_INFO, logMessage.c_str());
    if(nextRound) {
        combat.nextState = TurnState::EndRound;
        combat.waitTime = 1.0f;
    } else {
        combat.nextState = TurnState::StartTurn;
        combat.waitTime = 0.5f;
    }

    combat.turnState = TurnState::Waiting;
    TraceLog(LOG_INFO, "Next character: %s", combat.currentCharacter->name.c_str());
}
