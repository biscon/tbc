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

void CreateLevelScreen(GameData& data) {
    data.ui.actionBar.actionIconScrollIndex = 0;
    data.ui.actionBar.showActionBarTitle = true;
    data.ui.playField.floatingStatsCharacter = -1;
}

void DestroyLevelScreen(GameData& data) {

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

static bool DrawActionIcon(GameData& data, float x, float y, ActionIcon &actionIcon, Font font) {
    Vector2 mousePos = GetMousePosition();
    Rectangle iconRect = {x, y, 24, 24};
    static const Color bgColor = Color{15, 15, 15, 200};
    // Center text inside icon
    Vector2 textDim = MeasureTextEx(font, actionIcon.text, 5, 1);
    Vector2 textPos = {
            roundf(x + iconRect.width/2 - textDim.x / 2),
            floorf(y + iconRect.width/2 - textDim.y / 2)
    };

    if (CheckCollisionPointRec(mousePos, iconRect) && !actionIcon.disabled) {
        data.ui.actionBar.showActionBarTitle = false;
        DrawStatusTextBg(actionIcon.description, WHITE, 318, 5, font);
        DrawRectangleRec(iconRect, bgColor);
        DrawRectangleLinesEx(iconRect, 1, YELLOW);

        DrawTextEx(font, actionIcon.text, textPos, 5, 1, YELLOW);

        //int textWidth = MeasureText(actionIcon.text, 10);
        //DrawText(actionIcon.text, x + 16 - textWidth / 2, y + 16 - 5, 10, YELLOW);

        // Check for a mouse click
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            return true;
        }
    } else {
        // Background color for the icon
        //DrawRectangleRec(iconRect, actionIcon.disabled ? bgColor : bgColor);
        //DrawRectangleLinesEx(iconRect, 1, DARKGRAY);

        DrawRectangleRounded(iconRect, 0.1f, 16, bgColor);
        DrawRectangleRoundedLinesEx(iconRect, 0.1f, 16, 1.0f, DARKGRAY);


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
        DrawTextEx(font, actionIcon.text, textPos, 5, 1, LIGHTGRAY);
    }
    return false;
}

static void DisplayActionUI(GameData& data, Level &combat, PlayField &gridState, Font font) {
    SpriteData& spriteData = data.spriteData;
    CharacterData& charData = data.charData;

    float iconWidth = 24;
    float iconHeight = 24;
    int visibleIcons = 10;
    float spacing = 3.0f;

    float barWidth = (float) visibleIcons * (iconWidth + spacing);
    float iconX = gameScreenHalfWidthF - barWidth/2;
    float iconY = 331;

    float offsetX = iconX - 14;
    float offsetY = iconY;
    Vector2 scrollLeft[3] = {{offsetX + 11, offsetY + 1},
                             {offsetX + 1,  offsetY + 12},
                             {offsetX + 11, offsetY + 23}};

    bool mouseOverScrollLeft = CheckCollisionPointTriangle(GetMousePosition(), scrollLeft[0], scrollLeft[1],
                                                           scrollLeft[2]);

    DrawTriangle(scrollLeft[0], scrollLeft[1], scrollLeft[2], mouseOverScrollLeft ? YELLOW : DARKGRAY);

    offsetX = iconX + barWidth - 1;
    Vector2 scrollRight[3] = {{offsetX + 1,  offsetY + 1},
                              {offsetX + 1,  offsetY + 23},
                              {offsetX + 11, offsetY + 12}};
    bool mouseOverScrollRight = CheckCollisionPointTriangle(GetMousePosition(), scrollRight[0], scrollRight[1],
                                                            scrollRight[2]);
    DrawTriangle(scrollRight[0], scrollRight[1], scrollRight[2], mouseOverScrollRight ? YELLOW : DARKGRAY);

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

    data.ui.actionBar.showActionBarTitle = true;

    // Draw 14 action icons 32x32 in a row, no spacing

    for (int i = 0; i < visibleIcons; ++i) {
        if (i >= actionIcons.size()) {
            break;
        }

        if (DrawActionIcon(data, iconX, iconY, actionIcons[data.ui.actionBar.actionIconScrollIndex + i], font)) {
            PlaySoundEffect(SoundEffectType::Select);
            // action icon clicked
            TraceLog(LOG_INFO, "Action icon clicked: %s", actionIcons[data.ui.actionBar.actionIconScrollIndex + i].text);
            if (i + data.ui.actionBar.actionIconScrollIndex == 0) {
                // Move button pressed
                combat.turnState = TurnState::Waiting;
                combat.waitTime = 0.15f;
                combat.nextState = TurnState::SelectDestination;
                gridState.mode = PlayFieldMode::SelectingTile;
                break;
            }
            if (i + data.ui.actionBar.actionIconScrollIndex == 1) {
                // Attack button pressed
                combat.turnState = TurnState::Waiting;
                combat.waitTime = 0.15f;
                combat.nextState = TurnState::SelectEnemy;
                gridState.mode = PlayFieldMode::SelectingEnemyTarget;
                break;
            }
            if (i + data.ui.actionBar.actionIconScrollIndex == 2) {
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
            if (i + data.ui.actionBar.actionIconScrollIndex == 3) {
                // End button pressed
                combat.turnState = TurnState::EndTurn;
                break;
            }
            if (i + data.ui.actionBar.actionIconScrollIndex > 3) {
                // Skill button pressed
                combat.turnState = TurnState::Waiting;
                combat.waitTime = 0.25f;
                combat.selectedSkill = actionIcons[data.ui.actionBar.actionIconScrollIndex + i].skill;
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
        iconX += iconWidth + spacing;
    }

    if (data.ui.actionBar.showActionBarTitle) {
        DrawStatusTextBg("Select Action", WHITE, 318, 5, font);
    }

    if (CheckCollisionPointTriangle(GetMousePosition(), scrollRight[0], scrollRight[1], scrollRight[2]) &&
        IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        data.ui.actionBar.actionIconScrollIndex++;
    }
    if (CheckCollisionPointTriangle(GetMousePosition(), scrollLeft[0], scrollLeft[1], scrollLeft[2]) &&
        IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        data.ui.actionBar.actionIconScrollIndex--;
    }
    // clip scroll index
    if (data.ui.actionBar.actionIconScrollIndex < 0) {
        data.ui.actionBar.actionIconScrollIndex = 0;
    }
    if (data.ui.actionBar.actionIconScrollIndex > actionIcons.size() - visibleIcons) {
        data.ui.actionBar.actionIconScrollIndex = (int) actionIcons.size() - visibleIcons;
    }
    if (actionIcons.size() <= visibleIcons) {
        data.ui.actionBar.actionIconScrollIndex = 0;
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

static void DisplayTextAnimations(Level &level) {
    for (auto &animation: level.animations) {
        switch (animation.type) {
            case AnimationType::Text: {
                // Draw veil
                DrawRectangle(0, 0, gameScreenWidth, gameScreenHeight, Fade(BLACK, animation.state.text.veilAlpha));
                DrawText(animation.state.text.text,
                         gameScreenHalfWidth - (MeasureText(animation.state.text.text, 20) / 2),
                         (int) animation.state.text.y, 20,
                         Fade(WHITE, animation.state.text.alpha));
                break;
            }
            case AnimationType::FancyText: {
                const FancyTextAnimationState& state = animation.state.fancyText;
                const char* src = state.text;
                int totalLen = (int) strlen(src);

                char displayText[129] = {0};

                for (int i = 0; i < totalLen; ++i) {
                    if (i < state.finalRevealLength) {
                        // Fully revealed
                        displayText[i] = src[i];
                    } else if (i < state.scrambleLength) {
                        // Scrambled
                        displayText[i] = (char) GetRandomValue(33, 126); // printable ASCII
                    } else {
                        // Not yet shown
                        displayText[i] = '\0';
                        break; // stop here
                    }
                }

                DrawText(displayText,
                         50,
                         (int) state.y, state.fontSize,
                         Fade(WHITE, state.alpha));
                break;
            }
            default:
                break;
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

static void ResetGridState(PlayField &playField) {
    playField.moving = false;
    playField.mode = PlayFieldMode::None;
    playField.selectedCharacter = -1;
    playField.selectedTile = {-1, -1};
    playField.path = {};
}

static void DrawPathSelection(SpriteData& spriteData, CharacterData& charData, PlayField &playField, Level &level) {
    // check if mouse is over tile
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), level.camera.camera);
    Vector2 gridPos = PixelToGridPosition(mousePos.x, mousePos.y);
    if (!IsTileOccupied(spriteData, charData, level, static_cast<int>(gridPos.x), static_cast<int>(gridPos.y), -1)) {
        playField.selectedTile = gridPos;
        // calculate a path and draw it as lines
        Path path;
        CharacterStats& stats = charData.stats[level.currentCharacter];
        Vector2i target = PixelToGridPositionI(static_cast<int>(mousePos.x), static_cast<int>(mousePos.y));
        if (CalcPath(spriteData, charData, level, path, PixelToGridPositionI((int) GetCharacterSpritePosX(spriteData, charData.sprite[level.currentCharacter]),
                                                                             (int) GetCharacterSpritePosY(spriteData, charData.sprite[level.currentCharacter])),
                     target, level.currentCharacter, IsTileOccupied)) {
            Color pathColor = Fade(WHITE, playField.highlightAlpha);
            if (path.cost > stats.movePoints) {
                playField.hintText = TextFormat("Not enough movement points (%d)", stats.movePoints);
                pathColor = Fade(RED, playField.highlightAlpha);
                // Draw cross
                DrawLine(gridPos.x * 16, gridPos.y * 16 + 1, gridPos.x * 16 + 15, gridPos.y * 16 + 16, pathColor);
                DrawLine(gridPos.x * 16 + 15, gridPos.y * 16 + 1, gridPos.x * 16, gridPos.y * 16 + 16, pathColor);

            } else {
                playField.hintText = TextFormat("Movement points %d/%d", path.cost, stats.movePoints);
            }
            for (int i = 0; i < path.path.size() - 1; i++) {
                Vector2 start = GridToPixelPosition(path.path[i].x, path.path[i].y);
                Vector2 end = GridToPixelPosition(path.path[i + 1].x, path.path[i + 1].y);
                DrawLineEx(start, end, 1, pathColor);
            }
            int gridX = static_cast<int>(gridPos.x);
            int gridY = static_cast<int>(gridPos.y);
            if (gridX >= 0 && gridX < level.tileMap.width && gridY >= 0 && gridY < level.tileMap.height) {
                DrawRectangleLinesEx(
                        Rectangle{
                                (gridPos.x * 16),
                                (gridPos.y * 16) + 1,
                                15, 15
                        },
                        1, pathColor
                );
            }
            // Check for a mouse click
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && path.cost <= stats.movePoints) {
                playField.mode = PlayFieldMode::None;
                playField.path = path;
                playField.moving = true;
                stats.movePoints -= path.cost;
                // cap at zero
                if (stats.movePoints < 0) {
                    stats.movePoints = 0;
                }
                level.turnState = TurnState::Move;
                PlaySoundEffect(SoundEffectType::Select);
                PlaySoundEffect(SoundEffectType::Footstep);
                StartCameraPanToTargetPos(level.camera, mousePos, 250.0f);
            }
        }
    } else {
        DrawRectangleLinesEx(
                Rectangle{
                        (gridPos.x * 16),
                        (gridPos.y * 16) + 1,
                        15, 15
                },
                1, Fade(RED, playField.highlightAlpha)
        );
        // Draw cross
        DrawLine(gridPos.x * 16, gridPos.y * 16 + 1, gridPos.x * 16 + 15, gridPos.y * 16 + 16,
                 Fade(RED, playField.highlightAlpha));
        DrawLine(gridPos.x * 16 + 15, gridPos.y * 16 + 1, gridPos.x * 16, gridPos.y * 16 + 16,
                 Fade(RED, playField.highlightAlpha));
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
        ResetGridState(playField);
        level.turnState = TurnState::SelectAction;
    }
}

static void DrawSelectCharacters(SpriteData& spriteData, CharacterData& charData, PlayField &playField, std::vector<int> &characters, Color color, Camera2D &camera, bool onlyEnemies) {
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
    Vector2 gridPos = PixelToGridPosition(mousePos.x, mousePos.y);
    for (auto &character: characters) {
        // skip death characters
        if (charData.stats[character].health <= 0) {
            continue;
        }
        if(onlyEnemies && charData.faction[character] == CharacterFaction::Player) {
            continue;
        }
        Vector2 charPos = GetCharacterSpritePos(spriteData, charData.sprite[character]);
        Vector2 gridPosCharacter = PixelToGridPosition(charPos.x, charPos.y);
        if ((int) gridPosCharacter.x == (int) gridPos.x && (int) gridPosCharacter.y == (int) gridPos.y) {
            DrawCircleLines(charPos.x, charPos.y, 10,
                            Fade(color, playField.highlightAlpha));
            // Draw plus
            DrawLine(gridPos.x * 16 + 8, gridPos.y * 16 - 1, gridPos.x * 16 + 8, gridPos.y * 16 + 17,
                     Fade(color, playField.highlightAlpha)); // Vertical line
            DrawLine(gridPos.x * 16 - 1, gridPos.y * 16 + 8, gridPos.x * 16 + 17, gridPos.y * 16 + 8,
                     Fade(color, playField.highlightAlpha)); // Horizontal line
            playField.selectedCharacter = character;
        }
    }
}

static void DrawSelectCharacter(SpriteData& spriteData, CharacterData& charData, PlayField &playField, Level &level, bool onlyEnemies) {
    playField.selectedCharacter = -1;
    DrawSelectCharacters(spriteData, charData, playField, level.allCharacters, RED, level.camera.camera, onlyEnemies);
    if (playField.selectedCharacter != -1) {
        playField.hintText = TextFormat("Selected: %s", charData.name[playField.selectedCharacter].c_str());
        int range = 1;
        if(level.selectedSkill != nullptr) {
            range = level.selectedSkill->range;
        }
        if(range == 1) {
            if (IsCharacterAdjacentToPlayer(spriteData, charData, level.currentCharacter, playField.selectedCharacter)) {
                // draw last line from player to selected character
                Vector2 start = GetCharacterSpritePos(spriteData, charData.sprite[level.currentCharacter]);
                Vector2 end = GetCharacterSpritePos(spriteData, charData.sprite[playField.selectedCharacter]);
                DrawLineEx(start, end, 1, Fade(RED, playField.highlightAlpha));
                // Check for a mouse click
                if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                    PlaySoundEffect(SoundEffectType::Select);
                    level.turnState = TurnState::Waiting;
                    level.waitTime = 0.25f;
                    level.selectedCharacter = playField.selectedCharacter;
                    if (level.selectedSkill == nullptr)
                        level.nextState = TurnState::Attack;
                    else
                        level.nextState = TurnState::UseSkill;
                    ResetGridState(playField);
                }
            } else {
                playField.hintText = "Too far away!";
            }
        } else {
            // draw last line from player to selected character
            Vector2 start = GetCharacterSpritePos(spriteData, charData.sprite[level.currentCharacter]);
            Vector2 end = GetCharacterSpritePos(spriteData, charData.sprite[playField.selectedCharacter]);
            if(HasLineOfSight(level, PixelToGridPositionI((int) start.x, (int) start.y), PixelToGridPositionI((int) end.x, (int) end.y))) {
                int distance = (int) Vector2Distance(start, end);
                if(distance <= range * 16) {
                    DrawLineEx(start, end, 1, Fade(RED, playField.highlightAlpha));
                    // Check for a mouse click
                    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                        PlaySoundEffect(SoundEffectType::Select);
                        level.turnState = TurnState::Waiting;
                        level.waitTime = 0.25f;
                        level.selectedCharacter = playField.selectedCharacter;
                        if (level.selectedSkill == nullptr)
                            level.nextState = TurnState::Attack;
                        else
                            level.nextState = TurnState::UseSkill;
                        ResetGridState(playField);
                    }
                } else {
                    playField.hintText = "Too far away!";

                }
            } else {
                playField.hintText = "No line of sight!";
            }
        }
    } else {
        playField.hintText = "Select a character";
    }
}

static void DrawTargetSelection(SpriteData& spriteData, CharacterData& charData, PlayField &gridState, Level &level, bool onlyEnemies) {
    DrawSelectCharacter(spriteData, charData, gridState, level, onlyEnemies);
    if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
        level.selectedSkill = nullptr;
        level.selectedCharacter = -1;
        ResetGridState(gridState);
        level.turnState = TurnState::SelectAction;
    }
}

static void DrawPathAndSelection(SpriteData& spriteData, CharacterData& charData, PlayField &playField, Level &level) {
    if (playField.mode == PlayFieldMode::SelectingTile) {
        DrawPathSelection(spriteData, charData, playField, level);
    }
    if (playField.mode == PlayFieldMode::SelectingEnemyTarget) {
        DrawTargetSelection(spriteData, charData, playField, level, true);
    }
}

static void DrawTileSelection(PlayField &playField, Level &level) {
    if(playField.selectedTilePos != Vector2i{-1, -1}) {
        Color pathColor = Fade(YELLOW, playField.highlightAlpha);
        Vector2i& gridPos = playField.selectedTilePos;

        if (gridPos.x >= 0 && gridPos.x < level.tileMap.width && gridPos.y >= 0 && gridPos.y < level.tileMap.height) {
            DrawRectangleLinesEx(
                    Rectangle{
                            ((float) gridPos.x * 16),
                            ((float) gridPos.y * 16),
                            16, 16
                    },
                    1, pathColor
            );
        }
    }
}

static void DrawSelectActionHighlight(GameData& data, Level &level, PlayField &playField) {
    // Draw a highlight for the current character if not moving
    if (level.currentCharacter != -1 && (level.turnState == TurnState::SelectAction || level.turnState == TurnState::SelectEnemy)) {
        Vector2 charPos = GetAnimatedCharPos(data, level, level.currentCharacter);
        Color outlineColor = Fade(YELLOW, playField.highlightAlpha);
        DrawRectangleLinesEx(
                Rectangle{
                        charPos.x - 9,
                        charPos.y - 18,
                        17, 24
                },
                1, outlineColor
        );
    }
}

static void RenderActiveCharacterIndicator(GameData& data, float alpha) {
    auto& sprite = data.charData.sprite[data.ui.selectedCharacter];
    Vector2 pos = GetCharacterSpritePos(data.spriteData, sprite);
    if(sprite.bodyPlayer == -1) {
        return;
    }
    SpriteAnimationPlayerRenderData& renderData = data.spriteData.player.renderData[sprite.bodyPlayer];
    int animIdx = data.spriteData.player.animationIdx[sprite.bodyPlayer];
    Vector2 origin = data.spriteData.anim.origin[animIdx];

    Rectangle rect = {pos.x - origin.x + 6, pos.y - origin.y + 7, 19, 25};

    //DrawRectangleLinesEx(rect, 1.0f, ColorAlpha(WHITE, Clamp(alpha - 0.5f, 0.0f, 1.0f)));
    //DrawRectangleLinesEx(rect, 1.0f, ColorAlpha(WHITE, 0.5f));
    //DrawRectangleCorners(rect, YELLOW, 4);
    DrawRectangleCorners(rect, ColorAlpha(YELLOW, alpha), 4);


}

void DrawLevelScreen(GameData& data, Level &level, PlayField &playField) {
    SpriteData& spriteData = data.spriteData;
    CharacterData& charData = data.charData;

    BeginMode2D(level.camera.camera);
    DrawSelectActionHighlight(data, level, playField);
    if(playField.mode == PlayFieldMode::Explore) {
        DrawTileSelection(playField, level);
        RenderActiveCharacterIndicator(data, playField.highlightAlpha);
    } else {
        DrawPathAndSelection(spriteData, charData, playField, level);
    }
    DisplaySpeechBubbleAnimations(level);
    DisplayDamageNumbers(level);
    EndMode2D();


    if (level.turnState == TurnState::SelectAction) {
        DisplayActionUI(data, level, playField, data.smallFont1);
    }
    if (level.turnState == TurnState::Victory) {
        std::string text = "Victory!";
        // Draw the enemy selection UI
        DrawText(text.c_str(), gameScreenHalfWidth - (MeasureText(text.c_str(), 20) / 2), 10, 20, WHITE);
        if (GuiButton((Rectangle) {gameScreenHalfWidthF - 50, 330, 100, 20}, "End Battle")) {
            PublishEndCombatEvent(data.ui.eventQueue, true);
        }
    }
    if (level.turnState == TurnState::Defeat) {
        std::string text = "Defeat!";
        // Draw the enemy selection UI
        DrawText(text.c_str(), gameScreenHalfWidth - (MeasureText(text.c_str(), 20) / 2), 10, 20, WHITE);
        if (GuiButton((Rectangle) {gameScreenHalfWidthF - 50, 330, 100, 20}, "End Battle")) {
            PublishEndCombatEvent(data.ui.eventQueue, false);
        }
    }

    DisplayTextAnimations(level);

    int statsCharId = data.ui.playField.floatingStatsCharacter;
    if (data.state != GameState::DIALOGUE && statsCharId != -1 && (level.turnState == TurnState::None || level.turnState == TurnState::SelectAction ||
                                                        level.turnState == TurnState::SelectEnemy || level.turnState == TurnState::SelectDestination)) {
        float x = GetCharacterSpritePosX(spriteData, charData.sprite[statsCharId]);
        float y = GetCharacterSpritePosY(spriteData, charData.sprite[statsCharId]);
        // to screen space
        Vector2 screenPos = GetWorldToScreen2D(Vector2{x, y}, level.camera.camera);
        DisplayCharacterStatsFloating(charData, statsCharId, (int) screenPos.x - 10, (int) screenPos.y + 12,
                                      IsPlayerCharacter(charData, statsCharId), data.smallFont1);
    }
    // Display hint text
    if(!playField.hintText.empty()) {
        DrawStatusTextBg(playField.hintText.c_str(), WHITE, 318, 5, data.smallFont1);
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

void UpdateLevelScreen(GameData& data, Level &level, float dt) {
    UpdateAnimations(data.spriteData, data.charData, level, dt);
}

void HandleInputLevelScreen(GameData& data, Level &level) {
    // get mouse position
    data.ui.playField.floatingStatsCharacter = -1;
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), level.camera.camera);
    Vector2 gridPos = PixelToGridPosition(mousePos.x, mousePos.y);
    // check if mouse is over character
    for (auto &character: level.allCharacters) {
        // skip dead
        if (data.charData.stats[character].health <= 0) {
            continue;
        }

        // don't show floating stats for characters out of LoS
        if(!HasLineOfSightToParty(data.spriteData, data.charData, level, character))
            continue;

        Vector2 gridPosCharacter = PixelToGridPosition(GetCharacterSpritePosX(data.spriteData, data.charData.sprite[character]),
                                                       GetCharacterSpritePosY(data.spriteData, data.charData.sprite[character]));
        if ((int) gridPosCharacter.x == (int) gridPos.x && (int) gridPosCharacter.y == (int) gridPos.y) {
            data.ui.playField.floatingStatsCharacter = character;
        }
    }
}

