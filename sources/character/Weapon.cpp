//
// Created by bison on 28-01-25.
//

#include <fstream>
#include "Weapon.h"
#include "../util/json.hpp"
#include "raylib.h"
#include <vector>
#include <string>

using json = nlohmann::json;

// Helper functions for WeaponType
inline std::string WeaponTypeToString(WeaponType type) {
    switch (type) {
        case WeaponType::Melee: return "Melee";
        case WeaponType::Ranged: return "Ranged";
        default: return "Unknown";
    }
}

inline WeaponType WeaponTypeFromString(const std::string& str) {
    if (str == "Melee") return WeaponType::Melee;
    if (str == "Ranged") return WeaponType::Ranged;
    throw std::runtime_error("Unknown WeaponType: " + str);
}


void InitWeaponData(WeaponData& weaponData, const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) throw std::runtime_error("Could not open file: " + filename);

    json j;
    file >> j;

    for (auto& [id, value] : j.items()) {
        WeaponTemplate tmpl;
        tmpl.name = value.at("name");
        tmpl.baseDamage = value.at("baseDamage");
        tmpl.critChance = value.at("critChance");
        tmpl.critMultiplier = value.at("critMultiplier");
        tmpl.weaponAccuracy = value.at("weaponAccuracy");
        tmpl.range = value.at("range");
        tmpl.apCost = value.at("apCost");
        tmpl.skillUsed = value.at("skillUsed");
        tmpl.type = WeaponTypeFromString(value.at("type"));
        tmpl.animationTemplate = value.at("animationTemplate");

        if (tmpl.type == WeaponType::Ranged) {
            WeaponRanged ranged;
            ranged.ammoType = value.at("rangedData").at("ammoType");
            ranged.magazineSize = value.at("rangedData").at("magazineSize");
            ranged.currentAmmo = value.at("rangedData").at("currentAmmo");

            for (auto& mode : value.at("rangedData").at("fireModes")) {
                FireMode fm;
                fm.name = mode.at("name");
                if(mode.contains("icon")) {
                    fm.icon = mode.at("icon");
                } else {
                    fm.icon = -1;
                }
                fm.apCost = mode.at("apCost");
                fm.roundsFired = mode.at("roundsFired");
                fm.accuracyMod = mode.at("accuracyMod");
                fm.critChance = mode.at("critChance");
                fm.critMultiplier = mode.at("critMultiplier");
                ranged.fireModes.push_back(fm);
            }

            tmpl.rangeDataId = static_cast<int>(weaponData.rangedData.size());
            weaponData.rangedData.push_back(std::move(ranged));
        } else {
            tmpl.rangeDataId = -1;
        }

        int index = static_cast<int>(weaponData.templateData.size());
        weaponData.templateData.push_back(tmpl);
        weaponData.templateIdToIndex[id] = index;
        weaponData.indexToTemplateId.push_back(id);
    }
}

