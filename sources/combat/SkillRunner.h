//
// Created by bison on 13-01-25.
//

#ifndef SANDBOX_SKILLRUNNER_H
#define SANDBOX_SKILLRUNNER_H

#include "character/Character.h"
#include "combat/CombatState.h"
#include "ui/Grid.h"

struct SkillResult {
    bool success;
    std::string message;
    bool attack;
    bool consumeAction;
    bool giveAggro;
};

SkillResult ExecuteSkill(CombatState& combat, GridState& gridState);
void UpdateSkillCooldown(CombatState &combat);

#endif //SANDBOX_SKILLRUNNER_H
