//
// Created by bison on 13-01-25.
//

#ifndef SANDBOX_SKILLRUNNER_H
#define SANDBOX_SKILLRUNNER_H

#include "character/Character.h"
#include "PlayField.h"

struct SkillResult {
    bool success;
    std::string message;
    bool attack;
    bool consumeAction;
    bool giveAggro;
};

SkillResult ExecuteSkill(Level& level, PlayField& playField);
void UpdateSkillCooldown(Level &level);

#endif //SANDBOX_SKILLRUNNER_H
