//
// Created by bison on 13-01-25.
//

#include "SkillRunner.h"
#include "util/Random.h"
#include "raymath.h"
#include "CombatAnimation.h"
#include "audio/SoundEffect.h"

SkillResult ExecuteSkill(CombatState& combat, GridState& gridState) {
    SkillResult result;
    result.success = false;
    result.attack = false;
    result.consumeAction = true;
    result.giveAggro = false;

    Skill* skill = combat.selectedSkill;
    Character &user = *combat.currentCharacter;
    Character &target = *combat.selectedCharacter;

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
                Animation damageNumberAnim{};
                float userX = user.sprite.player.position.x;
                float userY = user.sprite.player.position.y;
                SetupDamageNumberAnimation(damageNumberAnim, combat.selectedSkill->name, userX, userY - 25, YELLOW, 10);
                combat.animations.push_back(damageNumberAnim);
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
        case SkillType::FlameJet: {
            int success = RandomInRange(0, 100);
            if (success > 5) {
                // Apply taunt effect to target
                //user.statusEffects.push_back({StatusEffectType::ThreatModifier, skill->rank + 1, 2.0f});
                result.success = true;
                result.message = user.name + " used " + skill->name;
                Vector2 dir = CalculateDirection(user.sprite.player.position, target.sprite.player.position);
                dir.x *= 100;
                dir.y *= 100;
                CreateFireEffect(*gridState.particleManager, user.sprite.player.position, dir, 0.2f, 5);
                Vector2i startPos = PixelToGridPositionI(user.sprite.player.position.x, user.sprite.player.position.y);
                Vector2i endPos = PixelToGridPositionI(target.sprite.player.position.x, target.sprite.player.position.y);
                Vector2 gridDir = CalculateDirection(startPos, endPos);
                PlaySoundEffect(SoundEffectType::Burning);

                std::vector<Character*> targets = GetTargetsInLine(combat, startPos, gridDir, skill->range+1, &user);
                for(auto &t : targets) {
                    AssignStatusEffect(t->statusEffects, StatusEffectType::Burning, skill->rank + 2, 5.0f);
                    // calculate damage
                    int damage = RandomInRange(10, 20);
                    DealDamage(combat, user, *t, damage);
                    CreateExplosionEffect(*gridState.particleManager, {t->sprite.player.position.x, t->sprite.player.position.y}, 10, 16.0f, 0.2f);
                    // check if dead
                    if(t->health <= 0) {
                        KillCharacter(combat, *t);
                    } else {
                        PlayDefendAnimation(combat, user, *t);
                    }
                }
            } else {
                result.success = false;
                result.message = user.name + " used " + skill->name + " but it failed!";
            }
            result.attack = false;
            result.giveAggro = false;
            result.consumeAction = true;
            break;
        }
    }
    skill->cooldown = skill->maxCooldown;
    return result;
}

void UpdateSkillCooldown(CombatState &combat) {
    for(auto &character : combat.turnOrder) {
        DecreaseSkillCooldown(character->skills);
    }
}
