//
// Created by bison on 10-01-25.
//

#include <algorithm>
#include <cstring>
#include "CombatScreen.h"
#include "raylib.h"

#define RAYGUI_IMPLEMENTATION

#include "raygui.h"
#include "Character.h"
#include "Grid.h"
#include "ui.h"
#include "Skills.h"
#include "ai/Ai.h"

const Color BACKGROUND_GREY = Color{50, 50, 50, 255};

void InitCombatUIState(CombatUIState &uiState) {
    uiState.actionIconScrollIndex = 0;
    uiState.showActionBarTitle = true;
}

static bool IsCharacterVisible(CombatState &combat, Character *character) {
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

static bool DrawActionIcon(float x, float y, ActionIcon &actionIcon, CombatUIState &uiState) {
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

static void DisplayActionUI(CombatState &combat, CombatUIState &uiState, GridState &gridState) {
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
            ActionIcon{"DEF", "Defend (50% damage reduction, cannot attack)", false, nullptr},
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
            // action icon clicked
            TraceLog(LOG_INFO, "Action icon clicked: %s", actionIcons[uiState.actionIconScrollIndex + i].text);
            if (i + uiState.actionIconScrollIndex == 0) {
                // Move button pressed
                combat.turnState = TurnState::Waiting;
                combat.waitTime = 0.15f;
                combat.nextState = TurnState::SelectDestination;
                gridState.mode = GridMode::SelectingTile;
                break;
            }
            if (i + uiState.actionIconScrollIndex == 1) {
                // Attack button pressed
                combat.turnState = TurnState::Waiting;
                combat.waitTime = 0.15f;
                combat.nextState = TurnState::SelectEnemy;
                gridState.mode = GridMode::SelectingEnemyTarget;
                break;
            }
            if (i + uiState.actionIconScrollIndex == 2) {
                // Defend button pressed
                combat.turnState = TurnState::EndTurn;
                StatusEffect effect = {StatusEffectType::DamageReduction, 1, 0.5f};
                combat.currentCharacter->statusEffects.push_back(effect);
                float charX = combat.currentCharacter->sprite.player.position.x;
                float charY = combat.currentCharacter->sprite.player.position.y;
                Animation anim{};
                SetupDamageNumberAnimation(anim, "DEFENDING", charX, charY - 25, WHITE, 10);
                combat.animations.push_back(anim);
                break;
            }
            if (i + uiState.actionIconScrollIndex > 2) {
                // Skill button pressed
                combat.turnState = TurnState::Waiting;
                combat.waitTime = 0.25f;
                combat.selectedSkill = actionIcons[uiState.actionIconScrollIndex + i].skill;
                if (combat.selectedSkill->noTarget) {
                    combat.nextState = TurnState::UseSkill;
                    combat.selectedCharacter = nullptr;
                } else {
                    combat.nextState = TurnState::SelectEnemy;
                    gridState.mode = GridMode::SelectingEnemyTarget;
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

void DisplayCombatLog(CombatState &combat) {
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

static void DisplayDamageNumbers(CombatState &combat) {
    for (auto &animation: combat.animations) {
        if (animation.type == AnimationType::DamageNumber) {
            float alpha = 1.0f - animation.time / animation.duration;
            // Draw the damage number
            int w = MeasureText(animation.state.damageNumber.text, animation.state.damageNumber.fontSize);
            // Calculate the initial rectangle
            auto backgroundRect = (Rectangle) {(float) animation.state.damageNumber.x, (float) animation.state.damageNumber.y, (float) w+2, 12};

            // Adjust the rectangle position to fit within the screen boundaries
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

            DrawText(animation.state.damageNumber.text, (int) backgroundRect.x, (int) backgroundRect.y, animation.state.damageNumber.fontSize,
                     Fade(animation.state.damageNumber.color, alpha));
        }
    }
}

static void DisplayTextAnimations(CombatState &combat) {
    for (auto &animation: combat.animations) {
        if (animation.type == AnimationType::Text) {
            // Draw veil
            DrawRectangle(0, 0, 480, 270, Fade(BLACK, animation.state.text.veilAlpha));
            DrawText(animation.state.text.text, 240 - (MeasureText(animation.state.text.text, 20) / 2),
                     (int) animation.state.text.y, 20, Fade(WHITE, animation.state.text.alpha));
        }
    }
}

static void DisplaySpeechBubbleAnimations(CombatState &combat) {
    for (auto &animation: combat.animations) {
        if (animation.type == AnimationType::SpeechBubble) {
            // Draw the speech bubble
            DrawSpeechBubble(animation.state.speechBubble.x, animation.state.speechBubble.y, animation.state.speechBubble.text, animation.state.speechBubble.alpha);
        }
    }
}


// Function to display combat screen
void DisplayCombatScreen(CombatState &combat, CombatUIState &uiState, GridState &gridState) {
    // Clear screen with a background color (dark gray)
    ClearBackground(BACKGROUND_GREY);

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
    DrawGrid(gridState, combat);


    if (combat.turnState == TurnState::SelectAction) {
        DisplayActionUI(combat, uiState, gridState);
    }
    if (combat.turnState == TurnState::Victory) {
        std::string text = "Victory!";
        // Draw the enemy selection UI
        DrawText(text.c_str(), 240 - (MeasureText(text.c_str(), 20) / 2), 10, 20, WHITE);
        if (GuiButton((Rectangle) {240 - 50, 245, 100, 20}, "End Battle")) {

        }
    }
    if (combat.turnState == TurnState::Defeat) {
        std::string text = "Defeat!";
        // Draw the enemy selection UI
        DrawText(text.c_str(), 240 - (MeasureText(text.c_str(), 20) / 2), 10, 20, WHITE);
        if (GuiButton((Rectangle) {240 - 50, 245, 100, 20}, "End Battle")) {

        }
    }

    DisplaySpeechBubbleAnimations(combat);

    DisplayDamageNumbers(combat);
    DisplayTextAnimations(combat);

    if (gridState.floatingStatsCharacter != nullptr && (combat.turnState == TurnState::SelectAction ||
        combat.turnState == TurnState::SelectEnemy || combat.turnState == TurnState::SelectDestination)) {
        float x = gridState.floatingStatsCharacter->sprite.player.position.x;
        float y = gridState.floatingStatsCharacter->sprite.player.position.y;
        DisplayCharacterStatsFloating(*gridState.floatingStatsCharacter, (int) x - 10, (int) y + 12,
                                      IsPlayerCharacter(combat, *gridState.floatingStatsCharacter));
    }
}

static void UpdateAnimations(CombatState &combat, float dt) {
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

static void SetSpriteAnimPaused(CharacterSprite& sprite, SpriteAnimationType type) {
    PlaySpriteAnimation(sprite.player, GetCharacterAnimation(sprite, type), true);
    SetFrame(sprite.player, 0);
    sprite.player.playing = false;
}

static void FaceCharacter(Character &attacker, Character &defender) {
    // Determine the direction of movement and set the appropriate animation
    Vector2 start = attacker.sprite.player.position;
    Vector2 end = defender.sprite.player.position;
    if (fabs(end.x - start.x) > fabs(end.y - start.y)) {
        // Horizontal movement
        if (end.x > start.x) {
            SetSpriteAnimPaused(attacker.sprite, SpriteAnimationType::WalkRight);
        } else {
            SetSpriteAnimPaused(attacker.sprite, SpriteAnimationType::WalkLeft);
        }
    } else {
        // Vertical movement
        if (end.y > start.y) {
            SetSpriteAnimPaused(attacker.sprite, SpriteAnimationType::WalkDown);
        } else {
            SetSpriteAnimPaused(attacker.sprite, SpriteAnimationType::WalkUp);
        }
    }
}

void UpdateCombatScreen(CombatState &combat, CombatUIState &uiState, GridState& gridState, float dt) {
    switch(combat.turnState) {
        case TurnState::StartTurn: {
            TraceLog(LOG_INFO, "Start turn");
            Animation blinkAnim{};

            // restore movepoints
            int movePoints = (int) (5 + sqrt(combat.currentCharacter->speed) * 2);
            combat.currentCharacter->movePoints = movePoints;
            TraceLog(LOG_INFO, "Restored move points for %s: %d", combat.currentCharacter->name.c_str(), movePoints);


            combat.waitTime = 0.5f;
            if(IsPlayerCharacter(combat, *combat.currentCharacter)) {
                SetupBlinkAnimation(blinkAnim, combat.currentCharacter, 2.0f);
                combat.nextState = TurnState::SelectAction;
            } else {
                SetupBlinkAnimation(blinkAnim, combat.currentCharacter, 1.0f);
                combat.nextState = TurnState::EnemyTurn;
            }
            if(IsIncapacitated(combat.currentCharacter)) {
                combat.nextState = TurnState::EndTurn;
                std::string logMessage = combat.currentCharacter->name + " is skipping the turn!";
                combat.log.push_back(logMessage);
                float charX = combat.currentCharacter->sprite.player.position.x;
                float charY = combat.currentCharacter->sprite.player.position.y;
                Animation anim{};
                SetupDamageNumberAnimation(anim, "STUNNED", charX, charY-25, WHITE, 10);
                combat.animations.push_back(anim);
            }
            combat.turnState = TurnState::Waiting;
            combat.animations.push_back(blinkAnim);
            break;
        }
        case TurnState::EndTurn: {
            NextCharacter(combat);
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
            float attackerX = combat.currentCharacter->sprite.player.position.x;
            float attackerY = combat.currentCharacter->sprite.player.position.y;
            if(!combat.selectedSkill->noTarget) {
                float defenderX = combat.selectedCharacter->sprite.player.position.x;
                float defenderY = combat.selectedCharacter->sprite.player.position.y;
                FaceCharacter(*combat.currentCharacter, *combat.selectedCharacter);
                FaceCharacter(*combat.selectedCharacter, *combat.currentCharacter);
                Animation attackAnim{};
                if(IsPlayerCharacter(combat, *combat.currentCharacter)) {
                    SetupAttackAnimation(attackAnim, combat.currentCharacter, 0.5f, attackerY, defenderY, attackerX, defenderX);
                } else {
                    SetupAttackAnimation(attackAnim, combat.currentCharacter, 0.5f, attackerY, defenderY, attackerX, defenderX);
                }
                combat.animations.push_back(attackAnim);
            }
            combat.waitTime = 1.0f;
            combat.nextState = TurnState::EndTurn;
            combat.turnState = TurnState::Waiting;
            SkillResult result = UseSkill(combat.selectedSkill, *combat.currentCharacter, *combat.selectedCharacter);
            Animation damageNumberAnim{};
            if(!combat.selectedSkill->noTarget) {
                float defenderX = combat.selectedCharacter->sprite.player.position.x;
                float defenderY = combat.selectedCharacter->sprite.player.position.y;
                if(result.success) {
                    SetupDamageNumberAnimation(damageNumberAnim, combat.selectedSkill->name, defenderX, defenderY-25, YELLOW, 10);
                } else {
                    SetupDamageNumberAnimation(damageNumberAnim, "FAILED", attackerX, attackerY-25, WHITE, 10);
                }
            } else {
                if(result.success) {
                    SetupDamageNumberAnimation(damageNumberAnim, combat.selectedSkill->name, attackerX, attackerY-25, YELLOW, 10);
                } else {
                    SetupDamageNumberAnimation(damageNumberAnim, "FAILED", attackerX, attackerY-25, WHITE, 10);
                }
            }

            combat.log.push_back(result.message);
            combat.animations.push_back(damageNumberAnim);
            if(result.attack) {
                combat.nextState = TurnState::AttackDone;
            } else if(!result.consumeAction) {
                combat.nextState = TurnState::SelectAction;
            } else {
                combat.nextState = TurnState::EndTurn;
            }
            if(result.giveAggro) {
                SetTaunt(combat, combat.currentCharacter);
            }
            combat.selectedSkill = nullptr;
            break;
        }
        case TurnState::Attack: {
            TraceLog(LOG_INFO, "Attack");
            FaceCharacter(*combat.currentCharacter, *combat.selectedCharacter);
            FaceCharacter(*combat.selectedCharacter, *combat.currentCharacter);
            Animation attackAnim{};

            float attackerX = combat.currentCharacter->sprite.player.position.x;
            float defenderX = combat.selectedCharacter->sprite.player.position.x;
            float attackerY = combat.currentCharacter->sprite.player.position.y;
            float defenderY = combat.selectedCharacter->sprite.player.position.y;
            if(IsPlayerCharacter(combat, *combat.currentCharacter)) {
                SetupAttackAnimation(attackAnim, combat.currentCharacter, 0.5f, attackerY, defenderY, attackerX, defenderX);
            } else {
                SetupAttackAnimation(attackAnim, combat.currentCharacter, 0.5f, attackerY, defenderY, attackerX, defenderX);
            }

            combat.animations.push_back(attackAnim);
            combat.waitTime = 0.75f;
            combat.nextState = TurnState::AttackDone;
            combat.turnState = TurnState::Waiting;
            break;
        }
        case TurnState::AttackDone: {
            Attack(combat, *combat.currentCharacter, *combat.selectedCharacter);
            if(combat.selectedCharacter->health <= 0) {

                float attackerX = combat.currentCharacter->sprite.player.position.x;
                float defenderX = combat.selectedCharacter->sprite.player.position.x;
                float attackerY = combat.currentCharacter->sprite.player.position.y;
                float defenderY = combat.selectedCharacter->sprite.player.position.y;
                Animation speechBubble{};
                SetupSpeechBubbleAnimation(speechBubble, "Haha!", attackerX, attackerY - 25, 1.5f, 0.0f);
                combat.animations.push_back(speechBubble);

                std::string logMessage = combat.selectedCharacter->name + " is defeated!";
                combat.log.push_back(logMessage);
                Animation deathAnim{};
                SetupDeathAnimation(deathAnim, combat.selectedCharacter, 0.5f);
                combat.animations.push_back(deathAnim);
                combat.waitTime = 0.6f;
                combat.nextState = TurnState::EndTurn;
                combat.turnState = TurnState::Waiting;
                Animation bloodAnim{};
                SetupBloodPoolAnimation(bloodAnim, combat.selectedCharacter->sprite.player.position, 5.0f);
                combat.animations.push_back(bloodAnim);
            } else {
                combat.turnState = TurnState::EndTurn;
            }
            break;
        }
        case TurnState::EnemyTurn: {
            //TraceLog(LOG_INFO, "Enemy turn");
            // Select a random alive player character to attack
            /*
            while(combat.selectedCharacter == nullptr || combat.selectedCharacter->health <= 0) {
                combat.selectedCharacter = combat.playerCharacters[RandomInRange(0, combat.playerCharacters.size() - 1)];
            }
            */

            // obtain AiInterface
            AiInterface* ai = GetAiInterface(combat.currentCharacter->ai);
            if(ai != nullptr) {
                HandleTurn(*ai, combat, gridState);
            } else {
                TraceLog(LOG_WARNING, "No AI interface found for %s", combat.currentCharacter->ai.c_str());
                combat.turnState = TurnState::EndTurn;
            }
            break;
        }
        case TurnState::Waiting: {
            combat.waitTime -= dt;
            if (combat.waitTime <= 0) {
                combat.turnState = combat.nextState;
                TraceLog(LOG_INFO, "Waiting done");
            }
            break;
        }
        case TurnState::EndRound: {
            Animation textAnim{};
            SetupTextAnimation(textAnim, "Next round!", 125, 1.0f, 1.0f);
            combat.animations.push_back(textAnim);
            combat.waitTime = 2.0f;
            combat.nextState = TurnState::StartTurn;
            combat.turnState = TurnState::Waiting;
            UpdateStatusEffects(combat);
            DecayThreat(combat, 10);
            UpdateSkillCooldown(combat);
            break;
        }
    }
    UpdateAnimations(combat, dt);
    // check victory condition, all enemies have zero health
    bool allEnemiesDefeated = true;
    for (auto &enemy : combat.enemyCharacters) {
        if (enemy->health > 0) {
            allEnemiesDefeated = false;
            break;
        }
    }
    if(allEnemiesDefeated) {
        combat.turnState = TurnState::Victory;
        //combat.animations.clear();
    }
    // check defeat condition, all players have zero health
    bool allPlayersDefeated = true;
    for (auto &player : combat.playerCharacters) {
        if (player->health > 0) {
            allPlayersDefeated = false;
            break;
        }
    }
    if(allPlayersDefeated) {
        combat.turnState = TurnState::Defeat;
        //combat.animations.clear();
    }
}
