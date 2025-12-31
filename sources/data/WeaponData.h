//
// Created by bison on 13-03-25.
//

#ifndef SANDBOX_WEAPONDATA_H
#define SANDBOX_WEAPONDATA_H

#include <vector>
#include <string>
#include <unordered_map>
#include <stdexcept>

enum class WeaponType {
    Melee,
    Ranged,
};

enum class WeaponAnimType {
    Sword,
    Pistol,
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
    std::vector<FireMode> fireModes;
};

struct WeaponTemplate {
    std::string name;
    int minDamage;
    int maxDamage;
    float critChance;
    float critMultiplier;
    float weaponAccuracy; // +/- % hit bonus
    int range;            // Tiles (1 = melee)
    std::string skillUsed;
    WeaponType type;
    int rangeDataId;
    WeaponAnimType animType;
    int apCost;
};

struct WeaponInstance {
    int templateId = 0;
    int currentAmmo = 0;
    bool jammed = false;
};

struct WeaponData {
    std::vector<WeaponTemplate> templateData;
    std::unordered_map<std::string, int> templateIdToIndex;
    std::vector<std::string> indexToTemplateId;

    std::vector<WeaponInstance> instanceData;

    // additional template data for ranged weapon
    std::vector<WeaponRanged> rangedData;
};

// Helper functions for WeaponType
inline std::string WeaponAnimTypeToString(WeaponAnimType type) {
    switch (type) {
        case WeaponAnimType::Sword: return "Sword";
        case WeaponAnimType::Pistol: return "Pistol";
        default: return "Unknown";
    }
}

inline WeaponAnimType WeaponAnimTypeFromString(const std::string& str) {
    if (str == "Sword") return WeaponAnimType::Sword;
    if (str == "Pistol") return WeaponAnimType::Pistol;
    throw std::runtime_error("Unknown WeaponAnimType: " + str);
}


#endif //SANDBOX_WEAPONDATA_H
