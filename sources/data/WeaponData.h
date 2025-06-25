//
// Created by bison on 13-03-25.
//

#ifndef SANDBOX_WEAPONDATA_H
#define SANDBOX_WEAPONDATA_H

#include <vector>
#include <string>
#include <unordered_map>

enum class WeaponType {
    Melee,
    Ranged,
};

enum class ScalingStat {
    Attack,
    Speed,
};

struct WeaponStats {
    int baseAttack;
    int range;
    float armorPenetration;  // Percentage of defender’s defense ignored
    float attackMultiplier;
    ScalingStat scalingStat;
};

struct WeaponTemplateData {
    std::vector<std::string> name;
    std::vector<WeaponType> type;
    std::vector<WeaponStats> stats;
    std::vector<std::string> animationTemplate;
    std::vector<std::string> soundEffectType;
};

struct WeaponData {
    WeaponTemplateData templateData;
    std::unordered_map<std::string, int> templateIdToIndex;
    std::vector<std::string> indexToTemplateId;
};

#endif //SANDBOX_WEAPONDATA_H
