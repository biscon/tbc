//
// Created by bison on 23-01-25.
//

#ifndef SANDBOX_SKILL_H
#define SANDBOX_SKILL_H

#include "data/SkillData.h"

Skill* GetSkill(std::vector<Skill> &skills, SkillType type);
int GetSkillRank(std::vector<Skill> &skills, SkillType type);
std::vector<Skill*> GetActiveSkills(std::vector<Skill> &skills);
void DecreaseSkillCooldown(std::vector<Skill> &skills);
void AssignSkill(std::vector<Skill> &skills, SkillType type, const char* name, int rank, bool isPassive, bool noTarget, int cooldown, int maxCooldown, int range);

#endif //SANDBOX_SKILL_H
