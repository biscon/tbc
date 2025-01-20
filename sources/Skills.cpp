//
// Created by bison on 13-01-25.
//

#include "Skills.h"
#include "Random.h"

SkillResult UseSkill(Skill *skill, Character &user, Character &target) {
    SkillResult result;
    result.success = false;
    result.attack = false;
    result.consumeAction = true;
    result.giveAggro = false;

    switch(skill->type) {
        case SkillType::Stun: {
            int success = RandomInRange(0, 100);
            if (success > 50) {
                // Apply stun effect to target
                target.statusEffects.push_back({StatusEffectType::Stun, skill->rank + 1, 0.0f});
                result.success = true;
                result.message = user.name + " used " + skill->name + " on " + target.name + " and stunned them!";
                result.attack = true;
                result.consumeAction = true;
            } else {
                result.message = user.name + " used " + skill->name + " on " + target.name + " but it failed!";
                result.attack = true;
                result.consumeAction = true;
            }
            break;
        }
        case SkillType::Taunt: {
            int success = RandomInRange(0, 100);
            if (success > 50) {
                // Apply taunt effect to target
                user.statusEffects.push_back({StatusEffectType::ThreatModifier, skill->rank + 1, 2.0f});
                result.success = true;
                result.message = user.name + " used " + skill->name;
            } else {
                result.success = false;
                result.message = user.name + " used " + skill->name + " but it failed!";
            }
            result.attack = false;
            result.giveAggro = true;
            result.consumeAction = false;
            break;
        }
    }
    skill->cooldown = skill->maxCooldown;
    return result;
}

void DecreaseSkillCooldown(Character &character) {
    for (Skill &skill : character.skills) {
        if (skill.cooldown > 0) {
            skill.cooldown--;
        }
    }
}
