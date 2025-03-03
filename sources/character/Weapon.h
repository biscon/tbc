//
// Created by bison on 28-01-25.
//

#ifndef SANDBOX_WEAPON_H
#define SANDBOX_WEAPON_H

#include <string>
#include <unordered_map>
#include <vector>

enum class WeaponType {
    Melee,
    Ranged,
};

enum class ScalingStat {
    Attack,
    Speed,
};

/*
struct WeaponTemplate {
    std::string name;
    WeaponType type;
    int baseAttack;
    int range;
    float armorPenetration;  // Percentage of defender’s defense ignored
    float attackMultiplier;
    ScalingStat scalingStat;
    std::string animationTemplate;
    std::string soundEffectType;
};

struct Weapon {
    WeaponTemplate* weaponTemplate;
    int baseAttack;
    int range;
};

struct WeaponManager {
    std::unordered_map<std::string, WeaponTemplate> weaponTemplates;
};
*/

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
    std::unordered_map<std::string, int> weaponTemplates;
};

struct WeaponInstanceData {
    std::vector<std::string> name;
    std::vector<int> weaponTemplateIdx;
};

struct WeaponData {
    WeaponTemplateData templateData;
    WeaponInstanceData instanceData;
};

void InitWeaponTemplateData(WeaponTemplateData& weaponTemplateData, const std::string& filename);
int CreateWeapon(WeaponData& weaponData, const std::string& templateName);
#endif //SANDBOX_WEAPON_H
