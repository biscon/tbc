//
// Created by bison on 10-01-25.
//

#include <algorithm>
#include <cstring>
#include "LevelScreen.h"
#include "raylib.h"

#define RAYGUI_IMPLEMENTATION

#include "raygui.h"
#include "character/Character.h"
#include "PlayField.h"
#include "ui/UI.h"
#include "level/SkillRunner.h"
#include "ai/Ai.h"
#include "raymath.h"
#include "util/Random.h"
#include "level/StatusEffectRunner.h"
#include "level/CombatAnimation.h"
#include "audio/SoundEffect.h"
#include <cassert>

void CreateLevelScreen(LevelScreen &levelScreen, GameEventQueue* eventQueue) {
    levelScreen.actionIconScrollIndex = 0;
    levelScreen.showActionBarTitle = true;
    levelScreen.eventQueue = eventQueue;
}

void DestroyLevelScreen(LevelScreen &levelScreen) {

}

static bool IsCharacterVisible(Level &combat, Character *character) {
    // Check if the character is visible (not blinking)
    for (auto &animation: combat.animations) {
        if (animation.type == AnimationType::Blink) {
            if (animation.state.blink.character == character) {
                if (!animation.state.blink.visible) {
                    return false;
                }
            }
        }
    }
    return true;
}

static bool DrawActionIcon(float x, float y, ActionIcon &actionIcon, LevelScreen &uiState) {
    Vector2 mousePos = GetMousePosition();
    Rectangle iconRect = {x, y, 32, 32};

    if (CheckCollisionPointRec(mousePos, iconRect) && !actionIcon.disabled) {
        uiState.showActionBarTitle = false;
        DrawText(actionIcon.description, 240 - (MeasureText(actionIcon.description, 10) / 2), 220, 10, WHITE);
        DrawRectangleRec(iconRect, GRAY);
        DrawRectangleLinesEx(iconRect, 1, YELLOW);

        // Center text inside icon
        int textWidth = MeasureText(actionIcon.text, 10);
        DrawText(actionIcon.text, x + 16 - textWidth / 2, y + 16 - 5, 10, YELLOW);

        // Check for a mouse click
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            return true;
        }
    } else {
        // Background color for the icon
        DrawRectangleRec(iconRect, actionIcon.disabled ? DARKGRAY : GRAY);
        DrawRectangleLinesEx(iconRect, 1, actionIcon.disabled ? GRAY : LIGHTGRAY);

        // Draw cooldown overlay with alpha gradient if the skill exists and has a cooldown
        if (actionIcon.skill != nullptr && actionIcon.skill->cooldown > 0) {
            float cooldownRatio = actionIcon.skill->cooldown / (float) actionIcon.skill->maxCooldown; // Cooldown ratio
            float overlayHeight = 32 * cooldownRatio; // Height of the overlay based on cooldown

            // Draw gradient overlay
            for (int i = 0; i < (int) overlayHeight; i++) {
                float alpha = 0.5f * ((overlayHeight - i) / overlayHeight); // Alpha decreases from top to bottom
                DrawRectangle(x, y + 32 - overlayHeight + i, 32, 1, Fade(BLACK, alpha));
            }
        }

        // Center text inside icon
        int textWidth = MeasureText(actionIcon.text, 10);
        DrawText(actionIcon.text, x + 16 - textWidth / 2, y + 16 - 5, 10, actionIcon.disabled ? LIGHTGRAY : WHITE);
    }
    return false;
}

static void DisplayActionUI(Level &combat, LevelScreen &uiState, PlayField &gridState) {
    //DrawRectangleRec((Rectangle) {0, 209, 480, 65}, Fade(BLACK, 0.75f));

    float iconWidth = 32;
    float iconHeight = 32;
    int visibleIcons = 14;

    float offsetX = 0;
    float offsetY = 236;
    Vector2 scrollLeft[3] = {{offsetX + 15, offsetY + 1},
                             {offsetX + 1,  offsetY + 16},
                             {offsetX + 15, offsetY + 31}};

    bool mouseOverScrollLeft = CheckCollisionPointTriangle(GetMousePosition(), scrollLeft[0], scrollLeft[1],
                                                           scrollLeft[2]);

    DrawTriangle(scrollLeft[0], scrollLeft[1], scrollLeft[2], mouseOverScrollLeft ? WHITE : GRAY);

    offsetX = 480 - 16;
    Vector2 scrollRight[3] = {{offsetX + 1,  offsetY + 1},
                              {offsetX + 1,  offsetY + 31},
                              {offsetX + 15, offsetY + 16}};
    bool mouseOverScrollRight = CheckCollisionPointTriangle(GetMousePosition(), scrollRight[0], scrollRight[1],
                                                            scrollRight[2]);
    DrawTriangle(scrollRight[0], scrollRight[1], scrollRight[2], mouseOverScrollRight ? WHITE : GRAY);

    std::vector<ActionIcon> actionIcons = {
            ActionIcon{"MOV", "Move", combat.currentCharacter->movePoints <= 0, nullptr},
            ActionIcon{"ATK", "Attack", false, nullptr},
            ActionIcon{"DEF", "Defend (50% baseAttack reduction, cannot attack)", false, nullptr},
            ActionIcon{"END", "End turn (Do nothing)", false, nullptr},
    };

    // add active character skills
    for (auto &skill: combat.currentCharacter->skills) {
        if (skill.isPassive) {
            continue;
        }
        ActionIcon icon{};
        strncpy(icon.text, skill.name, 4);
        // capitalize string
        for (int i = 0; i < 4; i++) {
            icon.text[i] = toupper(icon.text[i]);
        }
        strncpy(icon.description, skill.name, sizeof(skill.name));
        icon.disabled = skill.cooldown > 0;
        icon.skill = &skill;
        actionIcons.push_back(icon);
    }

    uiState.showActionBarTitle = true;

    //DisplayCharacterStatsUI(*combat.currentCharacter, true);
    // Draw 14 action icons 32x32 in a row, no spacing
    for (int i = 0; i < visibleIcons; ++i) {
        if (i >= actionIcons.size()) {
            break;
        }
        float iconX = 16 + i * iconWidth;
        float iconY = 236;
        if (DrawActionIcon(iconX, iconY, actionIcons[uiState.actionIconScrollIndex + i], uiState)) {
            PlaySoundEffect(SoundEffectType::Select);
            // action icon clicked
            TraceLog(LOG_INFO, "Action icon clicked: %s", actionIcons[uiState.actionIconScrollIndex + i].text);
            if (i + uiState.actionIconScrollIndex == 0) {
                // Move button pressed
                combat.turnState = TurnState::Waiting;
                combat.waitTime = 0.15f;
                combat.nextState = TurnState::SelectDestination;
                gridState.mode = PlayFieldMode::SelectingTile;
                break;
            }
            if (i + uiState.actionIconScrollIndex == 1) {
                // Attack button pressed
                combat.turnState = TurnState::Waiting;
                combat.waitTime = 0.15f;
                combat.nextState = TurnState::SelectEnemy;
                gridState.mode = PlayFieldMode::SelectingEnemyTarget;
                break;
            }
            if (i + uiState.actionIconScrollIndex == 2) {
                // Defend button pressed
                combat.turnState = TurnState::EndTurn;
                AssignStatusEffectAllowStacking(combat.currentCharacter->statusEffects, StatusEffectType::DamageReduction, 1, 0.5f);
                float charX = GetCharacterSpritePosX(combat.currentCharacter->sprite);
                float charY = GetCharacterSpritePosY(combat.currentCharacter->sprite);
                Animation anim{};
                SetupDamageNumberAnimation(anim, "DEFENDING", charX, charY - 25, WHITE, 10);
                combat.animations.push_back(anim);
                break;
            }
            if (i + uiState.actionIconScrollIndex == 3) {
                // End button pressed
                combat.turnState = TurnState::EndTurn;
                break;
            }
            if (i + uiState.actionIconScrollIndex > 3) {
                // Skill button pressed
                combat.turnState = TurnState::Waiting;
                combat.waitTime = 0.25f;
                combat.selectedSkill = actionIcons[uiState.actionIconScrollIndex + i].skill;
                if (combat.selectedSkill->noTarget) {
                    combat.nextState = TurnState::UseSkill;
                    combat.selectedCharacter = nullptr;
                } else {
                    combat.nextState = TurnState::SelectEnemy;
                    gridState.mode = PlayFieldMode::SelectingEnemyTarget;
                }
                break;
            }
        }

        //DrawRectangleLines(iconX, iconY, iconWidth, iconHeight, DARKGRAY);
    }

    if (uiState.showActionBarTitle) {
        DrawText("Select Action", 240 - (MeasureText("Select Action", 10) / 2), 220, 10, YELLOW);
    }

    if (CheckCollisionPointTriangle(GetMousePosition(), scrollRight[0], scrollRight[1], scrollRight[2]) &&
        IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        uiState.actionIconScrollIndex++;
    }
    if (CheckCollisionPointTriangle(GetMousePosition(), scrollLeft[0], scrollLeft[1], scrollLeft[2]) &&
        IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        uiState.actionIconScrollIndex--;
    }
    // clip scroll index
    if (uiState.actionIconScrollIndex < 0) {
        uiState.actionIconScrollIndex = 0;
    }
    if (uiState.actionIconScrollIndex > actionIcons.size() - visibleIcons) {
        uiState.actionIconScrollIndex = (int) actionIcons.size() - visibleIcons;
    }
    if (actionIcons.size() <= visibleIcons) {
        uiState.actionIconScrollIndex = 0;
    }
}

void DisplayCombatLog(Level &combat) {
    // Display Combat Log (at the bottom of the screen)
    int logStartY = 220;
    int logHeight = 40;
    int logLineHeight = 10;

    // Create a string for the last 5 lines of the combat log
    int logSize = (int) combat.log.size();
    int startIdx = (logSize > 4) ? logSize - 4 : 0;  // Correct calculation for the last 5 lines
    // Draw the background for the log area (lighter gray)
    //DrawRectangle(5, logStartY-5, 470, logHeight+10, DARKGRAY);
    DrawRectangleRounded((Rectangle) {5, (float) logStartY - 5, 470, (float) logHeight + 10}, 0.1f, 16, DARKGRAY);
    //DrawRectangleRoundedLinesEx((Rectangle) {5, (float) logStartY-5, 470, (float) logHeight+10}, 0.1f, 16, 1.0f, DARKGRAY);

    //std::string lines = "";
    for (int i = startIdx; i < logSize; ++i) {
        //lines += combat.log[i] + "\n";
        // Draw the text for the combat log entry
        int textWidth = MeasureText(combat.log[i].c_str(), 10);
        DrawText(combat.log[i].c_str(), 240 - textWidth / 2, logStartY + (i - startIdx) * logLineHeight, 10, LIGHTGRAY);
    }
    //GuiDrawText(lines.c_str(), (Rectangle) {0, (float) logStartY, 480, (float) logHeight}, TEXT_ALIGN_CENTER, DARKGRAY);

    // Draw each line of the log in the designated area
    /*
    for (int i = startIdx; i < logSize; ++i) {
        // Draw the text for the combat log entry
        int textWidth = MeasureText(combat.log[i].c_str(), 10);
        DrawText(combat.log[i].c_str(), 240 - textWidth / 2, logStartY + (i - startIdx) * logLineHeight, 10, DARKGRAY);
    }
    */
}

static void DisplayDamageNumbers(Level &combat) {
    for (auto &animation: combat.animations) {
        if (animation.type == AnimationType::DamageNumber) {
            float alpha = 1.0f - animation.time / animation.duration;
            // Draw the baseAttack number
            int w = MeasureText(animation.state.damageNumber.text, animation.state.damageNumber.fontSize);
            // Calculate the initial rectangle
            auto backgroundRect = (Rectangle) {(float) animation.state.damageNumber.x, (float) animation.state.damageNumber.y, (float) w+2, 12};

            // Adjust the rectangle position to fit within the screen boundaries
            /*
            if (backgroundRect.x + backgroundRect.width > 480) {
                backgroundRect.x = 480 - backgroundRect.width - 2; // Push left
            }
            if (backgroundRect.x < 0) {
                backgroundRect.x = 0; // Push right
            }
            if (backgroundRect.y + backgroundRect.height > 270) {
                backgroundRect.y = 270 - backgroundRect.height - 2; // Push up
            }
            if (backgroundRect.y < 0) {
                backgroundRect.y = 0; // Push down
            }
             */

            DrawText(animation.state.damageNumber.text, (int) backgroundRect.x, (int) backgroundRect.y, animation.state.damageNumber.fontSize,
                     Fade(animation.state.damageNumber.color, alpha));
        }
    }
}

static void DisplayTextAnimations(Level &combat) {
    for (auto &animation: combat.animations) {
        if (animation.type == AnimationType::Text) {
            // Draw veil
            DrawRectangle(0, 0, 480, 270, Fade(BLACK, animation.state.text.veilAlpha));
            DrawText(animation.state.text.text, 240 - (MeasureText(animation.state.text.text, 20) / 2),
                     (int) animation.state.text.y, 20, Fade(WHITE, animation.state.text.alpha));
        }
    }
}

static void DisplaySpeechBubbleAnimations(Level &combat) {
    for (auto &animation: combat.animations) {
        if (animation.type == AnimationType::SpeechBubble) {
            // Draw the speech bubble
            DrawSpeechBubble(animation.state.speechBubble.x, animation.state.speechBubble.y, animation.state.speechBubble.text, animation.state.speechBubble.alpha);
        }
    }
}


// Function to display combat screen
void DrawLevelScreen(Level &level, LevelScreen &levelScreen, PlayField &playField) {

    // Draw the dividing line in the middle of the screen
    //DrawLine(0, 125, 480, 125, LIGHTGRAY); // Horizontal dividing line

    // Draw the combat log
    /*
    if (combat.turnState != TurnState::Victory && combat.turnState != TurnState::Defeat &&
        combat.turnState != TurnState::SelectEnemy && combat.turnState != TurnState::SelectAction && combat.turnState != TurnState::SelectDestination) {
        DisplayCombatLog(combat);
    }
     */

    // Draw the grid

    BeginMode2D(level.camera.camera);
    DisplaySpeechBubbleAnimations(level);
    DisplayDamageNumbers(level);
    EndMode2D();


    if (level.turnState == TurnState::SelectAction) {
        DisplayActionUI(level, levelScreen, playField);
    }
    if (level.turnState == TurnState::Victory) {
        std::string text = "Victory!";
        // Draw the enemy selection UI
        DrawText(text.c_str(), 240 - (MeasureText(text.c_str(), 20) / 2), 10, 20, WHITE);
        if (GuiButton((Rectangle) {240 - 50, 245, 100, 20}, "End Battle")) {
            PublishEndCombatEvent(*levelScreen.eventQueue, true);
        }
    }
    if (level.turnState == TurnState::Defeat) {
        std::string text = "Defeat!";
        // Draw the enemy selection UI
        DrawText(text.c_str(), 240 - (MeasureText(text.c_str(), 20) / 2), 10, 20, WHITE);
        if (GuiButton((Rectangle) {240 - 50, 245, 100, 20}, "End Battle")) {
            PublishEndCombatEvent(*levelScreen.eventQueue, false);
        }
    }

    DisplayTextAnimations(level);

    if (playField.floatingStatsCharacter != nullptr && (level.turnState == TurnState::SelectAction ||
                                                        level.turnState == TurnState::SelectEnemy || level.turnState == TurnState::SelectDestination)) {
        float x = GetCharacterSpritePosX(playField.floatingStatsCharacter->sprite);
        float y = GetCharacterSpritePosY(playField.floatingStatsCharacter->sprite);
        // to screen space
        Vector2 screenPos = GetWorldToScreen2D(Vector2{x, y}, level.camera.camera);
        DisplayCharacterStatsFloating(*playField.floatingStatsCharacter, (int) screenPos.x - 10, (int) screenPos.y + 12,
                                      IsPlayerCharacter(*playField.floatingStatsCharacter));
        /*
        DisplayCharacterStatsFloating(*gridState.floatingStatsCharacter, (int) x - 10, (int) y + 12,
                                      IsPlayerCharacter(combat, *gridState.floatingStatsCharacter));
        */
    }
}

static void UpdateAnimations(Level &combat, float dt) {
    for (auto &anim : combat.animations) {
        UpdateAnimation(anim, dt);
    }
    // Use erase-remove idiom to remove animations which are done
    combat.animations.erase(
            std::remove_if(combat.animations.begin(), combat.animations.end(),
                           [](const Animation& anim) {
                               return anim.IsDone();
                           }),
            combat.animations.end()
    );
}

void UpdateLevelScreen(Level &level, LevelScreen &levelScreen, PlayField& playField, float dt) {

    switch(level.turnState) {
        case TurnState::StartRound: {
            TraceLog(LOG_INFO, "Start round");
            WaitTurnState(level, TurnState::StartTurn, 1.0f);
            ApplyStatusEffects(level, playField);
            PlaySoundEffect(SoundEffectType::StartRound);
            break;
        }
        case TurnState::StartTurn: {
            TraceLog(LOG_INFO, "Start turn");
            StartCameraPanToTargetChar(level.camera, level.currentCharacter, 500.0f);

            Animation blinkAnim{};

            // restore movepoints
            int movePoints = (int) (5 + sqrt(level.currentCharacter->speed) * 2);
            level.currentCharacter->movePoints = movePoints;
            TraceLog(LOG_INFO, "Restored move points for %s: %d", level.currentCharacter->name.c_str(), movePoints);

            level.waitTime = 0.5f;
            if(IsPlayerCharacter(*level.currentCharacter)) {
                SetupBlinkAnimation(blinkAnim, level.currentCharacter, 2.0f);
                level.nextState = TurnState::SelectAction;
            } else {
                SetupBlinkAnimation(blinkAnim, level.currentCharacter, 1.0f);
                level.nextState = TurnState::EnemyTurn;
            }
            if(IsIncapacitated(level.currentCharacter)) {
                level.nextState = TurnState::EndTurn;
                std::string logMessage = level.currentCharacter->name + " is skipping the turn!";
                level.log.push_back(logMessage);
                float charX = GetCharacterSpritePosX(level.currentCharacter->sprite);
                float charY = GetCharacterSpritePosY(level.currentCharacter->sprite);
                Animation anim{};
                SetupDamageNumberAnimation(anim, "STUNNED", charX, charY-25, WHITE, 10);
                level.animations.push_back(anim);
            }
            level.turnState = TurnState::Waiting;
            level.animations.push_back(blinkAnim);
            break;
        }
        case TurnState::EndTurn: {
            NextCharacter(level);
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
            float attackerX = GetCharacterSpritePosX(level.currentCharacter->sprite);
            float attackerY = GetCharacterSpritePosY(level.currentCharacter->sprite);

            SkillResult result = ExecuteSkill(level, playField);
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
                SetTaunt(level, level.currentCharacter);
            }
            level.selectedSkill = nullptr;
            break;
        }
        case TurnState::Attack: {
            TraceLog(LOG_INFO, "Attack");

            level.attackResult = Attack(level, *level.currentCharacter, *level.selectedCharacter);
            FaceCharacter(*level.currentCharacter, *level.selectedCharacter);
            FaceCharacter(*level.selectedCharacter, *level.currentCharacter);
            assert(level.attackResult.defender == level.selectedCharacter);
            assert(level.attackResult.attacker == level.currentCharacter);

            PlayAttackDefendAnimation(level, *level.currentCharacter, *level.selectedCharacter);

            level.waitTime = 0.25f;
            level.nextState = TurnState::AttackDone;
            level.turnState = TurnState::Waiting;
            break;
        }
        case TurnState::AttackDone: {
            assert(level.attackResult.defender == level.selectedCharacter);
            assert(level.attackResult.attacker == level.currentCharacter);
            float attackerX = GetCharacterSpritePosX(level.currentCharacter->sprite);
            float attackerY = GetCharacterSpritePosY(level.currentCharacter->sprite);
            float defenderX = GetCharacterSpritePosX(level.selectedCharacter->sprite);
            float defenderY = GetCharacterSpritePosY(level.selectedCharacter->sprite);
            int damage = level.attackResult.damage;
            if(damage > 0) {
                float intensity = (float) GetBloodIntensity(damage, GetAttack(*level.currentCharacter));
                TraceLog(LOG_INFO, "Damage: %d, intensity: %f", damage, intensity);
                Vector2 bloodPos = {defenderX + (float) RandomInRange(-2,2), defenderY - 8 + (float) RandomInRange(-2,2)};
                CreateBloodSplatter(*playField.particleManager, bloodPos, 10, intensity);
                Animation damageNumberAnim{};
                Color dmgColor = GetDamageColor(damage, GetAttack(*level.currentCharacter));
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

            level.attackResult.defender->health -= damage;
            if(level.attackResult.defender->health <= 0) {
                Animation speechBubble{};
                SetupSpeechBubbleAnimation(speechBubble, "Haha!", attackerX, attackerY - 25, 1.5f, 0.0f);
                level.animations.push_back(speechBubble);
                RemoveAttackAnimations(level);
                KillCharacter(level, *level.attackResult.defender);
                WaitTurnState(level, TurnState::EndTurn, 0.95f);
            } else {
                WaitTurnState(level, TurnState::EndTurn, 0.60f);
            }
            break;
        }
        case TurnState::EnemyTurn: {
            TraceLog(LOG_INFO, "Enemy turn");

            // obtain AiInterface
            AiInterface* ai = GetAiInterface(level.currentCharacter->ai);
            if(ai != nullptr) {
                HandleTurn(*ai, level, playField);
            } else {
                TraceLog(LOG_WARNING, "No AI interface found for %s", level.currentCharacter->ai.c_str());
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
            UpdateStatusEffects(level);
            DecayThreat(level, 10);
            UpdateSkillCooldown(level);
            break;
        }
    }
    UpdateAnimations(level, dt);
    if(level.turnState != TurnState::None) {
        // check victory condition, all enemies have zero health
        bool allEnemiesDefeated = true;
        for (auto &enemy: level.enemyCharacters) {
            if (enemy->health > 0) {
                allEnemiesDefeated = false;
                break;
            }
        }
        if (allEnemiesDefeated && level.turnState != TurnState::Victory) {
            level.turnState = TurnState::Victory;
            //StopSoundEffect(SoundEffectType::Ambience);
            PlaySoundEffect(SoundEffectType::Victory, 0.5f);
            PlayPlayerVictoryAnimation(level);
            //combat.animations.clear();
        }
        // check defeat condition, all players have zero health
        bool allPlayersDefeated = true;
        for (auto &player: level.partyCharacters) {
            if (player->health > 0) {
                allPlayersDefeated = false;
                break;
            }
        }
        if (allPlayersDefeated && level.turnState != TurnState::Defeat) {
            level.turnState = TurnState::Defeat;
            //StopSoundEffect(SoundEffectType::Ambience);
            PlaySoundEffect(SoundEffectType::Defeat, 0.5f);
            PlayEnemyVictoryAnimation(level);
            //combat.animations.clear();
        }
    }
}

void HandleInputLevelScreen(LevelScreen &levelScreen, Level &level) {

}

