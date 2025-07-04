//
// Created by bison on 04-07-25.
//

#include <unordered_map>
#include "SkillData.h"

const std::vector<std::string> skillIds = {
        "Melee",
        "SmallGuns",
        "EnergyWeapons",
        "Explosives",
        "Lockpicking",
        "Hacking",
        "Speech",
        "Medicine",
};

std::unordered_map<std::string, Skill> skillIdToEnum = {
        { "Melee", Skill::Melee },
        { "SmallGuns", Skill::SmallGuns },
        { "EnergyWeapons", Skill::EnergyWeapons },
        { "Explosives", Skill::Explosives },
        { "Lockpicking", Skill::LockPicking },
        { "Hacking", Skill::Hacking },
        { "Speech", Skill::Speech },
        { "Medicine", Skill::Medicine },
};

Skill SkillIdToEnum(const std::string& id) {
    return skillIdToEnum.at(id);
}

std::string EnumToSkillId(Skill skill) {
    return skillIds.at(static_cast<size_t>(skill));
}
