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
#include "LevelCamera.h"
#include "raymath.h"
#include "ui/Icons.h"
#include <cassert>

void CreateLevelScreen(GameData& data) {
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


static void DrawPathSelection(GameData& data, PlayField &playField, Level &level) {
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), level.camera.camera);
    Vector2 gridPos = PixelToGridPosition(mousePos.x, mousePos.y);
    if(data.ui.playField.validMovePath) {
        Path& path = data.ui.playField.movePath;
        Color pathColor = Fade(YELLOW, playField.highlightAlpha);
        DrawIcon(data, gridPos.x * 16, gridPos.y * 16, pathColor, ICON_SQUARE);

        for (int i = 0; i < path.path.size() - 1; i++) {
            Vector2 start = GridToPixelPosition(path.path[i].x, path.path[i].y);
            Vector2 end = GridToPixelPosition(path.path[i + 1].x, path.path[i + 1].y);
            DrawLineEx(start, end, 1, pathColor);
        }
        /*
        EndMode2D();
        DrawToolTip(data.smallFont1, 5, 1, TextFormat("AP: %d/%d", path.cost, stats.AP));
        BeginMode2D(level.camera.camera);
         */
    } else {
        DrawIcon(data, gridPos.x * 16, gridPos.y * 16 + 1, Fade(RED, playField.highlightAlpha), ICON_END_TURN);
    }
}

static void DrawSelectTargetAttackInfo(Font& font, float fontSize, float spacing, AttackInfo& info, const Vector2& pos) {
    std::string text = TextFormat("%.0f%%", info.hitChance);
    Vector2 size = MeasureTextEx(font, text.c_str(), fontSize, spacing);
    size.x = ceilf(size.x); size.y = ceilf(size.y);

    Rectangle rect = {floorf(pos.x), ceilf(pos.y), size.x + 6, size.y + 6};
    rect.x -= ceilf(rect.width/2);
    rect.y -= 24;
    //ClampToScreenBounds(rect);
    DrawRectangleRounded(rect, 0.5f, 4, Color{0, 0, 0, 225});
    //DrawRectangleLinesEx(tipRect, 1, DARKGRAY);
    //DrawRectangleRoundedLinesEx(tipRect, 0.5f, 4, 1, LIGHTGRAY);
    DrawTextEx(font, text.c_str(), {rect.x + 3, rect.y + 3}, fontSize, spacing, WHITE);
}

static void DrawSelectTargetCharacter(GameData& data, PlayField &playField, Level &level) {
    if (playField.selectedCharacter != -1) {
        AttackInfo& info = data.ui.playField.attackInfo;
        int ap = data.charData.stats[data.ui.selectedCharacter].AP;
        Vector2i gridPos = GetCharacterGridPosI(data.spriteData, data.charData.sprite[playField.selectedCharacter]);
        if(ap < info.apCost) {
            DrawIcon(data, gridPos.x * 16, gridPos.y * 16, ColorAlpha(RED, playField.highlightAlpha), ICON_END_TURN);
        } else {
            DrawIcon(data, gridPos.x * 16, gridPos.y * 16, ColorAlpha(YELLOW, playField.highlightAlpha), ICON_ATTACK);
            Vector2 infoPos = GridToPixelPosition(gridPos.x, gridPos.y);
            DrawSelectTargetAttackInfo(data.smallFont1, 5, 1, data.ui.playField.attackInfo, infoPos);
        }
    }
}

static void DrawPathAndSelection(GameData& data, PlayField &playField, Level &level) {
    if(data.state == GameState::PLAY_LEVEL && !data.ui.actionBar.hovered) {
        if (playField.mode == PlayFieldMode::SelectingTile) {
            DrawPathSelection(data, playField, level);
        }
        if (playField.mode == PlayFieldMode::SelectingEnemyTarget) {
            DrawSelectTargetCharacter(data, playField, level);
        }
    }
}

static void DrawTileSelection(GameData& data, PlayField &playField, Level &level) {
    if(playField.selectedTilePos != Vector2i{-1, -1}) {
        Color pathColor = Fade(YELLOW, playField.highlightAlpha);
        Vector2i& gridPos = playField.selectedTilePos;

        if (gridPos.x >= 0 && gridPos.x < level.tileMap.width && gridPos.y >= 0 && gridPos.y < level.tileMap.height) {
            DrawIcon(data, gridPos.x * 16, gridPos.y * 16, ColorAlpha(YELLOW, playField.highlightAlpha), ICON_SQUARE);
        }
    }
}

static void RenderActiveCharacterIndicator(GameData& data, float alpha, int charId) {
    auto& sprite = data.charData.sprite[charId];
    Vector2 pos = GetCharacterSpritePos(data.spriteData, sprite);
    if(sprite.bodyPlayer == -1) {
        return;
    }
    SpriteAnimationPlayerRenderData& renderData = data.spriteData.player.renderData[sprite.bodyPlayer];
    int animIdx = data.spriteData.player.animationIdx[sprite.bodyPlayer];
    Vector2 origin = data.spriteData.anim.origin[animIdx];
    Rectangle rect = {pos.x - origin.x + 6, pos.y - origin.y + 7, 19, 25};
    DrawRectangleCorners(rect, ColorAlpha(YELLOW, alpha), 4);
}

static void DrawSelectActionHighlight(GameData& data, Level &level, PlayField &playField) {
    // Draw a highlight for the current character if not moving
    if (level.currentCharacter != -1 && (level.turnState == TurnState::SelectAction || level.turnState == TurnState::SelectEnemy || level.turnState == TurnState::SelectDestination)) {
        RenderActiveCharacterIndicator(data, playField.highlightAlpha, level.currentCharacter);
    }
}

void DrawLevelScreen(GameData& data, Level &level, PlayField &playField) {
    BeginMode2D(level.camera.camera);
    DrawSelectActionHighlight(data, level, playField);
    if(playField.mode == PlayFieldMode::Explore) {
        DrawTileSelection(data, playField, level);
        RenderActiveCharacterIndicator(data, playField.highlightAlpha, data.ui.selectedCharacter);
    } else {
        DrawPathAndSelection(data, playField, level);
    }
    DisplaySpeechBubbleAnimations(level);
    DisplayDamageNumbers(level);
    EndMode2D();

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

void RenderFloatingStats(GameData& data, Level& level) {
    int statsCharId = data.ui.playField.floatingStatsCharacter;
    if (data.state != GameState::DIALOGUE && statsCharId != -1 && (level.turnState == TurnState::None || level.turnState == TurnState::SelectAction ||
                                                                   level.turnState == TurnState::SelectEnemy || level.turnState == TurnState::SelectDestination)) {
        float x = GetCharacterSpritePosX(data.spriteData, data.charData.sprite[statsCharId]);
        float y = GetCharacterSpritePosY(data.spriteData, data.charData.sprite[statsCharId]);
        // to screen space
        Vector2 screenPos = GetWorldToScreen2D(Vector2{x, y}, level.camera.camera);
        DisplayCharacterStatsFloating(data.charData, statsCharId, (int) screenPos.x - 10, (int) screenPos.y + 12,
                                      IsPlayerCharacter(data.charData, statsCharId), data.smallFont1);
    }
}

void UpdateLevelScreen(GameData& data, Level &level, float dt) {
    UpdateAnimations(data.spriteData, data.charData, level, dt);
}


static void HandleInputPathSelection(GameData& data, PlayField &playField, Level &level) {
    SpriteData& spriteData = data.spriteData;
    CharacterData& charData = data.charData;
    // check if mouse is over tile
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), level.camera.camera);
    Vector2 gridPos = PixelToGridPosition(mousePos.x, mousePos.y);
    data.ui.actionBar.previewApUse = -1;
    data.ui.playField.validMovePath = false;
    if (!IsTileOccupied(spriteData, charData, level, static_cast<int>(gridPos.x), static_cast<int>(gridPos.y), -1)) {
        playField.selectedTile = gridPos;
        // calculate a path and draw it as lines
        Path& path = data.ui.playField.movePath;
        CharacterStats& stats = charData.stats[level.currentCharacter];
        Vector2i target = PixelToGridPositionI(static_cast<int>(mousePos.x), static_cast<int>(mousePos.y));
        if (CalcPath(spriteData, charData, level, path, PixelToGridPositionI((int) GetCharacterSpritePosX(spriteData, charData.sprite[level.currentCharacter]),
                                                                             (int) GetCharacterSpritePosY(spriteData, charData.sprite[level.currentCharacter])),
                     target, level.currentCharacter, IsTileOccupied)) {

            if (path.cost <= stats.AP) {
                data.ui.playField.validMovePath = true;
                data.ui.actionBar.previewApUse = path.cost;
            }
            // Check for a mouse click
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && path.cost <= stats.AP) {
                playField.mode = PlayFieldMode::None;
                playField.path = path;
                playField.moving = true;
                stats.AP -= path.cost;
                // cap at zero
                if (stats.AP < 0) {
                    stats.AP = 0;
                }
                level.turnState = TurnState::Move;
                PlaySoundEffect(SoundEffectType::Select);
                PlaySoundEffect(SoundEffectType::Footstep);
                StartCameraPanToTargetPos(level.camera, mousePos, 250.0f);
            }
        }
    }
}

static void HandleMeleeTargetSelection(GameData& data, Level& level, PlayField& playField, WeaponTemplate* weaponTemplate, int targetId) {
    if (IsCharacterAdjacentToPlayer(data.spriteData, data.charData, data.ui.selectedCharacter, targetId)) {
        playField.selectedCharacter = targetId;
        int weaponItemId = GetSelectedWeaponItemId(data, data.ui.selectedCharacter);
        CalcHitChance(data, data.ui.selectedCharacter, weaponItemId, -1, data.ui.playField.attackInfo);
        data.ui.actionBar.previewApUse = data.ui.playField.attackInfo.apCost;
        int ap = data.charData.stats[data.ui.selectedCharacter].AP;
        // Check for a mouse click
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && ap >= data.ui.playField.attackInfo.apCost) {
            PlaySoundEffect(SoundEffectType::Select);
            level.turnState = TurnState::Waiting;
            level.waitTime = 0.25f;
            level.selectedCharacter = playField.selectedCharacter;
            level.nextState = TurnState::Attack;
            ResetPlayField(playField);
        }
    }
}

static void HandleRangedTargetSelection(GameData& data, Level& level, PlayField& playField, WeaponTemplate* weaponTemplate, int targetId) {
    Vector2i start = GetCharacterGridPosI(data.spriteData, data.charData.sprite[data.ui.selectedCharacter]);
    Vector2i end = GetCharacterGridPosI(data.spriteData, data.charData.sprite[targetId]);
    if(HasLineOfSight(level, start, end, weaponTemplate->range)) {
        int weaponItemId = GetSelectedWeaponItemId(data, data.ui.selectedCharacter);
        CalcHitChance(data, data.ui.selectedCharacter, weaponItemId, data.ui.actionBar.selectedModeIdx, data.ui.playField.attackInfo);
        data.ui.actionBar.previewApUse = data.ui.playField.attackInfo.apCost;
        playField.selectedCharacter = targetId;
    }
}

static void HandleInputTargetSelection(GameData& data, Level& level, PlayField &playField, bool onlyEnemies) {
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), level.camera.camera);
    Vector2i gridPos = PixelToGridPositionI(mousePos.x, mousePos.y);
    playField.selectedCharacter = -1;
    data.ui.actionBar.previewApUse = -1;
    for (auto &character: level.allCharacters) {
        // skip death characters
        if (data.charData.stats[character].HP <= 0) {
            continue;
        }
        if(onlyEnemies && data.charData.faction[character] == CharacterFaction::Player) {
            continue;
        }
        Vector2i charPos = GetCharacterGridPosI(data.spriteData, data.charData.sprite[character]);
        if (charPos.x == gridPos.x && charPos.y == gridPos.y) {
            WeaponTemplate* weaponTemplate = GetSelectedWeaponTemplate(data, data.ui.selectedCharacter);
            // Treat unarmed as melee
            if(weaponTemplate == nullptr) {
                HandleMeleeTargetSelection(data, level, playField, weaponTemplate, character);
            }
            else {
                switch (weaponTemplate->type) {
                    case WeaponType::Melee:
                        HandleMeleeTargetSelection(data, level, playField, weaponTemplate, character);
                        break;
                    case WeaponType::Ranged:
                        HandleRangedTargetSelection(data, level, playField, weaponTemplate, character);
                        break;
                }
            }
        }
    }
}

void HandleInputLevelScreen(GameData& data, Level &level, PlayField &playField) {
    // get mouse position
    data.ui.playField.floatingStatsCharacter = -1;
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), level.camera.camera);
    Vector2 gridPos = PixelToGridPosition(mousePos.x, mousePos.y);
    // check if mouse is over character
    for (auto &character: level.allCharacters) {
        // skip dead
        if (data.charData.stats[character].HP <= 0) {
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

    if(data.state == GameState::PLAY_LEVEL) {
        if (playField.mode == PlayFieldMode::SelectingTile) {
            HandleInputPathSelection(data, playField, level);
        }
        if (playField.mode == PlayFieldMode::SelectingEnemyTarget) {
            HandleInputTargetSelection(data, level, playField, true);
        }
    }
}

