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

struct FireMode {
    std::string name;
    int icon = -1;
    int apCost;
    int roundsFired; // -1 = dump mag
    float accuracyMod; // Penalty/bonus for this mode
    float critChance;
    float critMultiplier;
};

struct WeaponRanged {
    std::string ammoType;
    int magazineSize;
    int currentAmmo;
    std::vector<FireMode> fireModes;
};

struct WeaponTemplate {
    std::string name;
    int baseDamage;
    float critChance;
    float critMultiplier;
    float weaponAccuracy; // +/- % hit bonus
    int range;            // Tiles (1 = melee)
    std::string skillUsed;
    WeaponType type;
    int rangeDataId;
    std::string animationTemplate;
};

struct WeaponData {
    std::vector<WeaponTemplate> templateData;
    std::unordered_map<std::string, int> templateIdToIndex;
    std::vector<std::string> indexToTemplateId;
    // additional template data for ranged weapon
    std::vector<WeaponRanged> rangedData;
};

#endif //SANDBOX_WEAPONDATA_H
