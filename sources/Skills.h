//
// Created by bison on 13-01-25.
//

#ifndef SANDBOX_SKILLS_H
#define SANDBOX_SKILLS_H

#include "Character.h"
#include "CombatState.h"
#include "Grid.h"

struct SkillResult {
    bool success;
    std::string message;
    bool attack;
    bool consumeAction;
    bool giveAggro;
};

SkillResult UseSkill(CombatState& combat, GridState& gridState);

#endif //SANDBOX_SKILLS_H
