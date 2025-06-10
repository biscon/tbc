//
// Created by bison on 27-02-25.
//

#include <cassert>
#include <cmath>
#include "CombatEngine.h"
#include "StatusEffectRunner.h"
#include "SkillRunner.h"
#include "audio/SoundEffect.h"
#include "CombatAnimation.h"
#include "ai/Ai.h"
#include "util/Random.h"
#include "ui/UI.h"
#include "graphics/Animation.h"

void UpdateCombat(GameData &data, Level &level, PlayField& playField, float dt) {
    SpriteData& spriteData = data.spriteData;
    CharacterData& charData = data.charData;
    WeaponData& weaponData = data.weaponData;
    switch(level.turnState) {
        case TurnState::StartRound: {
            TraceLog(LOG_INFO, "Start round");
            WaitTurnState(level, TurnState::StartTurn, 1.0f);
            ApplyStatusEffects(spriteData, charData, weaponData, level, playField);
            PlaySoundEffect(SoundEffectType::StartRound);
            break;
        }
        case TurnState::StartTurn: {
            TraceLog(LOG_INFO, "Start turn");
            StartCameraPanToTargetChar(spriteData, charData, level.camera, level.currentCharacter, 500.0f);

            Animation blinkAnim{};

            // restore movepoints
            CharacterStats& stats = charData.stats[level.currentCharacter];
            int movePoints = (int) (5 + sqrt(stats.speed) * 2);
            stats.movePoints = movePoints;
            TraceLog(LOG_INFO, "Restored move points for %s: %d", charData.name[level.currentCharacter].c_str(), movePoints);

            level.waitTime = 0.5f;
            if(IsPlayerCharacter(charData, level.currentCharacter)) {
                SetupBlinkAnimation(blinkAnim, level.currentCharacter, 2.0f);
                level.nextState = TurnState::SelectAction;
            } else {
                SetupBlinkAnimation(blinkAnim, level.currentCharacter, 1.0f);
                level.nextState = TurnState::EnemyTurn;
            }
            if(IsIncapacitated(charData, level.currentCharacter)) {
                level.nextState = TurnState::EndTurn;
                std::string logMessage = charData.name[level.currentCharacter] + " is skipping the turn!";
                level.log.push_back(logMessage);
                CharacterSprite& sprite = charData.sprite[level.currentCharacter];
                float charX = GetCharacterSpritePosX(spriteData, sprite);
                float charY = GetCharacterSpritePosY(spriteData, sprite);
                Animation anim{};
                SetupDamageNumberAnimation(anim, "STUNNED", charX, charY-25, WHITE, 10);
                level.animations.push_back(anim);
            }
            level.turnState = TurnState::Waiting;
            level.animations.push_back(blinkAnim);
            break;
        }
        case TurnState::EndTurn: {
            NextCharacter(charData, level);
            break;
        }
        case TurnState::SelectAction: {
            break;
        }
        case TurnState::SelectEnemy: {
            break;
        }
        case TurnState::UseSkill: {
            TraceLog(LOG_INFO, "Use skill");
            CharacterSprite& sprite = charData.sprite[level.currentCharacter];
            float attackerX = GetCharacterSpritePosX(spriteData, sprite);
            float attackerY = GetCharacterSpritePosY(spriteData, sprite);

            SkillResult result = ExecuteSkill(spriteData, charData, weaponData, level, playField);
            Animation damageNumberAnim{};
            if(!level.selectedSkill->noTarget) {
                if(!result.success) {
                    SetupDamageNumberAnimation(damageNumberAnim, "FAILED", attackerX, attackerY-25, WHITE, 10);
                }
            } else {
                if(result.success) {
                    SetupDamageNumberAnimation(damageNumberAnim, level.selectedSkill->name, attackerX, attackerY - 25, YELLOW, 10);
                } else {
                    SetupDamageNumberAnimation(damageNumberAnim, "FAILED", attackerX, attackerY-25, WHITE, 10);
                }
            }
            level.log.push_back(result.message);
            level.animations.push_back(damageNumberAnim);

            WaitTurnState(level, TurnState::EndTurn, 1.0f);


            if(result.attack) {
                level.nextState = TurnState::Attack;
            } else if(!result.consumeAction) {
                level.nextState = TurnState::SelectAction;
            } else {
                level.nextState = TurnState::EndTurn;
            }
            if(result.giveAggro) {
                SetTaunt(charData, level, level.currentCharacter);
            }
            level.selectedSkill = nullptr;
            break;
        }
        case TurnState::Attack: {
            TraceLog(LOG_INFO, "Attack");

            level.attackResult = Attack(charData, weaponData, level, level.currentCharacter, level.selectedCharacter);
            FaceCharacter(spriteData, charData, level.currentCharacter, level.selectedCharacter);
            FaceCharacter(spriteData, charData, level.selectedCharacter, level.currentCharacter);
            assert(level.attackResult.defender == level.selectedCharacter);
            assert(level.attackResult.attacker == level.currentCharacter);

            PlayAttackDefendAnimation(spriteData, charData, level, level.currentCharacter, level.selectedCharacter);

            level.waitTime = 0.25f;
            level.nextState = TurnState::AttackDone;
            level.turnState = TurnState::Waiting;
            break;
        }
        case TurnState::AttackDone: {
            assert(level.attackResult.defender == level.selectedCharacter);
            assert(level.attackResult.attacker == level.currentCharacter);
            float attackerX = GetCharacterSpritePosX(spriteData, charData.sprite[level.currentCharacter]);
            float attackerY = GetCharacterSpritePosY(spriteData, charData.sprite[level.currentCharacter]);
            float defenderX = GetCharacterSpritePosX(spriteData, charData.sprite[level.selectedCharacter]);
            float defenderY = GetCharacterSpritePosY(spriteData, charData.sprite[level.selectedCharacter]);
            int damage = level.attackResult.damage;
            if(damage > 0) {
                float intensity = (float) GetBloodIntensity(damage, GetAttack(charData, weaponData, level.currentCharacter));
                TraceLog(LOG_INFO, "Damage: %d, intensity: %f", damage, intensity);
                Vector2 bloodPos = {defenderX + (float) RandomInRange(-2,2), defenderY - 8 + (float) RandomInRange(-2,2)};
                CreateBloodSplatter(*playField.particleManager, bloodPos, 10, intensity);
                Animation damageNumberAnim{};
                Color dmgColor = GetDamageColor(damage, GetAttack(charData, weaponData, level.currentCharacter));
                SetupDamageNumberAnimation(damageNumberAnim, TextFormat("%d", damage), defenderX, defenderY-25, dmgColor, level.attackResult.crit ? 20 : 10);
                level.animations.push_back(damageNumberAnim);
                PlaySoundEffect(SoundEffectType::HumanPain, 0.25f);
            } else {
                Animation damageNumberAnim{};
                SetupDamageNumberAnimation(damageNumberAnim, "MISS", attackerX, attackerY-25, WHITE, 10);
                level.animations.push_back(damageNumberAnim);
                PlaySoundEffect(SoundEffectType::MeleeMiss);
            }

            if(level.attackResult.crit) {
                Animation damageNumberAnim{};
                SetupDamageNumberAnimation(damageNumberAnim, "CRITICAL!!!", attackerX, attackerY, WHITE, 10);
                level.animations.push_back(damageNumberAnim);
                PlaySoundEffect(SoundEffectType::MeleeCrit);
            } else {
                if(damage > 0)
                    PlaySoundEffect(SoundEffectType::MeleeHit);
            }

            charData.stats[level.attackResult.defender].health -= damage;
            if(charData.stats[level.attackResult.defender].health <= 0) {
                Animation speechBubble{};
                SetupSpeechBubbleAnimation(speechBubble, "Haha!", attackerX, attackerY - 25, 1.5f, 0.0f);
                level.animations.push_back(speechBubble);
                RemoveAttackAnimations(level);
                KillCharacter(spriteData, charData, level, level.attackResult.defender);
                WaitTurnState(level, TurnState::EndTurn, 0.95f);
            } else {
                WaitTurnState(level, TurnState::EndTurn, 0.60f);
            }
            break;
        }
        case TurnState::EnemyTurn: {
            TraceLog(LOG_INFO, "Enemy turn");

            // obtain AiInterface
            AiInterface* ai = GetAiInterface(charData.ai[level.currentCharacter]);
            if(ai != nullptr) {
                HandleTurn(*ai, spriteData, charData, level, playField);
            } else {
                TraceLog(LOG_WARNING, "No AI interface found for %s", charData.ai[level.currentCharacter].c_str());
                level.turnState = TurnState::EndTurn;
            }
            break;
        }
        case TurnState::Waiting: {
            level.waitTime -= dt;
            if (level.waitTime <= 0) {
                level.turnState = level.nextState;
                TraceLog(LOG_INFO, "Waiting done");
            }
            break;
        }
        case TurnState::EndRound: {
            Animation textAnim{};
            SetupTextAnimation(textAnim, "Next round!", 125, 1.0f, 1.0f);
            level.animations.push_back(textAnim);
            WaitTurnState(level, TurnState::StartRound, 0.2f);
            UpdateStatusEffects(charData, level);
            DecayThreat(charData, level, 10);
            UpdateSkillCooldown(charData, level);
            break;
        }
    }

    if(level.turnState != TurnState::None) {
        // check victory condition, all enemies have zero health
        bool allEnemiesDefeated = true;
        for (auto &enemy: level.enemyCharacters) {
            if (charData.stats[enemy].health > 0) {
                allEnemiesDefeated = false;
                break;
            }
        }
        if (allEnemiesDefeated && level.turnState != TurnState::Victory) {
            level.turnState = TurnState::Victory;
            //StopSoundEffect(SoundEffectType::Ambience);
            PlaySoundEffect(SoundEffectType::Victory, 0.5f);
            PlayPlayerVictoryAnimation(spriteData, charData, level);
            auto& levelState = data.levelState[level.name];
            levelState.defeatedGroups.insert(level.currentEnemyGroup);
        }
        // check defeat condition, all players have zero health
        bool allPlayersDefeated = true;
        for (auto &player: level.partyCharacters) {
            if (charData.stats[player].health > 0) {
                allPlayersDefeated = false;
                break;
            }
        }
        if (allPlayersDefeated && level.turnState != TurnState::Defeat) {
            level.turnState = TurnState::Defeat;
            //StopSoundEffect(SoundEffectType::Ambience);
            PlaySoundEffect(SoundEffectType::Defeat, 0.5f);
            PlayEnemyVictoryAnimation(spriteData, charData, level);
            //combat.animations.clear();
        }
    }
}