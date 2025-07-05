//
// Created by bison on 04-07-25.
//

#include <unordered_map>
#include "SkillData.h"

const std::vector<std::string> skillIds = {
        "Melee",
        "Small Guns",
        "Big Guns",
        "Energy Weapons",
        "Explosives",
        "Lockpicking",
        "Science",
        "Speech",
        "Medicine",
        "Repair"
};

std::unordered_map<std::string, Skill> skillIdToEnum = {
        { "Melee", Skill::Melee },
        { "Small Guns", Skill::SmallGuns },
        { "Big Guns", Skill::BigGuns },
        { "Energy Weapons", Skill::EnergyWeapons },
        { "Explosives", Skill::Explosives },
        { "Lockpicking", Skill::LockPicking },
        { "Science", Skill::Science },
        { "Speech", Skill::Speech },
        { "Medicine", Skill::Medicine },
        { "Repair", Skill::Repair },
};

Skill SkillIdToEnum(const std::string& id) {
    return skillIdToEnum.at(id);
}

std::string EnumToSkillId(Skill skill) {
    return skillIds.at(static_cast<size_t>(skill));
}
