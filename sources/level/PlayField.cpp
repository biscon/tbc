//
// Created by bison on 16-01-25.
//

#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_set>
#include "PlayField.h"
#include "raylib.h"
#include "ui/UI.h"
#include "raymath.h"
#include "graphics/BloodPool.h"
#include "graphics/ParticleSystem.h"
#include "audio/SoundEffect.h"
#include "level/Combat.h"
#include "graphics/TileMap.h"
#include "ai/PathFinding.h"

static bool IsCharacterVisible(Level &level, int character) {
    // Check if the character is visible (not blinking)
    for (auto &animation: level.animations) {
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

static Vector2 GetAnimatedCharPos(SpriteData& spriteData, CharacterData& charData, Level &level, int character) {
    // Check if the character is visible (not blinking)
    for (auto &animation: level.animations) {
        if (animation.type == AnimationType::Attack) {
            if (animation.state.attack.attacker == character) {
                return {animation.state.attack.currentX, animation.state.attack.currentY};
            }
        }
    }
    return GetCharacterSpritePos(spriteData, charData.sprite[character]);
}

void CreatePlayField(PlayField &playField, ParticleManager* particleManager, GameEventQueue* eventQueue) {
    playField.particleManager = particleManager;
    playField.moving = false;
    playField.mode = PlayFieldMode::None;
    playField.selectedCharacter = -1;
    playField.selectedTile = {-1, -1};
    playField.path = {};
    playField.eventQueue = eventQueue;
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

// Function to draw the health bar
void DrawHealthBar(float x, float y, float width, float health, float maxHealth) {
    // Draw the health bar background (gray)
    DrawRectangle(x, y, width, 2, GRAY);
    // Draw the health bar foreground (green for positive health, red for baseAttack)
    DrawRectangle(x, y, width * (health / maxHealth), 2, GREEN);
}


static void DrawGridCharacters(SpriteData& spriteData, CharacterData& charData, PlayField &playField, Level &level) {
    // Sort characters by y position
    std::vector<int> sortedCharacters;
    for (auto &character: level.allCharacters) {
        sortedCharacters.push_back(character);
    }
    std::sort(sortedCharacters.begin(), sortedCharacters.end(), [&spriteData, &charData, &level](int a, int b) {
        return GetAnimatedCharPos(spriteData, charData, level, a).y < GetAnimatedCharPos(spriteData, charData, level, b).y;
    });


    // Draw a highlight for the current character if not moving
    if (level.currentCharacter != -1 && (level.turnState == TurnState::SelectAction || level.turnState == TurnState::SelectEnemy)) {
        Vector2 charPos = GetAnimatedCharPos(spriteData, charData, level, level.currentCharacter);
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

    // Draw characters
    for (auto &character: sortedCharacters) {
        Vector2 charPos = GetAnimatedCharPos(spriteData, charData, level, character);
        // Draw oval shadow underneath
        if(charData.stats[character].health > 0 && level.turnState != TurnState::Victory && level.turnState != TurnState::Defeat)
            DrawEllipse((int) charPos.x, (int) charPos.y, 6, 4, Fade(BLACK, 0.25f));

        CharacterSprite& charSprite = charData.sprite[character];
        if (IsCharacterVisible(level, character)) {
            DrawCharacterSprite(spriteData, charSprite, charPos.x, charPos.y);
        } else {
            SetCharacterSpriteTint(spriteData, charSprite, {255, 255, 255, 64});
            DrawCharacterSprite(spriteData, charSprite, charPos.x, charPos.y);
            SetCharacterSpriteTint(spriteData, charSprite, WHITE); // Reset tint
        }
        CharacterStats& stats = charData.stats[character];
        // Draw health bar
        if(stats.health > 0 && level.turnState != TurnState::Victory && level.turnState != TurnState::Defeat)
            DrawHealthBar(charPos.x - 8, charPos.y - 21, 15, (float) stats.health, (float) stats.maxHealth);
    }
}

/*
static void DrawGridLines() {
    // draw a line grid of 30x13 16x16 tiles taking up a rectangle of 480x208
    for (int i = 0; i < GRID_WIDTH + 1; i++) {
        DrawLine(16 * i, 0, 16 * i, 208, Fade(BLACK, 0.15));
    }
    // draw vertical lines
    for (int i = 0; i < GRID_HEIGHT + 1; i++) {
        DrawLine(0, 16 * i, 480, 16 * i, Fade(BLACK, 0.15));
    }
}
*/

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

static void updateTurnBasedMove(SpriteData& spriteData, CharacterData& charData, PlayField &playField, Level &level, float dt) {
    if (playField.moving) {
        playField.path.moveTime += dt;

        // Calculate the percentage of completion for the current step
        float t = playField.path.moveTime / playField.path.moveSpeed;

        if (playField.path.currentStep < playField.path.path.size() - 1) {
            // Get the current and next waypoint positions
            Vector2 start = GridToPixelPosition(
                    playField.path.path[playField.path.currentStep].x,
                    playField.path.path[playField.path.currentStep].y);
            Vector2 end = GridToPixelPosition(
                    playField.path.path[playField.path.currentStep + 1].x,
                    playField.path.path[playField.path.currentStep + 1].y);

            CharacterSprite& sprite = charData.sprite[level.currentCharacter];
            // Lerp the x and y components separately
            SetCharacterSpritePosX(spriteData, sprite, Lerp(start.x, end.x, t));
            SetCharacterSpritePosY(spriteData, sprite, Lerp(start.y, end.y, t));

            // Determine the direction of movement and set the appropriate animation
            if (fabs(end.x - start.x) > fabs(end.y - start.y)) {
                // Horizontal movement
                if (end.x > start.x) {
                    PlayCharacterSpriteAnim(spriteData, sprite, SpriteAnimationType::WalkRight, true);
                    charData.orientation[level.currentCharacter] = Orientation::Right;
                } else {
                    PlayCharacterSpriteAnim(spriteData, sprite, SpriteAnimationType::WalkLeft, true);
                    charData.orientation[level.currentCharacter] = Orientation::Left;
                }
            } else {
                // Vertical movement
                if (end.y > start.y) {
                    PlayCharacterSpriteAnim(spriteData, sprite, SpriteAnimationType::WalkDown, true);
                    charData.orientation[level.currentCharacter] = Orientation::Down;
                } else {
                    PlayCharacterSpriteAnim(spriteData, sprite, SpriteAnimationType::WalkUp, true);
                    charData.orientation[level.currentCharacter] = Orientation::Up;
                }
            }

            // Check if we have completed the current step
            if (playField.path.moveTime >= playField.path.moveSpeed) {
                playField.path.moveTime = 0.0f;
                playField.path.currentStep++;

                // If the last step is reached, stop moving
                if (playField.path.currentStep >= playField.path.path.size() - 1) {
                    StopSoundEffect(SoundEffectType::Footstep);
                    playField.moving = false;
                    PauseCharacterSpriteAnim(spriteData, sprite);

                    SetCharacterSpriteFrame(spriteData, sprite, 0);
                    // set final position
                    auto finalPos = playField.path.path[playField.path.path.size() - 1];
                    SetCharacterSpritePos(spriteData, sprite, GridToPixelPosition(finalPos.x, finalPos.y));

                    if (IsPlayerCharacter(charData, level.currentCharacter)) {
                        level.turnState = TurnState::SelectAction;
                    } else {
                        level.turnState = TurnState::EnemyTurn;
                    }
                }
            }
        }
    }
}

static void updateActiveMovement(SpriteData& spriteData, CharacterData& charData, PlayField &playField, float dt) {
    for(auto& move : playField.activeMoves) {
        move.path.moveTime += dt;

        // Calculate the percentage of completion for the current step
        float t = move.path.moveTime / move.path.moveSpeed;

        if (move.path.currentStep < move.path.path.size() - 1) {
            // Get the current and next waypoint positions
            Vector2 start = GridToPixelPosition(
                    move.path.path[move.path.currentStep].x,
                    move.path.path[move.path.currentStep].y);
            Vector2 end = GridToPixelPosition(
                    move.path.path[move.path.currentStep + 1].x,
                    move.path.path[move.path.currentStep + 1].y);

            CharacterSprite& sprite = charData.sprite[move.character];
            // Lerp the x and y components separately
            SetCharacterSpritePosX(spriteData, sprite, Lerp(start.x, end.x, t));
            SetCharacterSpritePosY(spriteData, sprite, Lerp(start.y, end.y, t));

            // Determine the direction of movement and set the appropriate animation
            if (fabs(end.x - start.x) > fabs(end.y - start.y)) {
                // Horizontal movement
                if (end.x > start.x) {
                    PlayCharacterSpriteAnim(spriteData, sprite, SpriteAnimationType::WalkRight, true);
                    charData.orientation[move.character] = Orientation::Right;
                } else {
                    PlayCharacterSpriteAnim(spriteData, sprite, SpriteAnimationType::WalkLeft, true);
                    charData.orientation[move.character] = Orientation::Left;
                }
            } else {
                // Vertical movement
                if (end.y > start.y) {
                    PlayCharacterSpriteAnim(spriteData, sprite, SpriteAnimationType::WalkDown, true);
                    charData.orientation[move.character] = Orientation::Down;
                } else {
                    PlayCharacterSpriteAnim(spriteData, sprite, SpriteAnimationType::WalkUp, true);
                    charData.orientation[move.character] = Orientation::Up;
                }
            }

            // Check if we have completed the current step
            if (move.path.moveTime >= move.path.moveSpeed) {
                move.path.moveTime = 0.0f;
                move.path.currentStep++;

                // If the last step is reached, stop moving
                if (move.path.currentStep >= move.path.path.size() - 1) {
                    //StopSoundEffect(SoundEffectType::Footstep);
                    PauseCharacterSpriteAnim(spriteData, sprite);
                    SetCharacterSpriteFrame(spriteData, sprite, 0);
                    // set final position
                    auto finalPos = move.path.path[move.path.path.size() - 1];
                    SetCharacterSpritePos(spriteData, sprite, GridToPixelPosition(finalPos.x, finalPos.y));
                    move.isDone = true;
                    TraceLog(LOG_INFO, "Move done");
                }
            }
        }
    }
    // Use erase-remove idiom to remove animations which are done
    playField.activeMoves.erase(
            std::remove_if(playField.activeMoves.begin(), playField.activeMoves.end(),
                           [](const CharacterMove& m) {
                                if(m.isDone)
                                    TraceLog(LOG_INFO, "Removing move from active list");
                               return m.isDone;
                           }),
            playField.activeMoves.end()
    );
}

static void checkIfPartySpotted(SpriteData& spriteData, CharacterData& charData, PlayField &playField, Level &level) {
    for(auto& c : level.allCharacters) {
        if(charData.faction[c] != CharacterFaction::Enemy || charData.stats[c].health <= 0) {
            continue;
        }
        Vector2i enemyGridPos = GetCharacterGridPosI(spriteData, charData.sprite[c]);
        for(auto& partyChar : level.partyCharacters) {
            Vector2i partyGridPos = GetCharacterGridPosI(spriteData, charData.sprite[partyChar]);
            if(HasLineOfSight(level, enemyGridPos, partyGridPos, 5)) {
                TraceLog(LOG_INFO, "Party last spotted by %s", charData.name[c].c_str());
                PublishPartySpottedEvent(*playField.eventQueue, c);
                return;
            }
        }
    }
}

static void checkLevelExits(SpriteData& spriteData, CharacterData& charData, PlayField &playField, Level &level) {
    for(auto& exit : level.exits) {
        for(auto& c : level.partyCharacters) {
            Vector2i cGridPos = GetCharacterGridPosI(spriteData, charData.sprite[c]);
            if(exit.x == cGridPos.x && exit.y == cGridPos.y) {
                PublishExitLevelEvent(*playField.eventQueue, exit.levelFile, exit.spawnPoint);
            }
        }
    }
}

void UpdatePlayField(SpriteData& spriteData, CharacterData& charData, PlayField &playField, Level &level, float dt) {
    // Update the pulsing alpha
    if (playField.increasing) {
        playField.highlightAlpha = Lerp(playField.highlightAlpha, 1.0f, dt * playField.pulseSpeed);
        if (playField.highlightAlpha >= 0.99f) {
            playField.increasing = false;
        }
    } else {
        playField.highlightAlpha = Lerp(playField.highlightAlpha, 0.25f, dt * playField.pulseSpeed);
        if (playField.highlightAlpha <= 0.26f) {
            playField.increasing = true;
        }
    }
    updateActiveMovement(spriteData, charData, playField, dt);
    updateTurnBasedMove(spriteData, charData, playField, level, dt);

    // Update animations for all characters
    for (auto &character: level.allCharacters) {
        UpdateCharacterSprite(spriteData, charData.sprite[character], dt);
    }
    if(level.turnState == TurnState::None) {
        checkIfPartySpotted(spriteData, charData, playField, level);
        checkLevelExits(spriteData, charData, playField, level);
    }
}

static void handleInputPlayFieldNormal(PlayField &playField, Level &level) {

}

static void handleInputPlayFieldSelectingTile(PlayField &playField, Level &level) {

}

static void handleInputPlayFieldSelectingEnemyTarget(PlayField &playField, Level &level) {

}

static void handleInputPlayFieldShowMove(SpriteData& spriteData, CharacterData& charData, PlayField &playField, Level &level) {
    // check if mouse is over tile
    playField.selectedTilePos = {-1, -1};
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), level.camera.camera);
    Vector2i gridPos = PixelToGridPositionI(mousePos.x, mousePos.y);
    if (!IsTileOccupied(spriteData, charData, level, gridPos.x, gridPos.y, -1)) {
        // calculate a path and draw it as lines
        Path path;
        Vector2i target = PixelToGridPositionI(static_cast<int>(mousePos.x), static_cast<int>(mousePos.y));
        auto& playerChar = level.partyCharacters.front();
        if (CalcPath(spriteData, charData, level, path, PixelToGridPositionI((int) GetCharacterSpritePosX(spriteData, charData.sprite[playerChar]),
                                                       (int) GetCharacterSpritePosY(spriteData, charData.sprite[playerChar])),
                     target, playerChar, IsTileOccupiedEnemies)) {
            playField.selectedTilePos = gridPos;
            if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                PublishMovePartyEvent(*playField.eventQueue, gridPos);
            }
        }
    }
}

void HandleInputPlayField(SpriteData& spriteData, CharacterData& charData, PlayField &playField, Level &level) {
    switch(playField.mode) {
        case PlayFieldMode::None:handleInputPlayFieldNormal(playField, level);break;
        case PlayFieldMode::SelectingTile:handleInputPlayFieldSelectingTile(playField, level);break;
        case PlayFieldMode::SelectingEnemyTarget:handleInputPlayFieldSelectingEnemyTarget(playField, level);break;
        case PlayFieldMode::Move:handleInputPlayFieldShowMove(spriteData, charData, playField, level);break;
    }
}

void DrawPlayField(SpriteData& spriteData, CharacterData& charData, PlayField &playField, Level &level) {
    // Bottom layer
    BeginMode2D(level.camera.camera);
    DrawTileLayer(spriteData.sheet, level.tileMap, BOTTOM_LAYER, 0, 0);
    EndMode2D();

    DrawBloodPools();
    // Middle layer
    BeginMode2D(level.camera.camera);
    DrawTileLayer(spriteData.sheet, level.tileMap, MIDDLE_LAYER, 0, 0);

    if(playField.mode == PlayFieldMode::Move) {
        DrawTileSelection(playField, level);
    } else {
        DrawPathAndSelection(spriteData, charData, playField, level);
    }

    DrawGridCharacters(spriteData, charData, playField, level);
    EndMode2D();
    DrawParticleManager(*playField.particleManager);

    // Top layer
    BeginMode2D(level.camera.camera);
    DrawTileLayer(spriteData.sheet, level.tileMap, TOP_LAYER, 0, 0);
    EndMode2D();
}

void MoveCharacter(SpriteData& spriteData, CharacterData& charData, PlayField &playField, Level &level, int character, Vector2i target) {
    // calculate a path and draw it as lines
    Path path;
    Vector2i cCharPos = GetCharacterSpritePosI(spriteData, charData.sprite[character]);
    Vector2i cGridPos = PixelToGridPositionI(cCharPos.x, cCharPos.y);
    if (CalcPath(spriteData, charData, level, path, cGridPos, target, character, IsTileOccupiedEnemies)) {
        CharacterMove move;
        move.character = character;
        move.path = path;
        move.isDone = false;
        playField.activeMoves.push_back(move);
    } else {
        TraceLog(LOG_WARNING, "No path found");
    }
}

void MoveCharacterPartial(SpriteData& spriteData, CharacterData& charData, PlayField &playField, Level &level, int character, Vector2i target) {
    // calculate a path and draw it as lines
    Path path;
    Vector2i cCharPos = GetCharacterSpritePosI(spriteData, charData.sprite[character]);
    Vector2i cGridPos = PixelToGridPositionI(cCharPos.x, cCharPos.y);
    CalcPathWithRangePartial(spriteData, charData, level, path, cGridPos, target, 1, character, IsTileOccupiedEnemies);
    if(!path.path.empty()) {
        CharacterMove move;
        move.character = character;
        move.path = path;
        move.isDone = false;
        playField.activeMoves.push_back(move);
    } else {
        TraceLog(LOG_WARNING, "No path found");
    }
}

void ResetPlayField(PlayField &playField) {
    playField.selectedCharacter = -1;
    playField.activeMoves.clear();
    playField.moving = false;
    playField.mode = PlayFieldMode::None;
    playField.selectedCharacter = -1;
    playField.selectedTile = {-1, -1};
    playField.path = {};
}
