//
// Created by bison on 28-01-25.
//

#ifndef SANDBOX_WEAPON_H
#define SANDBOX_WEAPON_H

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

void InitWeaponManager(const std::string& filename);
void DestroyWeaponManager();
void CreateWeapon(Weapon &weapon, const std::string& name);
#endif //SANDBOX_WEAPON_H
