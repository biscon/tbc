//
// Created by bison on 13-01-25.
//

#ifndef SANDBOX_SKILLS_H
#define SANDBOX_SKILLS_H

#include "Character.h"

struct SkillResult {
    bool success;
    std::string message;
    bool attack;
    bool consumeAction;
    bool giveAggro;
};

SkillResult UseSkill(Skill* skill, Character &user, Character &target);
void DecreaseSkillCooldown(Character &character);

#endif //SANDBOX_SKILLS_H
