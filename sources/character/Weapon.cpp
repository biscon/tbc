//
// Created by bison on 28-01-25.
//

#include <fstream>
#include "Weapon.h"
#include "../util/json.hpp"
#include "raylib.h"

using json = nlohmann::json;

static WeaponManager manager;

void InitWeaponManager(const std::string& filename) {
    std::ifstream file(filename);
    json j;
    file >> j;
    WeaponTemplate weaponTemplate;
    for(auto& weaponJson : j) {
        weaponTemplate.name = weaponJson["name"].get<std::string>();
        weaponTemplate.type = weaponJson["type"].get<std::string>() == "Melee" ? WeaponType::Melee : WeaponType::Ranged;
        weaponTemplate.baseAttack = weaponJson["baseAttack"].get<int>();
        weaponTemplate.range = weaponJson["range"].get<int>();
        weaponTemplate.armorPenetration = weaponJson["armorPenetration"].get<float>();
        weaponTemplate.attackMultiplier = weaponJson["attackMultiplier"].get<float>();
        weaponTemplate.scalingStat = weaponJson["scalingStat"].get<std::string>() == "Attack" ? ScalingStat::Attack : ScalingStat::Speed;
        weaponTemplate.animationTemplate = weaponJson["animationTemplate"].get<std::string>();
        weaponTemplate.soundEffectType = weaponJson["soundEffectType"].get<std::string>();
        manager.weaponTemplates[weaponTemplate.name] = weaponTemplate;
        TraceLog(LOG_INFO, "Loaded weapon: %s", weaponTemplate.name.c_str());
    }
}

void DestroyWeaponManager() {
    manager.weaponTemplates.clear();
}

void CreateWeapon(Weapon &weapon, const std::string &name) {
    auto it = manager.weaponTemplates.find(name);
    if(it != manager.weaponTemplates.end()) {
        weapon.weaponTemplate = &it->second;
        weapon.baseAttack = it->second.baseAttack;
        weapon.range = it->second.range;
    } else {
        TraceLog(LOG_ERROR, "CreateWeapon: Weapon template not found: %s", name.c_str());
    }
}
