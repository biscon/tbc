//
// Created by bison on 23-01-25.
//


#include "Skill.h"

void SetInitialSkillValues(GameData &data, int charId) {
    CharacterStats& stats = data.charData.stats[charId];
    auto& values = data.charData.skillValues[charId];

    values[static_cast<int>(Skill::Melee)] = 20 + (2 * (stats.REF + stats.STR));
    values[static_cast<int>(Skill::SmallGuns)] = 5 + (4 * stats.REF);
    values[static_cast<int>(Skill::BigGuns)] = 2 * stats.REF;
    values[static_cast<int>(Skill::EnergyWeapons)] = 2 * stats.REF;
    values[static_cast<int>(Skill::Explosives)] = 10 + (stats.PER + stats.REF);
    values[static_cast<int>(Skill::LockPicking)] = 10 + (stats.PER + stats.REF);
    values[static_cast<int>(Skill::Science)] = 4 * stats.INT;
    values[static_cast<int>(Skill::Speech)] = 5 * stats.CHA;
    values[static_cast<int>(Skill::Medicine)] = 5 + (stats.PER + stats.INT);
    values[static_cast<int>(Skill::Repair)] = 3 * stats.INT;

}
