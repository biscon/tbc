//
// Created by bison on 16-01-25.
//

#include <algorithm>
#include <cmath>
#include "PlayField.h"
#include "raylib.h"
#include "ui/UI.h"
#include "raymath.h"
#include "graphics/BloodPool.h"
#include "graphics/ParticleSystem.h"
#include "audio/SoundEffect.h"
#include "level/Combat.h"

static bool IsCharacterVisible(Level &level, Character *character) {
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

static Vector2 GetAnimatedCharPos(Level &level, Character *character) {
    // Check if the character is visible (not blinking)
    for (auto &animation: level.animations) {
        if (animation.type == AnimationType::Attack) {
            if (animation.state.attack.attacker == character) {
                return {animation.state.attack.currentX, animation.state.attack.currentY};
            }
        }
    }
    return GetCharacterSpritePos(character->sprite);
}

void CreatePlayField(PlayField &playField, ParticleManager* particleManager, GameEventQueue* eventQueue) {
    playField.particleManager = particleManager;
    playField.moving = false;
    playField.mode = PlayFieldMode::Normal;
    playField.selectedCharacter = nullptr;
    playField.selectedTile = {-1, -1};
    playField.path = {};
    playField.floatingStatsCharacter = nullptr;
    playField.eventQueue = eventQueue;
}

static void ResetGridState(PlayField &playField) {
    playField.moving = false;
    playField.mode = PlayFieldMode::Normal;
    playField.selectedCharacter = nullptr;
    playField.selectedTile = {-1, -1};
    playField.path = {};
}

void DrawPathSelection(PlayField &playField, Level &level) {
    // check if mouse is over tile
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), level.camera.camera);
    Vector2 gridPos = PixelToGridPosition(mousePos.x, mousePos.y);
    if (!IsTileOccupied(level, static_cast<int>(gridPos.x), static_cast<int>(gridPos.y), nullptr)) {
        playField.selectedTile = gridPos;
        // calculate a path and draw it as lines
        Path path;
        Vector2i target = PixelToGridPositionI(static_cast<int>(mousePos.x), static_cast<int>(mousePos.y));
        if (CalcPath(level, path, PixelToGridPositionI((int) GetCharacterSpritePosX(level.currentCharacter->sprite),
                                                       (int) GetCharacterSpritePosY(level.currentCharacter->sprite)),
                     target, level.currentCharacter, IsTileOccupied)) {
            Color pathColor = Fade(WHITE, playField.highlightAlpha);
            if (path.cost > level.currentCharacter->movePoints) {
                DrawStatusText(TextFormat("Not enough movement points (%d)", level.currentCharacter->movePoints),
                               WHITE, 220, 10);
                pathColor = Fade(RED, playField.highlightAlpha);
                // Draw cross
                DrawLine(gridPos.x * 16, gridPos.y * 16 + 1, gridPos.x * 16 + 15, gridPos.y * 16 + 16, pathColor);
                DrawLine(gridPos.x * 16 + 15, gridPos.y * 16 + 1, gridPos.x * 16, gridPos.y * 16 + 16, pathColor);

            } else {
                DrawStatusText(TextFormat("Movement points %d/%d", path.cost, level.currentCharacter->movePoints),
                               WHITE, 220, 10);
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
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && path.cost <= level.currentCharacter->movePoints) {
                playField.mode = PlayFieldMode::Normal;
                playField.path = path;
                playField.moving = true;
                level.currentCharacter->movePoints -= path.cost;
                // cap at zero
                if (level.currentCharacter->movePoints < 0) {
                    level.currentCharacter->movePoints = 0;
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

void DrawSelectCharacters(PlayField &playField, std::vector<Character*> &characters, Color color, Camera2D &camera, bool onlyEnemies) {
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
    Vector2 gridPos = PixelToGridPosition(mousePos.x, mousePos.y);
    for (auto &character: characters) {
        // skip death characters
        if (character->health <= 0) {
            continue;
        }
        if(onlyEnemies && character->faction == CharacterFaction::Player) {
            continue;
        }
        Vector2 charPos = GetCharacterSpritePos(character->sprite);
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

void DrawSelectCharacter(PlayField &playField, Level &level, bool onlyEnemies) {
    playField.selectedCharacter = nullptr;
    DrawSelectCharacters(playField, level.allCharacters, RED, level.camera.camera, onlyEnemies);
    if (playField.selectedCharacter != nullptr) {
        DrawStatusText(TextFormat("Selected: %s", playField.selectedCharacter->name.c_str()), YELLOW, 220, 10);
        int range = 1;
        if(level.selectedSkill != nullptr) {
            range = level.selectedSkill->range;
        }
        if(range == 1) {
            if (IsCharacterAdjacentToPlayer(*level.currentCharacter, *playField.selectedCharacter)) {
                // draw last line from player to selected character
                Vector2 start = GetCharacterSpritePos(level.currentCharacter->sprite);
                Vector2 end = GetCharacterSpritePos(playField.selectedCharacter->sprite);
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
                DrawStatusText(TextFormat("Too far away!"), WHITE, 240, 10);
            }
        } else {
            // draw last line from player to selected character
            Vector2 start = GetCharacterSpritePos(level.currentCharacter->sprite);
            Vector2 end = GetCharacterSpritePos(playField.selectedCharacter->sprite);
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
                    DrawStatusText(TextFormat("Too far away!"), WHITE, 240, 10);
                }
            } else {
                DrawStatusText(TextFormat("No line of sight!"), WHITE, 240, 10);
            }
        }
    } else {
        DrawStatusText("Select a character", WHITE, 220, 10);
    }
}

void DrawTargetSelection(PlayField &gridState, Level &level, bool onlyEnemies) {
    DrawSelectCharacter(gridState, level, onlyEnemies);
    if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
        level.selectedSkill = nullptr;
        level.selectedCharacter = nullptr;
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


void DrawGridCharacters(PlayField &playField, Level &level) {
    // Sort characters by y position
    std::vector<Character *> sortedCharacters;
    for (auto &character: level.allCharacters) {
        sortedCharacters.push_back(character);
    }
    std::sort(sortedCharacters.begin(), sortedCharacters.end(), [&level](Character *a, Character *b) {
        return GetAnimatedCharPos(level, a).y < GetAnimatedCharPos(level, b).y;
        //return a->sprite.player.position.y < b->sprite.player.position.y;
    });


    // Draw a highlight for the current character if not moving
    if (level.currentCharacter != nullptr && (level.turnState == TurnState::SelectAction || level.turnState == TurnState::SelectEnemy)) {
        Vector2 charPos = GetAnimatedCharPos(level, level.currentCharacter);
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
        Vector2 charPos = GetAnimatedCharPos(level, character);
        // Draw oval shadow underneath
        if(character->health > 0 && level.turnState != TurnState::Victory && level.turnState != TurnState::Defeat)
            DrawEllipse((int) charPos.x, (int) charPos.y, 6, 4, Fade(BLACK, 0.25f));

        if (IsCharacterVisible(level, character)) {
            DrawCharacterSprite(character->sprite, charPos.x, charPos.y);
        } else {
            SetCharacterSpriteTint(character->sprite, {255, 255, 255, 64});
            DrawCharacterSprite(character->sprite, charPos.x, charPos.y);
            SetCharacterSpriteTint(character->sprite, WHITE); // Reset tint
        }

        // Draw health bar
        if(character->health > 0 && level.turnState != TurnState::Victory && level.turnState != TurnState::Defeat)
            DrawHealthBar(charPos.x - 8, charPos.y - 21, 15, character->health, character->maxHealth);
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

static void DrawPathAndSelection(PlayField &playField, Level &level) {
    if (playField.mode == PlayFieldMode::SelectingTile) {
        DrawPathSelection(playField, level);
    }
    if (playField.mode == PlayFieldMode::SelectingEnemyTarget) {
        DrawTargetSelection(playField, level, true);
    }
}


void DrawTileSelection(PlayField &playField, Level &level) {
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

void UpdatePlayFieldOLD(PlayField &playField, Level &level, float dt) {
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

            // Lerp the x and y components separately
            SetCharacterSpritePosX(level.currentCharacter->sprite, Lerp(start.x, end.x, t));
            SetCharacterSpritePosY(level.currentCharacter->sprite, Lerp(start.y, end.y, t));

            // Determine the direction of movement and set the appropriate animation
            if (fabs(end.x - start.x) > fabs(end.y - start.y)) {
                // Horizontal movement
                if (end.x > start.x) {
                    PlayCharacterSpriteAnim(level.currentCharacter->sprite, SpriteAnimationType::WalkRight, true);
                    level.currentCharacter->orientation = Orientation::Right;
                } else {
                    PlayCharacterSpriteAnim(level.currentCharacter->sprite, SpriteAnimationType::WalkLeft, true);
                    level.currentCharacter->orientation = Orientation::Left;
                }
            } else {
                // Vertical movement
                if (end.y > start.y) {
                    PlayCharacterSpriteAnim(level.currentCharacter->sprite, SpriteAnimationType::WalkDown, true);
                    level.currentCharacter->orientation = Orientation::Down;
                } else {
                    PlayCharacterSpriteAnim(level.currentCharacter->sprite, SpriteAnimationType::WalkUp, true);
                    level.currentCharacter->orientation = Orientation::Up;
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
                    PauseCharacterSpriteAnim(level.currentCharacter->sprite);

                    SetCharacterSpriteFrame(level.currentCharacter->sprite, 0);
                    // set final position
                    auto finalPos = playField.path.path[playField.path.path.size() - 1];
                    SetCharacterSpritePos(level.currentCharacter->sprite, GridToPixelPosition(finalPos.x, finalPos.y));

                    if (IsPlayerCharacter(*level.currentCharacter)) {
                        level.turnState = TurnState::SelectAction;
                    } else {
                        level.turnState = TurnState::EnemyTurn;
                    }
                }
            }
        }
    }

    // Update animations for all characters
    for (auto &character: level.allCharacters) {
        UpdateCharacterSprite(character->sprite, dt);
    }
}

void updateActiveMovement(PlayField &playField, float dt) {
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

            // Lerp the x and y components separately
            SetCharacterSpritePosX(move.character->sprite, Lerp(start.x, end.x, t));
            SetCharacterSpritePosY(move.character->sprite, Lerp(start.y, end.y, t));

            // Determine the direction of movement and set the appropriate animation
            if (fabs(end.x - start.x) > fabs(end.y - start.y)) {
                // Horizontal movement
                if (end.x > start.x) {
                    PlayCharacterSpriteAnim(move.character->sprite, SpriteAnimationType::WalkRight, true);
                    move.character->orientation = Orientation::Right;
                } else {
                    PlayCharacterSpriteAnim(move.character->sprite, SpriteAnimationType::WalkLeft, true);
                    move.character->orientation = Orientation::Left;
                }
            } else {
                // Vertical movement
                if (end.y > start.y) {
                    PlayCharacterSpriteAnim(move.character->sprite, SpriteAnimationType::WalkDown, true);
                    move.character->orientation = Orientation::Down;
                } else {
                    PlayCharacterSpriteAnim(move.character->sprite, SpriteAnimationType::WalkUp, true);
                    move.character->orientation = Orientation::Up;
                }
            }

            // Check if we have completed the current step
            if (move.path.moveTime >= move.path.moveSpeed) {
                move.path.moveTime = 0.0f;
                move.path.currentStep++;

                // If the last step is reached, stop moving
                if (move.path.currentStep >= move.path.path.size() - 1) {
                    //StopSoundEffect(SoundEffectType::Footstep);
                    PauseCharacterSpriteAnim(move.character->sprite);
                    SetCharacterSpriteFrame(move.character->sprite, 0);
                    // set final position
                    auto finalPos = move.path.path[move.path.path.size() - 1];
                    SetCharacterSpritePos(move.character->sprite, GridToPixelPosition(finalPos.x, finalPos.y));
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

void UpdatePlayField(PlayField &playField, Level &level, float dt) {
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
    updateActiveMovement(playField, dt);
    // Update animations for all characters
    for (auto &character: level.allCharacters) {
        UpdateCharacterSprite(character->sprite, dt);
    }
}

static void handleInputPlayFieldNormal(PlayField &playField, Level &level) {

}

static void handleInputPlayFieldSelectingTile(PlayField &playField, Level &level) {

}

static void handleInputPlayFieldSelectingEnemyTarget(PlayField &playField, Level &level) {

}

static void handleInputPlayFieldShowMove(PlayField &playField, Level &level) {
    // check if mouse is over tile
    playField.selectedTilePos = {-1, -1};
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), level.camera.camera);
    Vector2i gridPos = PixelToGridPositionI(mousePos.x, mousePos.y);
    if (!IsTileOccupied(level, gridPos.x, gridPos.y, nullptr)) {
        // calculate a path and draw it as lines
        Path path;
        Vector2i target = PixelToGridPositionI(static_cast<int>(mousePos.x), static_cast<int>(mousePos.y));
        auto& playerChar = level.partyCharacters.front();
        if (CalcPath(level, path, PixelToGridPositionI((int) GetCharacterSpritePosX(playerChar->sprite),
                                                       (int) GetCharacterSpritePosY(playerChar->sprite)),
                     target, playerChar, IsTileOccupiedEnemies)) {
            playField.selectedTilePos = gridPos;
            if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                PublishMovePartyEvent(*playField.eventQueue, gridPos);
            }
        }
    }
}

void HandleInputPlayField(PlayField &playField, Level &level) {
    switch(playField.mode) {
        case PlayFieldMode::Normal:handleInputPlayFieldNormal(playField, level);break;
        case PlayFieldMode::SelectingTile:handleInputPlayFieldSelectingTile(playField, level);break;
        case PlayFieldMode::SelectingEnemyTarget:handleInputPlayFieldSelectingEnemyTarget(playField, level);break;
        case PlayFieldMode::Move:handleInputPlayFieldShowMove(playField, level);break;
    }
}

void DrawPlayField(PlayField &playField, Level &level) {
    BeginMode2D(level.camera.camera);
    // Draw tilemap layer 0
    DrawTileLayer(level.tileMap, BOTTOM_LAYER, 0, 0);
    EndMode2D();
    DrawBloodPools();
    BeginMode2D(level.camera.camera);
    DrawTileLayer(level.tileMap, MIDDLE_LAYER, 0, 0);
    //DrawSelectCharacter(playField, level, true);
    //DrawPathAndSelection(playField, level);

    if(playField.mode == PlayFieldMode::Move) {
        DrawTileSelection(playField, level);
    }

    DrawGridCharacters(playField, level);
    DrawParticleManager(*playField.particleManager);
    DrawTileLayer(level.tileMap, TOP_LAYER, 0, 0);

    // get mouse position
    playField.floatingStatsCharacter = nullptr;
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), level.camera.camera);
    Vector2 gridPos = PixelToGridPosition(mousePos.x, mousePos.y);
    // check if mouse is over character
    for (auto &character: level.turnOrder) {
        // skip dead
        if (character->health <= 0) {
            continue;
        }
        Vector2 gridPosCharacter = PixelToGridPosition(GetCharacterSpritePosX(character->sprite),
                                                       GetCharacterSpritePosY(character->sprite));
        if ((int) gridPosCharacter.x == (int) gridPos.x && (int) gridPosCharacter.y == (int) gridPos.y) {
            playField.floatingStatsCharacter = character;
        }
    }
    EndMode2D();
}

void MoveCharacter(PlayField &playField, Level &level, Character *character, Vector2i target) {
    // calculate a path and draw it as lines
    Path path;
    Vector2i cCharPos = GetCharacterSpritePosI(character->sprite);
    Vector2i cGridPos = PixelToGridPositionI(cCharPos.x, cCharPos.y);
    if (CalcPath(level, path, cGridPos, target, character, IsTileOccupiedEnemies)) {
        CharacterMove move;
        move.character = character;
        move.path = path;
        move.isDone = false;
        playField.activeMoves.push_back(move);
    } else {
        TraceLog(LOG_WARNING, "No path found");
    }
}

void MoveCharacterPartial(PlayField &playField, Level &level, Character *character, Vector2i target) {
    // calculate a path and draw it as lines
    Path path;
    Vector2i cCharPos = GetCharacterSpritePosI(character->sprite);
    Vector2i cGridPos = PixelToGridPositionI(cCharPos.x, cCharPos.y);
    CalcPathWithRangePartial(level, path, cGridPos, target, 1, character, IsTileOccupiedEnemies);
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


