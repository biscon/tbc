//
// Created by bison on 23-01-25.
//

#ifndef SANDBOX_SKILL_H
#define SANDBOX_SKILL_H

#include <vector>

enum class SkillType {
    Dodge,
    Stun,
    Taunt,
    FlameJet,
    Bleed
};

// Skill struct to represent individual skills like Dodge
struct Skill {
    SkillType type;
    char name[64];
    int rank;  // Rank 1, Rank 2, Rank 3
    bool isPassive;  // Passive skills are always active
    bool noTarget;  // Skills that don't require a target
    int cooldown; // How many rounds before the skill can be used again
    int maxCooldown; // The maximum cooldown for the skill
    int range;  // How far the skill can reach
};

Skill* GetSkill(std::vector<Skill> &skills, SkillType type);
int GetSkillRank(std::vector<Skill> &skills, SkillType type);
std::vector<Skill*> GetActiveSkills(std::vector<Skill> &skills);
void DecreaseSkillCooldown(std::vector<Skill> &skills);

#endif //SANDBOX_SKILL_H
