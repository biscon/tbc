//
// Created by bison on 27-02-25.
//

#include <cassert>
#include <cmath>
#include "CombatEngine.h"
#include "StatusEffectRunner.h"
#include "CombatAnimation.h"
#include "ai/Ai.h"
#include "util/Random.h"
#include "ui/UI.h"
#include "graphics/Animation.h"
#include "LevelCamera.h"
#include "audio/Sound.h"
#include "game/ActionSystem.h"
#include "LevelSystem.h"
#include "graphics/CharSprite.h"

static bool CheckEndCombat(GameData& data, Level& level) {
    // check victory condition, all enemies have zero health
    bool allEnemiesDefeated = true;
    for (auto &enemy: level.enemyCharacters) {
        if (data.charData.stats[enemy].HP > 0) {
            allEnemiesDefeated = false;
            break;
        }
    }
    if (allEnemiesDefeated) {
        //StopSoundEffect(SoundEffectType::Ambience);
        //PlaySoundEffect(SoundEffectType::Victory, 0.5f);
        //PlayPlayerVictoryAnimation(spriteData, charData, level);
        auto& levelState = data.levelState[level.name];
        levelState.defeatedGroups.insert(level.currentEnemyGroup);
        PushCloseActionBar(data.actionQueue);
        PushEndCombat(data.actionQueue, true);
    }
    // check defeat condition, all players have zero health
    bool allPlayersDefeated = true;
    for (auto &player: level.partyCharacters) {
        if (data.charData.stats[player].HP > 0) {
            allPlayersDefeated = false;
            break;
        }
    }
    if (allPlayersDefeated) {
        //StopSoundEffect(SoundEffectType::Ambience);
        //PlayEnemyVictoryAnimation(data.spriteData, data.charData, level);
        //combat.animations.clear();
        PushCloseActionBar(data.actionQueue);
        PushEndCombat(data.actionQueue, false);
    }
    return allEnemiesDefeated || allPlayersDefeated;
}

void UpdateCombat(GameData &data, Level &level, float dt) {
    SpriteData& spriteData = data.spriteData;
    CharacterData& charData = data.charData;
    switch(level.turnState) {
        case TurnState::StartRound: {
            TraceLog(LOG_INFO, "Start round");
            WaitTurnState(level, TurnState::StartTurn, 1.0f);
            ApplyStatusEffects(data, level);
            PlaySfx(data.soundData, "startRound", false, 0.5f);
            break;
        }
        case TurnState::StartTurn: {
            TraceLog(LOG_INFO, "Start turn");
            StartCameraPanToTargetChar(spriteData, charData, level.camera, level.currentCharacter, 500.0f);

            Animation blinkAnim{};

            // restore action points
            CharacterStats& stats = charData.stats[level.currentCharacter];
            stats.AP = CalculateCharMaxAP(stats);

            TraceLog(LOG_INFO, "Restored action points for %s: %d", charData.name[level.currentCharacter].c_str(), stats.AP);

            level.waitTime = 0.5f;
            if(IsPlayerCharacter(charData, level.currentCharacter)) {
                SetupBlinkAnimation(blinkAnim, level.currentCharacter, 2.0f);
                level.nextState = TurnState::SelectAction;
                data.ui.selectedCharacter = level.currentCharacter;
                PushOpenActionBar(data.actionQueue);
            } else {
                // obtain AiInterface
                AiInterface* ai = GetAiInterface(charData.ai[level.currentCharacter]);
                ai->StartTurn(data, level, data.levelData);
                SetupBlinkAnimation(blinkAnim, level.currentCharacter, 1.0f);
                level.nextState = TurnState::EnemyTurn;
            }
            if(IsIncapacitated(charData, level.currentCharacter)) {
                level.nextState = TurnState::EndTurn;
                std::string logMessage = charData.name[level.currentCharacter] + " is skipping the turn!";
                level.log.push_back(logMessage);
                CharSprite& sprite = charData.sprite[level.currentCharacter];
                float charX = GetCharSpritePosX(spriteData, sprite);
                float charY = GetCharSpritePosY(spriteData, sprite);
                Animation anim{};
                SetupDamageNumberAnimation(anim, "STUNNED", charX, charY-25, WHITE, 10, 0);
                level.animations.push_back(anim);
            }
            level.turnState = TurnState::Waiting;
            level.animations.push_back(blinkAnim);
            break;
        }
        case TurnState::EndTurn: {
            //if(!CheckEndCombat(data, level)) {
            PushCloseActionBar(data.actionQueue);
            NextCharacter(charData, level);
            //}
            break;
        }
        case TurnState::SelectAction: {
            break;
        }
        case TurnState::SelectEnemy: {
            break;
        }
        case TurnState::Attack: {
            TraceLog(LOG_INFO, "Attack");

            level.attackResult = Attack(data, level, level.currentCharacter, level.selectedCharacter, -1);
            FaceCharacter(spriteData, charData, level.currentCharacter, level.selectedCharacter);
            FaceCharacter(spriteData, charData, level.selectedCharacter, level.currentCharacter);
            assert(level.attackResult.defender == level.selectedCharacter);
            assert(level.attackResult.attacker == level.currentCharacter);

            PlayAttackDefendAnimation(spriteData, charData, level, level.currentCharacter, level.selectedCharacter);

            WaitTurnState(level, TurnState::AttackDone, 0.25f);
            break;
        }
        case TurnState::AttackDone: {
            assert(level.attackResult.defender == level.selectedCharacter);
            assert(level.attackResult.attacker == level.currentCharacter);
            float attackerX = GetCharSpritePosX(spriteData, charData.sprite[level.currentCharacter]);
            float attackerY = GetCharSpritePosY(spriteData, charData.sprite[level.currentCharacter]);
            float defenderX = GetCharSpritePosX(spriteData, charData.sprite[level.selectedCharacter]);
            float defenderY = GetCharSpritePosY(spriteData, charData.sprite[level.selectedCharacter]);
            AttackResult& result = level.attackResult;
            AttackHit& hit = result.hits.back();
            float dmgNumDelay = 0.25f;
            float waitTime = 0;
            int damage = hit.damage;
            if(damage > 0) {
                float intensity = (float) GetBloodIntensity(damage, level.attackResult.minDmg, level.attackResult.maxDmg);
                TraceLog(LOG_INFO, "Damage: %d, intensity: %f", damage, intensity);
                Vector2 bloodPos = {defenderX + (float) RandomInRange(-2,2), defenderY - 8 + (float) RandomInRange(-2,2)};
                CreateBloodSplatter(data.particleManager, bloodPos, 10, intensity);
                Animation damageNumberAnim{};
                Color dmgColor = GetDamageColor(damage, level.attackResult.minDmg, level.attackResult.maxDmg);
                SetupDamageNumberAnimation(damageNumberAnim, TextFormat("%d", damage), defenderX, defenderY-25, dmgColor, hit.crit ? 20 : 10, 0);
                level.animations.push_back(damageNumberAnim);
                PlaySfx(data.soundData, "humanPain", false, 0.25f);
                charData.stats[level.attackResult.defender].HP -= damage;
                PlayCharSpriteAnim(data.spriteData, data.charData.sprite[level.attackResult.defender], CharAnimationType::MeleeHit, false);
                waitTime += 0.25f;
            } else {
                Animation damageNumberAnim{};
                SetupDamageNumberAnimation(damageNumberAnim, "MISS", attackerX, attackerY-25, WHITE, 10, 0);
                level.animations.push_back(damageNumberAnim);
                PlaySfx(data.soundData, "meleeMiss");
            }

            if(hit.crit) {
                Animation damageNumberAnim{};
                SetupDamageNumberAnimation(damageNumberAnim, "CRITICAL!!!", attackerX, attackerY-25, WHITE, 10, 0.25f);
                level.animations.push_back(damageNumberAnim);
                PlaySfx(data.soundData, "meleeCrit");
            } else {
                if(damage > 0)
                    PlaySfx(data.soundData, "meleeHit");
            }
            waitTime += dmgNumDelay;
            WaitTurnState(level, TurnState::KillCharacters, waitTime);
            break;
        }
        case TurnState::AttackRanged: {
            TraceLog(LOG_INFO, "AttackRanged");
            int fireMode = 0;
            if(IsPlayerCharacter(data.charData, level.currentCharacter)) {
                fireMode = data.ui.actionBar.selectedModeIdx;
            }
            level.attackResult = Attack(data, level, level.currentCharacter, level.selectedCharacter, fireMode);
            FaceCharacter(spriteData, charData, level.currentCharacter, level.selectedCharacter);
            //FaceCharacter(spriteData, charData, level.selectedCharacter, level.currentCharacter);
            assert(level.attackResult.defender == level.selectedCharacter);
            assert(level.attackResult.attacker == level.currentCharacter);
            PlayShootAnimation(data.spriteData, data.charData, level.currentCharacter);
            //PlayAttackDefendAnimation(spriteData, charData, level, level.currentCharacter, level.selectedCharacter);

            level.waitTime = 0.15f;
            level.nextState = TurnState::AttackRangedDone;
            level.turnState = TurnState::Waiting;
            break;
        }
        case TurnState::AttackRangedDone: {
            assert(level.attackResult.defender == level.selectedCharacter);
            assert(level.attackResult.attacker == level.currentCharacter);
            float defenderX = GetCharSpritePosX(spriteData, charData.sprite[level.selectedCharacter]);
            float defenderY = GetCharSpritePosY(spriteData, charData.sprite[level.selectedCharacter]);
            AttackResult& result = level.attackResult;
            float dmgNumDelay = 0.25f;
            float waitTime = 0;
            bool wasInjured = false;

            for(int i = 0; i < result.hits.size(); i++) {
                PlaySfx(data.soundData, "rifleShot", false, waitTime);
                AttackHit& hit = result.hits[i];
                int damage = hit.damage;
                if(damage > 0) {
                    wasInjured = true;
                    PlayGettingShotAnimation(spriteData, charData, level, level.currentCharacter, level.selectedCharacter, waitTime, 0.20f);
                    float intensity = (float) GetBloodIntensity(damage, level.attackResult.minDmg, level.attackResult.maxDmg);
                    TraceLog(LOG_INFO, "Damage: %d, intensity: %f", damage, intensity);
                    Vector2 bloodPos = {defenderX + (float) RandomInRange(-2,2), defenderY - 8 + (float) RandomInRange(-2,2)};
                    CreateBloodSplatter(data.particleManager, bloodPos, 10, intensity, waitTime);
                    Animation damageNumberAnim{};
                    Color dmgColor = GetDamageColor(damage, level.attackResult.minDmg, level.attackResult.maxDmg);
                    SetupDamageNumberAnimation(damageNumberAnim, TextFormat("%d", damage), defenderX, defenderY-25, dmgColor, hit.crit ? 20 : 10, waitTime);
                    level.animations.push_back(damageNumberAnim);
                } else {
                    Animation damageNumberAnim{};
                    SetupDamageNumberAnimation(damageNumberAnim, "MISS", defenderX, defenderY-25, WHITE, 10, waitTime);
                    level.animations.push_back(damageNumberAnim);
                    //PlaySoundEffect(SoundEffectType::MeleeMiss);
                }

                if(hit.crit) {
                    Animation damageNumberAnim{};
                    waitTime += dmgNumDelay;
                    SetupDamageNumberAnimation(damageNumberAnim, "CRITICAL!!!", defenderX, defenderY-25, WHITE, 10, waitTime);
                    level.animations.push_back(damageNumberAnim);
                    //PlaySoundEffect(SoundEffectType::MeleeCrit);
                } else {

                    //if(damage > 0)
                    //    PlaySoundEffect(SoundEffectType::MeleeHit);
                }
                waitTime += dmgNumDelay;
                charData.stats[level.attackResult.defender].HP -= damage;
            }
            if(wasInjured) {
                PlayCharSpriteAnim(data.spriteData, data.charData.sprite[level.attackResult.defender], CharAnimationType::PistolHit, false);
                PlaySfx(data.soundData, "humanPain", false, waitTime);
            }
            WaitTurnState(level, TurnState::KillCharacters, waitTime);
            break;
        }
        case TurnState::KillCharacters: {
            PlayCharSpriteAnim(spriteData, charData.sprite[level.attackResult.attacker], GetCharIdleAnimType(charData.sprite[level.attackResult.attacker]), true);

            float attackerX = GetCharSpritePosX(spriteData, charData.sprite[level.currentCharacter]);
            float attackerY = GetCharSpritePosY(spriteData, charData.sprite[level.currentCharacter]);
            TurnState nextState = IsPlayerCharacter(data.charData, level.currentCharacter) ? TurnState::SelectEnemy : TurnState::EnemyTurn;
            if(charData.stats[level.attackResult.defender].HP <= 0) {
                Animation speechBubble{};
                SetupSpeechBubbleAnimation(speechBubble, "Haha!", attackerX, attackerY - 25, 1.5f, 0.0f);
                level.animations.push_back(speechBubble);
                RemoveAttackAnimations(level);
                KillCharacter(data, level, level.attackResult.defender);
                WaitTurnState(level, nextState, 0.95f);
            } else {
                PlayCharSpriteAnim(spriteData, charData.sprite[level.attackResult.defender], GetCharIdleAnimType(charData.sprite[level.attackResult.defender]), true);
                WaitTurnState(level, nextState, 0.60f);
            }
            ResetLevelSystem(data.levelData);
            if(IsPlayerCharacter(data.charData, level.currentCharacter)) {
                data.levelData.mode = LevelMode::SelectingEnemyTarget;
            }
            CheckEndCombat(data, level);
            break;
        }
        case TurnState::EnemyTurn: {
            TraceLog(LOG_INFO, "Enemy turn");

            // obtain AiInterface
            AiInterface* ai = GetAiInterface(charData.ai[level.currentCharacter]);
            if(ai != nullptr) {
                HandleTurn(*ai, data, level, data.levelData);
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
            }
            break;
        }
        case TurnState::EndRound: {
            PushCloseActionBar(data.actionQueue);
            Animation textAnim{};
            SetupTextAnimation(textAnim, "Next round!", 150, 1.0f, 1.0f);
            level.animations.push_back(textAnim);
            WaitTurnState(level, TurnState::StartRound, 0.2f);
            UpdateStatusEffects(charData, level);
            break;
        }
    }
}