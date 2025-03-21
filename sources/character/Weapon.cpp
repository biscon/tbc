//
// Created by bison on 28-01-25.
//

#include <fstream>
#include "Weapon.h"
#include "../util/json.hpp"
#include "raylib.h"

using json = nlohmann::json;

void InitWeaponTemplateData(WeaponTemplateData& weaponTemplateData, const std::string& filename) {
    std::ifstream file(filename);
    json j;
    file >> j;
    for(auto& weaponJson : j) {
        std::string name = weaponJson["name"].get<std::string>();
        weaponTemplateData.name.emplace_back(name);
        WeaponType type = weaponJson["type"].get<std::string>() == "Melee" ? WeaponType::Melee : WeaponType::Ranged;
        weaponTemplateData.type.emplace_back(type);
        WeaponStats stats{};
        stats.baseAttack = weaponJson["baseAttack"].get<int>();
        stats.range = weaponJson["range"].get<int>();
        stats.armorPenetration = weaponJson["armorPenetration"].get<float>();
        stats.attackMultiplier = weaponJson["attackMultiplier"].get<float>();
        stats.scalingStat = weaponJson["scalingStat"].get<std::string>() == "Attack" ? ScalingStat::Attack : ScalingStat::Speed;
        weaponTemplateData.stats.emplace_back(stats);

        std::string animationTemplate = weaponJson["animationTemplate"].get<std::string>();
        weaponTemplateData.animationTemplate.emplace_back(animationTemplate);
        std::string soundEffectType = weaponJson["soundEffectType"].get<std::string>();
        weaponTemplateData.soundEffectType.emplace_back(soundEffectType);

        weaponTemplateData.weaponTemplates[name] = (int) weaponTemplateData.name.size()-1;
        TraceLog(LOG_INFO, "Loaded weapon: %s", name.c_str());
    }
}

int CreateWeapon(WeaponData& weaponData, const std::string &templateName) {
    auto it = weaponData.templateData.weaponTemplates.find(templateName);
    if(it != weaponData.templateData.weaponTemplates.end()) {
        int templateIdx = it->second;
        weaponData.instanceData.name.emplace_back(weaponData.templateData.name[templateIdx]);
        weaponData.instanceData.weaponTemplateIdx.emplace_back(templateIdx);
        return (int) weaponData.instanceData.name.size()-1;
    } else {
        TraceLog(LOG_ERROR, "CreateWeapon: Weapon template not found: %s", templateName.c_str());
        std::abort();
    }
    return -1;
}
