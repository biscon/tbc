//
// Created by bison on 20-11-25.
//

#include "LevelRenderer.h"


#include "character/Character.h"
#include "ui/UI.h"
#include "Combat.h"
#include "ai/PathFinding.h"
#include "raymath.h"
#include "ui/Icons.h"
#include "audio/Sound.h"
#include "graphics/Lighting.h"
#include "graphics/BloodPool.h"
#include "Weather.h"
#include <cassert>

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
            if(animation.state.damageNumber.initialDelay > 0) {
                continue;
            }
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


static void DrawPathSelection(GameData& data, LevelSystemData &playField, Level &level) {
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), level.camera.camera);
    Vector2 gridPos = PixelToGridPosition(mousePos.x, mousePos.y);
    if(data.ui.level.validMovePath) {
        Path& path = data.ui.level.movePath;
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

static void DrawSelectTargetCharacter(GameData& data, LevelSystemData &playField, Level &level) {
    if (playField.selectedCharacter != -1) {
        AttackInfo& info = data.ui.level.attackInfo;
        int ap = data.charData.stats[data.ui.selectedCharacter].AP;
        Vector2i gridPos = GetCharacterGridPosI(data.spriteData, data.charData.sprite[playField.selectedCharacter]);
        if(ap < info.apCost) {
            DrawIcon(data, gridPos.x * 16, gridPos.y * 16, ColorAlpha(RED, playField.highlightAlpha), ICON_END_TURN);
        } else {
            DrawIcon(data, gridPos.x * 16, gridPos.y * 16, ColorAlpha(YELLOW, playField.highlightAlpha), ICON_ATTACK);
            Vector2 infoPos = GridToPixelPosition(gridPos.x, gridPos.y);
            DrawSelectTargetAttackInfo(data.smallFont1, 5, 1, data.ui.level.attackInfo, infoPos);
        }
    }
}

static void DrawPathAndSelection(GameData& data, LevelSystemData &playField, Level &level) {
    if(data.state == GameState::PLAY_LEVEL && !data.ui.actionBar.hovered) {
        if (playField.mode == LevelMode::SelectingTile) {
            DrawPathSelection(data, playField, level);
        }
        if (playField.mode == LevelMode::SelectingEnemyTarget) {
            DrawSelectTargetCharacter(data, playField, level);
        }
    }
}

static void DrawTileSelection(GameData& data, LevelSystemData &playField, Level &level) {
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
    int animIdx = data.spriteData.player.animationIdx[sprite.bodyPlayer];
    Vector2 origin = data.spriteData.anim.origin[animIdx];
    Rectangle rect = {pos.x - origin.x + 6, pos.y - origin.y + 7, 19, 25};
    DrawRectangleCorners(rect, ColorAlpha(YELLOW, alpha), 4);
}

static void DrawSelectActionHighlight(GameData& data, Level &level, LevelSystemData &playField) {
    // Draw a highlight for the current character if not moving
    if (level.currentCharacter != -1 && (level.turnState == TurnState::SelectAction || level.turnState == TurnState::SelectEnemy || level.turnState == TurnState::SelectDestination)) {
        RenderActiveCharacterIndicator(data, playField.highlightAlpha, level.currentCharacter);
    }
}

void RenderLevelUi(GameData& data, Level &level) {
    BeginMode2D(level.camera.camera);
    DrawSelectActionHighlight(data, level, data.levelData);
    if(data.levelData.mode == LevelMode::Explore) {
        DrawTileSelection(data, data.levelData, level);
        RenderActiveCharacterIndicator(data, data.levelData.highlightAlpha, data.ui.selectedCharacter);
    } else {
        DrawPathAndSelection(data, data.levelData, level);
    }
    DisplaySpeechBubbleAnimations(level);
    DisplayDamageNumbers(level);
    EndMode2D();

    DisplayTextAnimations(level);


    // Display hint text
    if(!data.levelData.hintText.empty()) {
        DrawStatusTextBg(data.levelData.hintText.c_str(), WHITE, 318, 5, data.smallFont1);
        data.levelData.hintText = "";
    }
}

void RenderFloatingStats(GameData& data, Level& level) {
    int statsCharId = data.ui.level.floatingStatsCharacter;
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


Vector2 GetAnimatedCharPos(GameData& data, Level &level, int character) {
    // Check if the character is visible (not blinking)
    for (auto &animation: level.animations) {
        if (animation.type == AnimationType::Attack) {
            if (animation.state.attack.attacker == character) {
                return {animation.state.attack.currentX, animation.state.attack.currentY};
            }
        }
    }
    return GetCharacterSpritePos(data.spriteData, data.charData.sprite[character]);
}

// Function to draw the health bar
void DrawHealthBar(float x, float y, float width, float health, float maxHealth) {
    // Draw the health bar background (gray)
    DrawRectangle(x, y, width, 2, GRAY);
    // Draw the health bar foreground (green for positive health, red for baseAttack)
    DrawRectangle(x, y, width * (health / maxHealth), 2, GREEN);
}

static void DrawGridCharacters(GameData& data, Level &level, LevelSystemData& playField) {
    SpriteData& spriteData = data.spriteData;
    CharacterData& charData = data.charData;
    // Sort characters by y position
    std::vector<int> sortedCharacters;
    for (auto &character: level.allCharacters) {
        sortedCharacters.push_back(character);
    }
    std::sort(sortedCharacters.begin(), sortedCharacters.end(), [&data, &level](int a, int b) {
        return GetAnimatedCharPos(data, level, a).y < GetAnimatedCharPos(data, level, b).y;
    });


    // Draw characters
    for (auto &character: sortedCharacters) {
        CharacterSprite& charSprite = charData.sprite[character];
        auto gridPos = GetCharacterGridPosI(data.spriteData, charSprite);
        if(!HasLineOfSightToPartyLight(spriteData, charData, level, gridPos))
            continue;
        Vector2 charPos = GetAnimatedCharPos(data, level, character);
        // Draw oval shadow underneath
        if(charData.stats[character].HP > 0)
            DrawEllipse((int) charPos.x, (int) charPos.y, 6, 4, Fade(BLACK, 0.25f));


        if (IsCharacterVisible(level, character)) {
            Vector2i t = GetCharacterGridPosI(spriteData, charSprite);
            Color v1 = GetVertexLight(level.lighting, level.tileMap, t.x, t.y);     // top-left corner
            Color v2 = GetVertexLight(level.lighting, level.tileMap, t.x+1, t.y);   // top-right
            Color v3 = GetVertexLight(level.lighting, level.tileMap, t.x+1, t.y+1); // bottom-right
            Color v4 = GetVertexLight(level.lighting, level.tileMap, t.x, t.y+1);   // bottom-left

            DrawCharacterSpriteColors(spriteData, charSprite, charPos.x, charPos.y, v1, v2, v3, v4);
        } else {
            SetCharacterSpriteTint(spriteData, charSprite, {255, 255, 255, 64});
            DrawCharacterSprite(spriteData, charSprite, charPos.x, charPos.y);
            SetCharacterSpriteTint(spriteData, charSprite, WHITE); // Reset tint
        }
        CharacterStats& stats = charData.stats[character];
        // Draw health bar
        if(playField.mode != LevelMode::Explore) {
            if (stats.HP > 0 && level.turnState != TurnState::None) {
                DrawHealthBar(charPos.x - 8, charPos.y - 21, 15, (float) stats.HP, (float) CalculateCharHealth(stats));
            } else if (std::count(level.partyCharacters.begin(), level.partyCharacters.end(), character)) {
                DrawHealthBar(charPos.x - 8, charPos.y - 21, 15, (float) stats.HP, (float) CalculateCharHealth(stats));
            }
        }
    }
}

static void DrawSampleCorners(SpriteData& spriteData, Level &level, int animPlayer, Vector2i gridPos) {
    auto frameInfo = GetFrameInfo(spriteData, animPlayer);
    Vector2 pos = GridToPixelPosition(gridPos.x, gridPos.y);
    Vector2 drawPos = pos;
    pos.x -= 8.0f;
    pos.y -= 8.0f;

    Vector2 topLeft = pos;
    Vector2 topRight = {pos.x + frameInfo.srcRect.width, pos.y};
    Vector2 bottomRight = {pos.x + frameInfo.srcRect.width, pos.y + frameInfo.srcRect.height};
    Vector2 bottomLeft = {pos.x, pos.y + frameInfo.srcRect.height};
    Vector2i t1 = PixelToGridPositionI(topLeft.x, topLeft.y);
    Vector2i t2 = PixelToGridPositionI(topRight.x, topRight.y);
    Vector2i t3 = PixelToGridPositionI(bottomRight.x, bottomRight.y);
    Vector2i t4 = PixelToGridPositionI(bottomLeft.x, bottomLeft.y);

    Color v1 = GetVertexLight(level.lighting, level.tileMap, t1.x, t1.y);     // top-left corner
    Color v2 = GetVertexLight(level.lighting, level.tileMap, t2.x, t2.y);   // top-right
    Color v3 = GetVertexLight(level.lighting, level.tileMap, t3.x, t3.y); // bottom-right
    Color v4 = GetVertexLight(level.lighting, level.tileMap, t4.x, t4.y);   // bottom-left
    DrawSpriteAnimationColors(spriteData, animPlayer, drawPos.x - 8.0f, drawPos.y - 8.0f, v1, v2, v3, v4);
}


// should sample from tile under middle of object
static void DrawLevelObjects(SpriteData& spriteData, Level &level) {
    for(auto& entry : level.objects) {
        auto& obj = entry.second;
        Vector2 pos = GridToPixelPosition(obj.gridPos.x, obj.gridPos.y);
        if(obj.lit) {
            auto frameInfo = GetFrameInfo(spriteData, obj.animPlayer);
            if(frameInfo.srcRect.width > 16 || frameInfo.srcRect.height > 16) {
                DrawSampleCorners(spriteData, level, obj.animPlayer, obj.gridPos);
            } else {
                Vector2i &t = obj.gridPos;
                // NOTE: should really sample at all 4 corners if sprite is bigger than a tile
                Color v1 = GetVertexLight(level.lighting, level.tileMap, t.x, t.y);     // top-left corner
                Color v2 = GetVertexLight(level.lighting, level.tileMap, t.x + 1, t.y);   // top-right
                Color v3 = GetVertexLight(level.lighting, level.tileMap, t.x + 1, t.y + 1); // bottom-right
                Color v4 = GetVertexLight(level.lighting, level.tileMap, t.x, t.y + 1);   // bottom-left
                DrawSpriteAnimationColors(spriteData, obj.animPlayer, pos.x - 8.0f, pos.y - 8.0f, v1, v2, v3, v4);
            }
        } else {
            DrawSpriteAnimation(spriteData, obj.animPlayer, pos.x - 8.0f, pos.y - 8.0f);
        }

    }
}

// should sample from tile under middle of door
static void DrawDoors(SpriteData& spriteData, Level &level) {
    for(auto& entry : level.doors) {
        auto& door = entry.second;

        DrawSampleCorners(spriteData, level, door.animPlayer, door.gridPos);


    }
}

void RenderLevel(GameData& data, Level &level) {
    // Back layers
    BeginMode2D(level.camera.camera);
    DrawLayers(data, level.lighting, data.spriteData.sheet, level.tileMap, level.tileMap.backLayers, 0, 0);
    EndMode2D();

    DrawBloodPools();

    // Doors and level objects
    BeginMode2D(level.camera.camera);
    DrawLevelObjects(data.spriteData, level);
    DrawDoors(data.spriteData, level);
    EndMode2D();

    // Characters
    BeginMode2D(level.camera.camera);
    DrawGridCharacters(data, level, data.levelData);

    //DrawWeather(level.weather, {200, 200, 255, 120});
    EndMode2D();

    DrawParticleManager(data.particleManager);

    // Front layers
    BeginMode2D(level.camera.camera);
    DrawLayers(data, level.lighting, data.spriteData.sheet, level.tileMap, level.tileMap.frontLayers, 0, 0);
    if(level.outdoor) {
        DrawWeather(data.weatherData, level.lighting.ambient);
    } else {
        RenderVisibilityMap(level.lighting);
    }
    EndMode2D();
}

