//
// Created by bison on 23-01-25.
//

#include "Skill.h"

// Helper function to get the rank of a skill, returns -1 if the skill is not found
int GetSkillRank(std::vector<Skill> &skills, SkillType type) {
    for (const Skill &skill : skills) {
        if (skill.type == type) {
            return skill.rank;
        }
    }
    return -1;  // Return -1 if skill not found
}

Skill* GetSkill(std::vector<Skill> &skills, SkillType type) {
    for (Skill &skill : skills) {
        if (skill.type == type) {
            return &skill;
        }
    }
    return nullptr;  // Return -1 if skill not found
}

std::vector<Skill*> GetActiveSkills(std::vector<Skill> &skills) {
    std::vector<Skill*> activeSkills;
    for (Skill &skill : skills) {
        if (!skill.isPassive) {
            activeSkills.push_back(&skill);
        }
    }
    return activeSkills;
}

void DecreaseSkillCooldown(std::vector<Skill> &skills) {
    for (Skill &skill : skills) {
        if (skill.cooldown > 0) {
            skill.cooldown--;
        }
    }
}