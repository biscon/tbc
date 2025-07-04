//
// Created by bison on 13-01-25.
//

#include "SkillRunner.h"
#include "util/Random.h"
#include "raymath.h"
#include "CombatAnimation.h"
#include "audio/SoundEffect.h"
#include "Combat.h"
#include "graphics/Animation.h"
#include "ai/PathFinding.h"

SkillResult ExecuteSkill(GameData& data, Level& level, PlayField& playField) {
    SpriteData& spriteData = data.spriteData;
    CharacterData& charData = data.charData;

    SkillResult result;
    result.success = false;
    result.attack = false;
    result.consumeAction = true;
    result.giveAggro = false;

    Skill* skill = level.selectedSkill;
    int user = level.currentCharacter;
    int target = level.selectedCharacter;

    switch(skill->type) {
        case SkillType::Stun: {
            int success = RandomInRange(0, 100);
            if (success > 50) {
                // Apply stun effect to target
                charData.statusEffects[target].push_back({StatusEffectType::Stun, skill->rank + 1, 0.0f});
                result.success = true;
                result.message = charData.name[user] + " used " + skill->name + " on " + charData.name[target] + " and stunned them!";
                result.attack = true;
                result.consumeAction = true;
                Animation damageNumberAnim{};
                float userX = GetCharacterSpritePosX(spriteData, charData.sprite[user]);
                float userY = GetCharacterSpritePosY(spriteData, charData.sprite[user]);
                SetupDamageNumberAnimation(damageNumberAnim, level.selectedSkill->name, userX, userY - 25, YELLOW, 10);
                level.animations.push_back(damageNumberAnim);
            } else {
                result.message = charData.name[user] + " used " + skill->name + " on " + charData.name[target] + " but it failed!";
                result.attack = true;
                result.consumeAction = true;
            }
            break;
        }
        case SkillType::Taunt: {
            int success = RandomInRange(0, 100);
            if (success > 50) {
                // Apply taunt effect to target
                charData.statusEffects[user].push_back({StatusEffectType::ThreatModifier, skill->rank + 1, 2.0f});
                result.success = true;
                result.message = charData.name[user] + " used " + skill->name;
            } else {
                result.success = false;
                result.message = charData.name[user] + " used " + skill->name + " but it failed!";
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
                result.message = charData.name[user] + " used " + skill->name;
                Vector2 userPos = GetCharacterSpritePos(spriteData, charData.sprite[user]);
                Vector2 targetPos = GetCharacterSpritePos(spriteData, charData.sprite[target]);
                Vector2 dir = CalculateDirection(userPos, targetPos);
                dir.x *= 100;
                dir.y *= 100;
                CreateFireEffect(*playField.particleManager, userPos, dir, 0.2f, 5);
                Vector2i startPos = PixelToGridPositionI((int) userPos.x, (int) userPos.y);
                Vector2i endPos = PixelToGridPositionI((int) targetPos.x, (int) targetPos.y);
                Vector2 gridDir = CalculateDirection(startPos, endPos);
                PlaySoundEffect(SoundEffectType::Burning);

                std::vector<int> targets = GetTargetsInLine(spriteData, charData, level, startPos, gridDir, skill->range + 1, user);
                for(auto &t : targets) {
                    AssignStatusEffect(charData.statusEffects[t], StatusEffectType::Burning, skill->rank + 2, 5.0f);
                    // calculate baseAttack
                    int damage = RandomInRange(10, 20);
                    DealDamage(data, level, user, t, damage);
                    Vector2 tPos = GetCharacterSpritePos(spriteData, charData.sprite[t]);
                    CreateExplosionEffect(*playField.particleManager, tPos, 10, 16.0f, 0.2f);
                    // check if dead
                    if(charData.stats[t].health <= 0) {
                        KillCharacter(spriteData, charData, level, t);
                    } else {
                        PlayDefendAnimation(spriteData, charData, level, user, t);
                    }
                }
            } else {
                result.success = false;
                result.message = charData.name[user] + " used " + skill->name + " but it failed!";
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

void UpdateSkillCooldown(CharacterData& charData, Level &level) {
    for(auto &character : level.turnOrder) {
        DecreaseSkillCooldown(charData.skills[character]);
    }
}
