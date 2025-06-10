//
// Created by bison on 09-01-25.
//

#include <algorithm>
#include <cmath>
#include <climits>
#include <queue>
#include <unordered_set>
#include <cassert>
#include "Combat.h"
#include "util/Random.h"
#include "graphics/Animation.h"
#include "raylib.h"
#include "ui/UI.h"
#include "LevelScreen.h"
#include "audio/SoundEffect.h"
#include "ai/PathFinding.h"

void DecayThreat(CharacterData& charData, Level& level, int amount) {
    for (auto& entry : level.threatTable) {
        if (charData.stats[entry.first].health > 0) {
            entry.second = std::max(0, entry.second - amount); // Reduce threat but not below 0
        }
    }
}

/*
Character* SelectTargetBasedOnThreat(Level& level) {
    // Find the highest threat value among alive characters
    auto highestThreat = std::max_element(
            level.threatTable.begin(), level.threatTable.end(),
            [](const std::pair<Character*, int>& a, const std::pair<Character*, int>& b) {
                // Ensure both characters are alive, otherwise treat their threat as minimal
                int threatA = (a.first->health > 0) ? a.second : INT_MIN;
                int threatB = (b.first->health > 0) ? b.second : INT_MIN;
                return threatA < threatB;
            }
    );

    // If all threats are zero or no valid targets exist, choose a random alive target
    if (highestThreat == level.threatTable.end() || highestThreat->second <= 0 || highestThreat->first->health <= 0) {
        std::vector<Character*> aliveTargets;
        for (const auto& entry : level.threatTable) {
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
*/

void IncreaseThreat(CharacterData& charData, Level& combat, int target, int amount) {
    if (charData.stats[target].health <= 0) return; // Skip dead characters

    // Separate additive and multiplicative modifiers
    float additiveModifier = 0.0f;
    float multiplicativeModifier = 1.0f;

    auto effects = GetStatusEffectsByType(charData.statusEffects[target], StatusEffectType::ThreatModifier);
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

void SetTaunt(CharacterData& charData, Level& level, int target) {
    if (level.threatTable.find(target) != level.threatTable.end() && charData.stats[target].health > 0) { // Skip dead characters
        int highestThreat = 0;
        for (const auto& entry : level.threatTable) {
            if (entry.second > highestThreat && charData.stats[entry.first].health > 0) { // Skip dead characters
                highestThreat = entry.second;
            }
        }
        level.threatTable[target] = highestThreat + 1; // Taunt sets the target as the highest threat.
    }
}

bool IsIncapacitated(CharacterData& charData, int character) {
    // Check if the character is stunned
    if (GetStatusEffectByType(charData.statusEffects[character], StatusEffectType::Stun) != nullptr) {
        return true;
    }
    // Check if the character is dead
    if (charData.stats[character].health <= 0) {
        return true;
    }
    return false;
}

static int CalculateMissChance(CharacterData& charData, int attacker, int defender) {
    // Base miss chance starts at 15% at speed = 3 and should scale to 1% at speed = 30
    int baseMissChance = 15;

    CharacterStats& atkStats = charData.stats[attacker];
    CharacterStats& defStats = charData.stats[defender];
    // Calculate miss chance adjustment based on speed (higher speed reduces miss chance)
    // For speed = 3 -> miss chance = 15%, for speed = 30 -> miss chance = 1%
    float missChanceReduction = ((float) atkStats.speed - 3.0f) * 0.467f; // Adjust this multiplier for the desired scaling

    // Subtract the reduction from the base miss chance
    int missChance = baseMissChance - static_cast<int>(missChanceReduction);

    // Log for debugging
    TraceLog(LOG_INFO, "Base miss chance: %i, Speed: %i, Reduction: %.2f, missChance: %i", baseMissChance, atkStats.speed, missChanceReduction, missChance);

    // Use helper function to get the rank of the Dodge skill
    int dodgeRank = GetSkillRank(charData.skills[defender], SkillType::Dodge);

    // Apply dodge bonus if the character has the Dodge skill
    if (dodgeRank == 1) missChance += 5;  // +5% miss chance for Dodge Rank 1
    else if (dodgeRank == 2) missChance += 10;  // +10% miss chance for Dodge Rank 2
    else if (dodgeRank == 3) missChance += 15;  // +15% miss chance for Dodge Rank 3
    if(dodgeRank > 0) {
        TraceLog(LOG_INFO, "Dodge rank: %i, adjusted missChance: %i", dodgeRank, missChance);
    }

    // Defender is stunned, miss chance is zero
    if (IsIncapacitated(charData, defender)) {
        missChance = 0;
    }

    // Ensure miss chance doesn't go below 0% or above 100%
    if (missChance < 0) missChance = 0;
    if (missChance > 100) missChance = 100;

    return missChance;
}


static int CalculateDamageReduction(CharacterData& charData, int defender, int baseDamage) {
    // loop through all status effects and calculate baseAttack reduction
    float damageReductionPct = 0;
    auto effects = GetStatusEffectsByType(charData.statusEffects[defender], StatusEffectType::DamageReduction);
    for (auto &effect : effects) {
        TraceLog(LOG_INFO, "Damage reduction effect: %f", effect->value);
        damageReductionPct += effect->value;
    }
    return static_cast<int>((float) baseDamage * damageReductionPct);
}

// Function to calculate baseAttack dealt in combat, with chance to miss and critical hit
static void CalculateDamage(CharacterData& charData, WeaponData& weaponData, Level& combat, int attacker, int defender, AttackResult &result) {
    // Calculate miss chance based on defender's dodge skill and speed
    int missChance = CalculateMissChance(charData, attacker, defender);
    int missRoll = RandomInRange(1, 100);  // Random roll between 1 and 100
    TraceLog(LOG_INFO, "Miss chance: %i, roll: %i", missChance, missRoll);

    // If roll is less than missChance, the attack misses
    if (missRoll <= missChance) {
        result.hit = false;
        //std::cout << attacker.name << " misses the attack!\n";
        std::string logMessage = charData.name[attacker] + " misses the attack!";
        combat.log.push_back(logMessage);
        result.damage = 0;
        return;  // No baseAttack if the attack misses
    } else {
        TraceLog(LOG_INFO, "Attack hits!");
        result.hit = true;
    }

    CharacterStats& atkStats = charData.stats[attacker];
    CharacterStats& defStats = charData.stats[defender];

    // Base baseAttack calculation
    float totalAttack = (float) atkStats.attack;
    int effectiveDefense = defStats.defense;

    if (charData.isWeaponEquipped[attacker] && charData.weaponIdx[attacker] != -1) {
        int scalingStatValue = 0;
        int tplIdx = weaponData.instanceData.weaponTemplateIdx[charData.weaponIdx[attacker]];
        switch (weaponData.templateData.stats[tplIdx].scalingStat) {
            case ScalingStat::Attack: scalingStatValue = atkStats.attack; break;
            case ScalingStat::Speed: scalingStatValue = atkStats.speed; break;
            default: break;
        }
        totalAttack = (float) scalingStatValue * weaponData.templateData.stats[tplIdx].attackMultiplier +
                      (float) weaponData.templateData.stats[tplIdx].baseAttack;
        assert(totalAttack < 100.0f);
        effectiveDefense = (int)((float) defStats.defense * (1.0f - weaponData.templateData.stats[tplIdx].armorPenetration));
    }

    TraceLog(LOG_INFO, "Total attack: %f", totalAttack);
    TraceLog(LOG_INFO, "Effective defense: %i", effectiveDefense);
    int attackerAttack = (int) totalAttack;

    int randomVariance = GetRandomValue(-2, 2); // Small random range
    int baseDamage = std::max(1, ((attackerAttack * attackerAttack) / (attackerAttack + effectiveDefense)) + randomVariance);
    TraceLog(LOG_INFO, "baseDamage: %i", baseDamage);
    int damageReduction = CalculateDamageReduction(charData, defender, baseDamage);
    TraceLog(LOG_INFO, "Damage reduction: %i", damageReduction);
    baseDamage -= damageReduction;
    if (baseDamage < 0) baseDamage = 0;  // No negative baseAttack
    int critChance = static_cast<int>((2.0f + ((float) atkStats.speed / 2.0f)));  // 2% base + 0.5% per speed point
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
AttackResult Attack(CharacterData& charData, WeaponData& weaponData, Level& level, int attacker, int defender) {
    AttackResult result{};
    result.attacker = attacker;
    result.defender = defender;
    result.hit = false;
    result.crit = false;
    result.damage = 0;
    CalculateDamage(charData, weaponData, level, attacker, defender, result);
    if (result.damage > 0) {
        if(IsPlayerCharacter(charData, attacker)) {
            // If the attacker is a player character, increase their threat
            IncreaseThreat(charData, level, attacker, result.damage);
        }

        // Log the attack and baseAttack dealt
        std::string logMessage = charData.name[attacker] + " attacks " + charData.name[defender] + " for " + std::to_string(result.damage) + " damage!";
        level.log.push_back(logMessage);

        std::string logMessage2 = charData.name[defender] + " has " + std::to_string(charData.stats[defender].health) + " health left.";
        level.log.push_back(logMessage2);
    }
    return result;
}

int DealDamage(SpriteData& spriteData, CharacterData& charData, WeaponData& weaponData, Level& level, int attacker, int defender, int damage) {
    CharacterSprite& defenderSprite = charData.sprite[defender];
    float defenderX = GetCharacterSpritePosX(spriteData, defenderSprite);
    float defenderY = GetCharacterSpritePosY(spriteData, defenderSprite);

    // Base baseAttack calculation
    int baseDamage = damage;
    int damageReduction = CalculateDamageReduction(charData, defender, baseDamage);
    TraceLog(LOG_INFO, "Damage reduction: %i", damageReduction);
    baseDamage -= damageReduction;
    if (baseDamage < 0) baseDamage = 0;  // No negative baseAttack
    //int critChance = 1 + (attacker.speed * 1);  // 1% base + 1% per speed point
    // Base critical hit chance (1%), diminishing returns
    int critChance = 1 + static_cast<int>(sqrt(charData.stats[attacker].speed) * 5);  // Scales slower
    if (critChance > 25) critChance = 25;  // Max 25% crit chance
    int roll = RandomInRange(1, 100);  // Random roll between 1 and 100
    bool isCritical = roll <= critChance;

    // Apply critical multiplier
    if (isCritical) {
        baseDamage *= 2;  // Double the baseAttack for critical hit
        //std::cout << "**Critical Hit!** ";
        std::string logMessage = "**Critical Hit!** ";
        level.log.push_back(logMessage);
        /*
        Animation damageNumberAnim{};
        SetupDamageNumberAnimation(damageNumberAnim, "CRITICAL!!!", defenderX, defenderY-45, WHITE, 10);
        combat.animations.push_back(damageNumberAnim);
         */
    }
    Animation damageNumberAnim{};
    Color dmgColor = GetDamageColor(baseDamage, GetAttack(charData, weaponData, attacker));
    SetupDamageNumberAnimation(damageNumberAnim, TextFormat("%d", baseDamage), defenderX, defenderY-25, dmgColor, isCritical ? 20 : 10);
    level.animations.push_back(damageNumberAnim);

    charData.stats[defender].health -= baseDamage;
    if(IsPlayerCharacter(charData, attacker)) {
        // If the attacker is a player character, increase their threat
        IncreaseThreat(charData, level, attacker, damage);
    }

    // Ensure health does not drop below 0
    if (charData.stats[defender].health < 0) charData.stats[defender].health = 0;

    // Log the attack and baseAttack dealt
    std::string logMessage = charData.name[attacker] + " hits " + charData.name[defender] + " for " + std::to_string(damage) + " baseAttack!";
    level.log.push_back(logMessage);

    std::string logMessage2 = charData.name[defender] + " has " + std::to_string(charData.stats[defender].health) + " health left.";
    level.log.push_back(logMessage2);
    PlaySoundEffect(SoundEffectType::HumanPain, 0.25f);
    return baseDamage;
}

int DealDamageStatusEffect(SpriteData& spriteData, CharacterData& charData, WeaponData& weaponData, Level& level, int target, int damage) {
    float targetX = GetCharacterSpritePosX(spriteData, charData.sprite[target]);
    float targetY = GetCharacterSpritePosY(spriteData, charData.sprite[target]);

    // Base baseAttack calculation
    int baseDamage = damage;
    int damageReduction = CalculateDamageReduction(charData, target, baseDamage);
    TraceLog(LOG_INFO, "Damage reduction: %i", damageReduction);
    baseDamage -= damageReduction;
    if (baseDamage < 0) baseDamage = 0;  // No negative baseAttack

    Animation damageNumberAnim{};
    Color dmgColor = GetDamageColor(baseDamage, GetAttack(charData, weaponData, level.currentCharacter));
    SetupDamageNumberAnimation(damageNumberAnim, TextFormat("%d", baseDamage), targetX, targetY-25, dmgColor, 10);
    level.animations.push_back(damageNumberAnim);

    CharacterStats& stats = charData.stats[target];
    stats.health -= baseDamage;

    // Ensure health does not drop below 0
    if (stats.health < 0) stats.health = 0;

    // Log the attack and baseAttack dealt
    std::string logMessage = charData.name[target] + " takes " + std::to_string(damage) + " baseAttack! (status effect)";
    level.log.push_back(logMessage);

    std::string logMessage2 = charData.name[target] + " has " + std::to_string(stats.health) + " health left.";
    level.log.push_back(logMessage2);
    return baseDamage;
}

void KillCharacter(SpriteData& spriteData, CharacterData& charData, Level &level, int character) {
    std::string logMessage = charData.name[character] + " is defeated!";
    level.log.push_back(logMessage);
    Animation deathAnim{};
    SetupDeathAnimation(spriteData, charData, deathAnim, character, 0.5f);
    level.animations.push_back(deathAnim);
    charData.stats[character].health = 0;
    // Remove character from turn order
    //combat.turnOrder.erase(std::remove(combat.turnOrder.begin(), combat.turnOrder.end(), &character), combat.turnOrder.end());
    Animation bloodAnim{};
    Vector2 bloodPos = GetCharacterSpritePos(spriteData, charData.sprite[character]);
    //bloodPos = GetWorldToScreen2D(bloodPos, combat.camera.camera);
    SetupBloodPoolAnimation(bloodAnim, bloodPos, 5.0f);
    level.animations.push_back(bloodAnim);
    PlaySoundEffect(SoundEffectType::HumanDeath, 0.5f);
}

bool IsPlayerCharacter(CharacterData& charData, int character) {
    return charData.faction[character] == CharacterFaction::Player;
}

void NextCharacter(CharacterData& charData, Level &level) {
    level.currentCharacterIdx++;
    // skip dead characters
    while (level.currentCharacterIdx < level.turnOrder.size() && charData.stats[level.turnOrder[level.currentCharacterIdx]].health <= 0) {
        level.currentCharacterIdx++;
    }
    bool nextRound = level.currentCharacterIdx >= level.turnOrder.size();
    // check for end of turn/round
    if (nextRound) {
        level.currentCharacterIdx = 0;
    }
    // skip dead characters
    while (level.currentCharacterIdx < level.turnOrder.size() && charData.stats[level.turnOrder[level.currentCharacterIdx]].health <= 0) {
        level.currentCharacterIdx++;
    }
    level.selectedCharacter = -1;
    level.currentCharacter = level.turnOrder[level.currentCharacterIdx];
    // log current character health
    std::string logMessage = charData.name[level.currentCharacter] + " has " + std::to_string(charData.stats[level.currentCharacter].health) + " health.";
    TraceLog(LOG_INFO, logMessage.c_str());
    if(nextRound) {
        level.nextState = TurnState::EndRound;
        level.waitTime = 1.0f;
    } else {
        level.nextState = TurnState::StartTurn;
        level.waitTime = 0.5f;
    }

    level.turnState = TurnState::Waiting;
    TraceLog(LOG_INFO, "Next character: %s", charData.name[level.currentCharacter].c_str());
}

static void InitializeThreatTable(Level& level) {
    level.threatTable.clear();
    for (const auto& player : level.partyCharacters) {
        level.threatTable[player] = 0;
    }
}

void StartCombat(SpriteData& spriteData, CharacterData& charData, Level &level, int spotter, int maxDist) {
    level.turnOrder.clear();
    level.enemyCharacters.clear();
    std::unordered_set<int> alertedEnemies;

    alertedEnemies.insert(spotter);
    std::string groupId = level.enemyGroups[spotter];
    level.currentEnemyGroup = groupId;
    for (int enemy : level.allCharacters) {
        if (level.enemyGroups[enemy] == groupId && charData.faction[enemy] == CharacterFaction::Enemy && IsAlive(charData, enemy)) {
            // add to encounter
            alertedEnemies.insert(enemy);
        }
    }

    if(!alertedEnemies.empty()) {
        TraceLog(LOG_INFO, "%d enemies alerted.", alertedEnemies.size());
    }

    std::vector<std::pair<int, int>> allCharacters;
    for (auto &enemy : alertedEnemies) {
        level.enemyCharacters.emplace_back(enemy);
        allCharacters.emplace_back(charData.stats[enemy].speed, enemy);
        FaceCharacter(spriteData, charData, enemy, level.partyCharacters[0]);
    }
    for (auto &c : level.partyCharacters) {
        allCharacters.emplace_back(charData.stats[c].speed, c);
    }

    // Sort by speed, then randomize in case of tie
    std::sort(allCharacters.begin(), allCharacters.end(), [](std::pair<int, int> &left, std::pair<int, int> &right) {
        if (left.first != right.first) return left.first > right.first;
        return left.second < right.second;  // index
    });

    // Now set the current order
    for (auto &pair : allCharacters) {
        level.turnOrder.push_back(pair.second);
    }

    InitializeThreatTable(level);

    level.currentCharacter = level.turnOrder[0];
    level.currentCharacterIdx = 0;

    level.nextState = TurnState::StartRound;
    level.waitTime = 1.0f;
    level.turnState = TurnState::Waiting;
    Animation textAnim{};
    SetupTextAnimation(textAnim, "Entering Combat", 125, 2.0f, 0.0f);
    level.animations.push_back(textAnim);
}
