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
#include "level/CombatAnimation.h"
#include "audio/SoundEffect.h"
#include "graphics/Animation.h"
#include "Combat.h"
#include "ai/PathFinding.h"
#include <cassert>

void CreateLevelScreen(LevelScreen &levelScreen, GameEventQueue* eventQueue) {
    levelScreen.actionIconScrollIndex = 0;
    levelScreen.showActionBarTitle = true;
    levelScreen.eventQueue = eventQueue;
    levelScreen.floatingStatsCharacter = -1;
}

void DestroyLevelScreen(LevelScreen &levelScreen) {

}

static bool IsCharacterVisible(Level &combat, int character) {
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
        DrawStatusTextBg(actionIcon.description, WHITE, 220, 10);
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

static void DisplayActionUI(SpriteData& spriteData, CharacterData& charData, Level &combat, LevelScreen &uiState, PlayField &gridState) {
    //DrawRectangleRec((Rectangle) {0, 209, 480, 65}, Fade(BLACK, 0.75f));

    float iconWidth = 32;
    float iconHeight = 32;
    int visibleIcons = 14;

    float offsetX = 0;
    float offsetY = 326;
    Vector2 scrollLeft[3] = {{offsetX + 15, offsetY + 1},
                             {offsetX + 1,  offsetY + 16},
                             {offsetX + 15, offsetY + 31}};

    bool mouseOverScrollLeft = CheckCollisionPointTriangle(GetMousePosition(), scrollLeft[0], scrollLeft[1],
                                                           scrollLeft[2]);

    DrawTriangle(scrollLeft[0], scrollLeft[1], scrollLeft[2], mouseOverScrollLeft ? WHITE : GRAY);

    offsetX = gameScreenWidthF - 16;
    Vector2 scrollRight[3] = {{offsetX + 1,  offsetY + 1},
                              {offsetX + 1,  offsetY + 31},
                              {offsetX + 15, offsetY + 16}};
    bool mouseOverScrollRight = CheckCollisionPointTriangle(GetMousePosition(), scrollRight[0], scrollRight[1],
                                                            scrollRight[2]);
    DrawTriangle(scrollRight[0], scrollRight[1], scrollRight[2], mouseOverScrollRight ? WHITE : GRAY);

    std::vector<ActionIcon> actionIcons = {
            ActionIcon{"MOV", "Move", charData.stats[combat.currentCharacter].movePoints <= 0, nullptr},
            ActionIcon{"ATK", "Attack", false, nullptr},
            ActionIcon{"DEF", "Defend (50% baseAttack reduction, cannot attack)", false, nullptr},
            ActionIcon{"END", "End turn (Do nothing)", false, nullptr},
    };

    // add active character skills
    for (auto &skill: charData.skills[combat.currentCharacter]) {
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
        float iconY = 326;
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
                AssignStatusEffectAllowStacking(charData.statusEffects[combat.currentCharacter], StatusEffectType::DamageReduction, 1, 0.5f);
                float charX = GetCharacterSpritePosX(spriteData, charData.sprite[combat.currentCharacter]);
                float charY = GetCharacterSpritePosY(spriteData, charData.sprite[combat.currentCharacter]);
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
                    combat.selectedCharacter = -1;
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
        DrawStatusTextBg("Select Action", WHITE, 10, 10);
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
            DrawRectangle(0, 0, gameScreenWidth, gameScreenHeight, Fade(BLACK, animation.state.text.veilAlpha));
            DrawText(animation.state.text.text, gameScreenHalfWidth - (MeasureText(animation.state.text.text, 20) / 2),
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

void DrawLevelScreen(GameData& data, Level &level, LevelScreen &levelScreen, PlayField &playField) {
    SpriteData& spriteData = data.spriteData;
    CharacterData& charData = data.charData;
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
        DisplayActionUI(spriteData, charData, level, levelScreen, playField);
    }
    if (level.turnState == TurnState::Victory) {
        std::string text = "Victory!";
        // Draw the enemy selection UI
        DrawText(text.c_str(), gameScreenHalfWidth - (MeasureText(text.c_str(), 20) / 2), 10, 20, WHITE);
        if (GuiButton((Rectangle) {gameScreenHalfWidthF - 50, 340, 100, 20}, "End Battle")) {
            PublishEndCombatEvent(*levelScreen.eventQueue, true);
        }
    }
    if (level.turnState == TurnState::Defeat) {
        std::string text = "Defeat!";
        // Draw the enemy selection UI
        DrawText(text.c_str(), gameScreenHalfWidth - (MeasureText(text.c_str(), 20) / 2), 10, 20, WHITE);
        if (GuiButton((Rectangle) {gameScreenHalfWidthF - 50, 340, 100, 20}, "End Battle")) {
            PublishEndCombatEvent(*levelScreen.eventQueue, false);
        }
    }

    DisplayTextAnimations(level);

    if (data.state != GameState::DIALOGUE && levelScreen.floatingStatsCharacter != -1 && (level.turnState == TurnState::None || level.turnState == TurnState::SelectAction ||
                                                        level.turnState == TurnState::SelectEnemy || level.turnState == TurnState::SelectDestination)) {
        float x = GetCharacterSpritePosX(spriteData, charData.sprite[levelScreen.floatingStatsCharacter]);
        float y = GetCharacterSpritePosY(spriteData, charData.sprite[levelScreen.floatingStatsCharacter]);
        // to screen space
        Vector2 screenPos = GetWorldToScreen2D(Vector2{x, y}, level.camera.camera);
        DisplayCharacterStatsFloating(charData, levelScreen.floatingStatsCharacter, (int) screenPos.x - 10, (int) screenPos.y + 12,
                                      IsPlayerCharacter(charData, levelScreen.floatingStatsCharacter));
    }
    // Display hint text
    if(!playField.hintText.empty()) {
        DrawStatusTextBg(playField.hintText.c_str(), WHITE, 10, 10);
        playField.hintText = "";
    }
}

static void UpdateAnimations(SpriteData& spriteData, CharacterData& charData, Level &combat, float dt) {
    for (auto &anim : combat.animations) {
        UpdateAnimation(spriteData, charData, anim, dt);
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

void UpdateLevelScreen(SpriteData& spriteData, CharacterData& charData, Level &level, LevelScreen &levelScreen, float dt) {
    UpdateAnimations(spriteData, charData, level, dt);
}

void HandleInputLevelScreen(SpriteData& spriteData, CharacterData& charData, LevelScreen &levelScreen, Level &level) {
    // get mouse position
    levelScreen.floatingStatsCharacter = -1;
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), level.camera.camera);
    Vector2 gridPos = PixelToGridPosition(mousePos.x, mousePos.y);
    // check if mouse is over character
    for (auto &character: level.allCharacters) {
        // skip dead
        if (charData.stats[character].health <= 0) {
            continue;
        }
        Vector2 gridPosCharacter = PixelToGridPosition(GetCharacterSpritePosX(spriteData, charData.sprite[character]),
                                                       GetCharacterSpritePosY(spriteData, charData.sprite[character]));
        if ((int) gridPosCharacter.x == (int) gridPos.x && (int) gridPosCharacter.y == (int) gridPos.y) {
            levelScreen.floatingStatsCharacter = character;
        }
    }
}

