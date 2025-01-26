//
// Created by bison on 23-01-25.
//

#include <cstring>
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

void AssignSkill(std::vector<Skill> &skills, SkillType type, const char *name, int rank, bool isPassive, bool noTarget,
                 int cooldown, int maxCooldown, int range) {
    Skill skill{};
    skill.type = type;
    strncpy(skill.name, name, sizeof(skill.name));
    skill.rank = rank;
    skill.isPassive = isPassive;
    skill.noTarget = noTarget;
    skill.cooldown = cooldown;
    skill.maxCooldown = maxCooldown;
    skill.range = range;
    skills.push_back(skill);
}
