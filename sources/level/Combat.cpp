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
#include "game/Items.h"
#include "raymath.h"

bool IsIncapacitated(CharacterData& charData, int character) {
    // Check if the character is stunned
    if (GetStatusEffectByType(charData.statusEffects[character], StatusEffectType::Stun) != nullptr) {
        return true;
    }
    // Check if the character is dead
    if (charData.stats[character].HP <= 0) {
        return true;
    }
    return false;
}

static AttackResult AttackUnarmed(GameData& data, Level& level, int attacker, int defender) {
    CharacterStats& attackerStats = data.charData.stats[attacker];
    AttackResult result{};
    result.attacker = attacker;
    result.defender = defender;
    int unarmedMod = attackerStats.STR / 2;
    result.minDmg = 1 + unarmedMod;
    result.maxDmg = 3 + unarmedMod;
    AttackInfo info{};
    CalcHitChance(data, attacker, GetSelectedWeaponItemId(data, attacker), -1, info);
    int hitRoll = GetRandomValue(1, 100);
    AttackHit hit{};
    hit.hit = false;
    hit.damage = 0;
    hit.crit = false;
    if(hitRoll <= (int) info.hitChance) {

        hit.hit = true;
        float totalCritChance = 2 + ((float) attackerStats.LUK * 1.0f); // 1% per LUK
        int rolledDmg = GetRandomValue(result.minDmg, result.maxDmg);
        hit.damage = rolledDmg + unarmedMod;
        hit.crit = GetRandomFloat01() < (totalCritChance/100);
        if (hit.crit) {
            hit.damage = static_cast<int>((float) hit.damage * 2.0f);
        }
    }
    result.hits.push_back(hit);
    attackerStats.AP -= info.apCost;
    return result;
}

static AttackResult AttackMelee(GameData& data, Level& level, int attacker, int defender, WeaponTemplate& weaponTemplate, int fireMode) {
    CharacterStats& attackerStats = data.charData.stats[attacker];
    AttackResult result{};
    result.attacker = attacker;
    result.defender = defender;
    int meleeMod = attackerStats.STR / 2;
    result.minDmg = weaponTemplate.minDamage + meleeMod;
    result.maxDmg = weaponTemplate.maxDamage + meleeMod;
    AttackInfo info{};
    CalcHitChance(data, attacker, GetSelectedWeaponItemId(data, attacker), fireMode, info);
    int hitRoll = GetRandomValue(1, 100);
    AttackHit hit{};
    hit.hit = false;
    hit.damage = 0;
    hit.crit = false;
    if(hitRoll <= (int) info.hitChance) {
        hit.hit = true;
        float totalCritChance = weaponTemplate.critChance + ((float) attackerStats.LUK * 1.0f); // 1% per LUK
        int rolledDmg = GetRandomValue(weaponTemplate.minDamage, weaponTemplate.maxDamage);
        hit.damage = rolledDmg + meleeMod;
        hit.crit = GetRandomFloat01() < (totalCritChance/100);
        if (hit.crit) {
            hit.damage = static_cast<int>((float) hit.damage * weaponTemplate.critMultiplier);
        }
    }
    result.hits.push_back(hit);
    attackerStats.AP -= info.apCost;
    return result;
}

static AttackResult AttackRanged(GameData& data, Level& level, int attacker, int defender,
                                 WeaponTemplate& weaponTemplate, WeaponRanged& rangedTemplate,
                                 WeaponInstance& weaponInstance, int fireMode) {
    CharacterStats& attackerStats = data.charData.stats[attacker];
    AttackResult result{};
    result.attacker = attacker;
    result.defender = defender;

    //int rangedMod = attackerStats.PER / 2;
    int rangedMod = 0;
    result.minDmg = weaponTemplate.minDamage + rangedMod;
    result.maxDmg = weaponTemplate.maxDamage + rangedMod;

    const FireMode& fm = rangedTemplate.fireModes[fireMode];
    int roundsFired = fm.roundsFired;
    // fire up to 8 bullets
    if(roundsFired == -1) {
        roundsFired = weaponInstance.currentAmmo >= FULL_AUTO_SHOTS ? FULL_AUTO_SHOTS : weaponInstance.currentAmmo;
    }
    for(int i = 0; i < roundsFired; i++) {
        AttackInfo info{};
        CalcHitChance(data, attacker, GetSelectedWeaponItemId(data, attacker), fireMode, info);
        int hitRoll = GetRandomValue(1, 100);
        AttackHit hit{};
        hit.hit = false;
        hit.damage = 0;
        hit.crit = false;
        if(hitRoll <= (int) info.hitChance) {
            hit.hit = true;
            float totalCritChance = fm.critChance + ((float) attackerStats.LUK * 1.0f); // 1% per LUK
            int rolledDmg = GetRandomValue(weaponTemplate.minDamage, weaponTemplate.maxDamage);
            hit.damage = rolledDmg + rangedMod;
            hit.crit = GetRandomFloat01() < (totalCritChance/100);
            if (hit.crit) {
                hit.damage = static_cast<int>((float) hit.damage * fm.critMultiplier);
            }
        }
        result.hits.push_back(hit);
    }
    weaponInstance.currentAmmo = std::max(0, weaponInstance.currentAmmo - roundsFired);
    attackerStats.AP -= fm.apCost;
    return result;
}

// Function for a character to attack another
AttackResult Attack(GameData& data, Level& level, int attacker, int defender, int fireMode) {
    WeaponTemplate* weaponTemplate = GetSelectedWeaponTemplate(data, attacker);
    WeaponRanged* weaponRanged = GetSelectedRangedTemplate(data, attacker);
    WeaponInstance* weaponInstance = GetSelectedWeaponInstance(data, attacker);

    if(weaponTemplate == nullptr) {
        return AttackUnarmed(data, level, attacker, defender);
    } else {
        switch(weaponTemplate->type) {
            case WeaponType::Melee:return AttackMelee(data, level, attacker, defender, *weaponTemplate, fireMode);
            case WeaponType::Ranged: return AttackRanged(data, level, attacker, defender, *weaponTemplate, *weaponRanged, *weaponInstance, fireMode);
        }
    }
    throw(std::runtime_error("Illegal state"));
}

int DealDamage(GameData& data, Level& level, int attacker, int defender, int damage) {
    SpriteData& spriteData = data.spriteData;
    CharacterData& charData = data.charData;

    CharacterSprite& defenderSprite = charData.sprite[defender];
    float defenderX = GetCharacterSpritePosX(spriteData, defenderSprite);
    float defenderY = GetCharacterSpritePosY(spriteData, defenderSprite);

    // Base baseAttack calculation
    int baseDamage = damage;

    Animation damageNumberAnim{};
    Color dmgColor = GetDamageColor(baseDamage, 1, 20);
    bool isCritical = false;
    SetupDamageNumberAnimation(damageNumberAnim, TextFormat("%d", baseDamage), defenderX, defenderY-25, dmgColor, isCritical ? 20 : 10, 0);
    level.animations.push_back(damageNumberAnim);

    charData.stats[defender].HP -= baseDamage;

    // Ensure health does not drop below 0
    if (charData.stats[defender].HP < 0) charData.stats[defender].HP = 0;

    PlaySoundEffect(SoundEffectType::HumanPain, 0.25f);
    return baseDamage;
}

int DealDamageStatusEffect(GameData& data, Level& level, int target, int damage) {
    SpriteData& spriteData = data.spriteData;
    CharacterData& charData = data.charData;

    float targetX = GetCharacterSpritePosX(spriteData, charData.sprite[target]);
    float targetY = GetCharacterSpritePosY(spriteData, charData.sprite[target]);

    // Base baseAttack calculation
    int baseDamage = damage;
    int damageReduction = 0;
    TraceLog(LOG_INFO, "Damage reduction: %i", damageReduction);
    baseDamage -= damageReduction;
    if (baseDamage < 0) baseDamage = 0;  // No negative baseAttack

    Animation damageNumberAnim{};
    Color dmgColor = WHITE;
    SetupDamageNumberAnimation(damageNumberAnim, TextFormat("%d", baseDamage), targetX, targetY-25, dmgColor, 10, 0);
    level.animations.push_back(damageNumberAnim);

    CharacterStats& stats = charData.stats[target];
    stats.HP -= baseDamage;

    // Ensure health does not drop below 0
    if (stats.HP < 0) stats.HP = 0;

    return baseDamage;
}

void KillCharacter(SpriteData& spriteData, CharacterData& charData, Level &level, int character) {
    std::string logMessage = charData.name[character] + " is defeated!";
    level.log.push_back(logMessage);
    Animation deathAnim{};
    SetupDeathAnimation(spriteData, charData, deathAnim, character, 0.5f);
    level.animations.push_back(deathAnim);
    charData.stats[character].HP = 0;
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
    while (level.currentCharacterIdx < level.turnOrder.size() && charData.stats[level.turnOrder[level.currentCharacterIdx]].HP <= 0) {
        level.currentCharacterIdx++;
    }
    bool nextRound = level.currentCharacterIdx >= level.turnOrder.size();
    // check for end of turn/round
    if (nextRound) {
        level.currentCharacterIdx = 0;
    }
    // skip dead characters
    while (level.currentCharacterIdx < level.turnOrder.size() && charData.stats[level.turnOrder[level.currentCharacterIdx]].HP <= 0) {
        level.currentCharacterIdx++;
    }
    level.selectedCharacter = -1;
    level.currentCharacter = level.turnOrder[level.currentCharacterIdx];
    // log current character health
    std::string logMessage = charData.name[level.currentCharacter] + " has " + std::to_string(charData.stats[level.currentCharacter].HP) + " health.";
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

void StartCombat(SpriteData& spriteData, CharacterData& charData, Level &level, int spotter) {
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
        allCharacters.emplace_back(CalculateCharInitiative(charData.stats[enemy]), enemy);
        FaceCharacter(spriteData, charData, enemy, level.partyCharacters[0]);
    }
    for (auto &c : level.partyCharacters) {
        allCharacters.emplace_back(CalculateCharInitiative(charData.stats[c]), c);
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

    level.currentCharacter = level.turnOrder[0];
    level.currentCharacterIdx = 0;

    level.nextState = TurnState::StartRound;
    level.waitTime = 1.0f;
    level.turnState = TurnState::Waiting;
    Animation textAnim{};
    SetupTextAnimation(textAnim, "Entering Combat", 150, 2.0f, 0.0f);
    level.animations.push_back(textAnim);
}

void CalcHitChance(GameData& data, int charId, int weaponItemId, int fireModeIdx, AttackInfo& info) {
    /*
    hitChance = skillValue
                + (PER / 2)
                + weapon.weaponAccuracy
                + fireMode.accuracyMod
                - target.evasion;
    */
    if(weaponItemId != -1) {
        if (data.itemData.templateData[GetItemTemplateId(data, weaponItemId)].type != ItemType::Weapon) {
            throw std::runtime_error("Item type must be Weapon");
        }
        CharacterStats& stats = data.charData.stats[charId];
        int weaponTplId = GetItemTypeTemplateId(data, weaponItemId);
        WeaponTemplate& weaponTemplate = data.weaponData.templateData[weaponTplId];
        info.apCost = weaponTemplate.apCost;
        Skill skill = SkillIdToEnum(weaponTemplate.skillUsed);
        float hitChance = (float) GetSkillValue(data, skill, charId)
                          + floorf((float) stats.PER / 2)
                          + weaponTemplate.weaponAccuracy;
        if(weaponTemplate.rangeDataId != -1) {
            WeaponRanged& ranged = data.weaponData.rangedData[weaponTemplate.rangeDataId];
            if(fireModeIdx != -1) {
                auto& fm = ranged.fireModes.at(fireModeIdx);
                hitChance += fm.accuracyMod;
                info.apCost = fm.apCost;
                info.ammoCost = fm.roundsFired == -1 ? FULL_AUTO_SHOTS : fm.roundsFired;
            }
        }
        info.hitChance = Clamp(hitChance, 5, 95);
    } else { // unarmed
        CharacterStats& stats = data.charData.stats[charId];
        float hitChance = (float) GetSkillValue(data, Skill::Melee, charId)
                          + floorf((float) stats.PER / 2);
        info.hitChance = Clamp(hitChance, 5, 95);
        info.apCost = 3;
    }
}

int GetAttackAPCost(GameData& data, int attacker, int fireMode) {
    int weaponItemId = GetSelectedWeaponItemId(data, attacker);
    if (data.itemData.templateData[GetItemTemplateId(data, weaponItemId)].type != ItemType::Weapon) {
        throw std::runtime_error("Item type must be Weapon");
    }
    int weaponTplId = GetItemTypeTemplateId(data, weaponItemId);
    WeaponTemplate& weaponTemplate = data.weaponData.templateData[weaponTplId];
    int apCost = weaponTemplate.apCost;
    if(weaponTemplate.rangeDataId != -1) {
        WeaponRanged& ranged = data.weaponData.rangedData[weaponTemplate.rangeDataId];
        if(fireMode != -1) {
            auto& fm = ranged.fireModes.at(fireMode);
            apCost = fm.apCost;
        }
    }
    return apCost;
}

int GetCurrentWeaponRange(GameData &data, int charId) {
    int weaponItemId = GetSelectedWeaponItemId(data, charId);
    if (data.itemData.templateData[GetItemTemplateId(data, weaponItemId)].type != ItemType::Weapon) {
        throw std::runtime_error("Item type must be Weapon");
    }
    int weaponTplId = GetItemTypeTemplateId(data, weaponItemId);
    WeaponTemplate& weaponTemplate = data.weaponData.templateData[weaponTplId];
    return weaponTemplate.range;
}
