//
// Created by bison on 13-03-25.
//

#ifndef SANDBOX_SKILLDATA_H
#define SANDBOX_SKILLDATA_H

#include <vector>
#include <string>

enum class Skill {
    Melee,
    SmallGuns,
    BigGuns,
    EnergyWeapons,
    Explosives,
    LockPicking,
    Science,
    Speech,
    Medicine,
    Repair,
    // ...
    Count
};

Skill SkillIdToEnum(const std::string& id);
std::string EnumToSkillId(Skill skill);

#endif //SANDBOX_SKILLDATA_H
