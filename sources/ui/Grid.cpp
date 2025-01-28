//
// Created by bison on 16-01-25.
//

#include <algorithm>
#include <cmath>
#include "Grid.h"
#include "raylib.h"
#include "UI.h"
#include "raymath.h"
#include "graphics/BloodPool.h"
#include "graphics/ParticleSystem.h"
#include "audio/SoundEffect.h"

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

static Vector2 GetAnimatedCharPos(CombatState &combat, Character *character) {
    // Check if the character is visible (not blinking)
    for (auto &animation: combat.animations) {
        if (animation.type == AnimationType::Attack) {
            if (animation.state.attack.attacker == character) {
                return {animation.state.attack.currentX, animation.state.attack.currentY};
            }
        }
    }
    return GetCharacterSpritePos(character->sprite);
}

void InitGrid(GridState &gridState, ParticleManager* particleManager) {
    gridState.particleManager = particleManager;
    gridState.moving = false;
    gridState.mode = GridMode::Normal;
    gridState.selectedCharacter = nullptr;
    gridState.selectedTile = {-1, -1};
    gridState.path = {};
    gridState.floatingStatsCharacter = nullptr;
}

static void ResetGridState(GridState &gridState) {
    gridState.moving = false;
    gridState.mode = GridMode::Normal;
    gridState.selectedCharacter = nullptr;
    gridState.selectedTile = {-1, -1};
    gridState.path = {};
}



// Helper function: Calculate vertical center offset in grid space
int CalculateVerticalCenterOffset(int gridHeight, int numCharacters) {
    int totalCharacterHeightInTiles = numCharacters;
    int emptySpaceInTiles = (gridHeight / 16) - totalCharacterHeightInTiles;
    return emptySpaceInTiles / 2; // Center the characters
}

void SetInitialGridPositions(GridState &gridState, CombatState &combat) {
    auto positions = FindFreePositionsCircular(combat, 6, 7, 5);
    // Set initial grid positions for player characters
    for (int i = 0; i < combat.playerCharacters.size(); i++) {
        // take a position from the list
        auto pos = positions.back();
        positions.pop_back();
        SetCharacterSpritePos(combat.playerCharacters[i]->sprite, GridToPixelPosition(pos.x, pos.y));
        // Set initial animation to paused
        StartPausedCharacterSpriteAnim(combat.playerCharacters[i]->sprite, SpriteAnimationType::WalkRight, true);;
        combat.playerCharacters[i]->orientation = Orientation::Right;
    }

    positions = FindFreePositionsCircular(combat, 23, 7, 5);
    // Set initial grid positions for enemy characters
    for (int i = 0; i < combat.enemyCharacters.size(); i++) {
        // take a position from the list
        auto pos = positions.back();
        positions.pop_back();
        SetCharacterSpritePos(combat.enemyCharacters[i]->sprite, GridToPixelPosition(pos.x, pos.y));
        // Set initial animation to paused
        StartPausedCharacterSpriteAnim(combat.enemyCharacters[i]->sprite, SpriteAnimationType::WalkLeft, true);
        combat.enemyCharacters[i]->orientation = Orientation::Left;
    }
}

void DrawPathSelection(GridState &gridState, CombatState &combat) {
    // check if mouse is over tile
    Vector2 mousePos = GetMousePosition();
    Vector2 gridPos = PixelToGridPosition(mousePos.x, mousePos.y);
    if (IsTileOccupied(combat, static_cast<int>(gridPos.x), static_cast<int>(gridPos.y), nullptr)) {
        gridState.selectedTile = gridPos;
        // calculate a path and draw it as lines
        Path path;
        Vector2i target = PixelToGridPositionI(static_cast<int>(mousePos.x), static_cast<int>(mousePos.y));
        if (InitPath(combat, path, PixelToGridPositionI((int) GetCharacterSpritePosX(combat.currentCharacter->sprite),
                                                        (int) GetCharacterSpritePosY(combat.currentCharacter->sprite)),
                     target, combat.currentCharacter)) {
            Color pathColor = Fade(WHITE, gridState.highlightAlpha);
            if (path.cost > combat.currentCharacter->movePoints) {
                DrawStatusText(TextFormat("Not enough movement points (%d)", combat.currentCharacter->movePoints),
                               WHITE, 220, 10);
                pathColor = Fade(RED, gridState.highlightAlpha);
                // Draw cross
                DrawLine(gridPos.x * 16, gridPos.y * 16 + 1, gridPos.x * 16 + 15, gridPos.y * 16 + 16, pathColor);
                DrawLine(gridPos.x * 16 + 15, gridPos.y * 16 + 1, gridPos.x * 16, gridPos.y * 16 + 16, pathColor);

            } else {
                DrawStatusText(TextFormat("Movement points %d/%d", path.cost, combat.currentCharacter->movePoints),
                               WHITE, 220, 10);
            }
            for (int i = 0; i < path.path.size() - 1; i++) {
                Vector2 start = GridToPixelPosition(path.path[i].x, path.path[i].y);
                Vector2 end = GridToPixelPosition(path.path[i + 1].x, path.path[i + 1].y);
                DrawLineEx(start, end, 1, pathColor);
            }
            int gridX = static_cast<int>(gridPos.x);
            int gridY = static_cast<int>(gridPos.y);
            if (gridX >= 0 && gridX < GRID_WIDTH && gridY >= 0 && gridY < GRID_HEIGHT) {
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
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && path.cost <= combat.currentCharacter->movePoints) {
                gridState.mode = GridMode::Normal;
                gridState.path = path;
                gridState.moving = true;
                combat.currentCharacter->movePoints -= path.cost;
                // cap at zero
                if (combat.currentCharacter->movePoints < 0) {
                    combat.currentCharacter->movePoints = 0;
                }
                combat.turnState = TurnState::Move;
                PlaySoundEffect(SoundEffectType::Select);
                PlaySoundEffect(SoundEffectType::Footstep);
            }
        }
    } else {
        DrawRectangleLinesEx(
                Rectangle{
                        (gridPos.x * 16),
                        (gridPos.y * 16) + 1,
                        15, 15
                },
                1, Fade(RED, gridState.highlightAlpha)
        );
        // Draw cross
        DrawLine(gridPos.x * 16, gridPos.y * 16 + 1, gridPos.x * 16 + 15, gridPos.y * 16 + 16,
                 Fade(RED, gridState.highlightAlpha));
        DrawLine(gridPos.x * 16 + 15, gridPos.y * 16 + 1, gridPos.x * 16, gridPos.y * 16 + 16,
                 Fade(RED, gridState.highlightAlpha));
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
        ResetGridState(gridState);
        combat.turnState = TurnState::SelectAction;
    }
}

void DrawSelectCharacters(GridState &gridState, std::vector<Character *> &characters, Color color) {
    Vector2 mousePos = GetMousePosition();
    Vector2 gridPos = PixelToGridPosition(mousePos.x, mousePos.y);
    for (auto &character: characters) {
        // skip death characters
        if (character->health <= 0) {
            continue;
        }
        Vector2 charPos = GetCharacterSpritePos(character->sprite);
        Vector2 gridPosCharacter = PixelToGridPosition(charPos.x, charPos.y);
        if ((int) gridPosCharacter.x == (int) gridPos.x && (int) gridPosCharacter.y == (int) gridPos.y) {
            DrawCircleLines(charPos.x, charPos.y, 10,
                            Fade(color, gridState.highlightAlpha));
            // Draw plus
            DrawLine(gridPos.x * 16 + 8, gridPos.y * 16 - 1, gridPos.x * 16 + 8, gridPos.y * 16 + 17,
                     Fade(color, gridState.highlightAlpha)); // Vertical line
            DrawLine(gridPos.x * 16 - 1, gridPos.y * 16 + 8, gridPos.x * 16 + 17, gridPos.y * 16 + 8,
                     Fade(color, gridState.highlightAlpha)); // Horizontal line
            gridState.selectedCharacter = character;
        }
    }
}

void DrawSelectCharacter(GridState &gridState, CombatState &combat, bool onlyEnemies) {
    gridState.selectedCharacter = nullptr;
    if (!onlyEnemies) {
        DrawSelectCharacters(gridState, combat.playerCharacters, YELLOW);
    }
    DrawSelectCharacters(gridState, combat.enemyCharacters, RED);
    if (gridState.selectedCharacter != nullptr) {
        DrawStatusText(TextFormat("Selected: %s", gridState.selectedCharacter->name.c_str()), YELLOW, 220, 10);
        int range = 1;
        if(combat.selectedSkill != nullptr) {
            range = combat.selectedSkill->range;
        }
        if(range == 1) {
            if (IsCharacterAdjacentToPlayer(*combat.currentCharacter, *gridState.selectedCharacter)) {
                // draw last line from player to selected character
                Vector2 start = GetCharacterSpritePos(combat.currentCharacter->sprite);
                Vector2 end = GetCharacterSpritePos(gridState.selectedCharacter->sprite);
                DrawLineEx(start, end, 1, Fade(RED, gridState.highlightAlpha));
                // Check for a mouse click
                if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                    PlaySoundEffect(SoundEffectType::Select);
                    combat.turnState = TurnState::Waiting;
                    combat.waitTime = 0.25f;
                    combat.selectedCharacter = gridState.selectedCharacter;
                    if (combat.selectedSkill == nullptr)
                        combat.nextState = TurnState::Attack;
                    else
                        combat.nextState = TurnState::UseSkill;
                    ResetGridState(gridState);
                }
            } else {
                DrawStatusText(TextFormat("Too far away!"), WHITE, 240, 10);
            }
        } else {
            // draw last line from player to selected character
            Vector2 start = GetCharacterSpritePos(combat.currentCharacter->sprite);
            Vector2 end = GetCharacterSpritePos(gridState.selectedCharacter->sprite);
            if(HasLineOfSight(combat, PixelToGridPositionI((int) start.x, (int) start.y), PixelToGridPositionI((int) end.x, (int) end.y))) {
                int distance = (int) Vector2Distance(start, end);
                if(distance <= range * 16) {
                    DrawLineEx(start, end, 1, Fade(RED, gridState.highlightAlpha));
                    // Check for a mouse click
                    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                        PlaySoundEffect(SoundEffectType::Select);
                        combat.turnState = TurnState::Waiting;
                        combat.waitTime = 0.25f;
                        combat.selectedCharacter = gridState.selectedCharacter;
                        if (combat.selectedSkill == nullptr)
                            combat.nextState = TurnState::Attack;
                        else
                            combat.nextState = TurnState::UseSkill;
                        ResetGridState(gridState);
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

void DrawTargetSelection(GridState &gridState, CombatState &combat, bool onlyEnemies) {
    DrawSelectCharacter(gridState, combat, onlyEnemies);
    if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
        combat.selectedSkill = nullptr;
        combat.selectedCharacter = nullptr;
        ResetGridState(gridState);
        combat.turnState = TurnState::SelectAction;
    }
}

// Function to draw the health bar
void DrawHealthBar(float x, float y, float width, float health, float maxHealth) {
    // Draw the health bar background (gray)
    DrawRectangle(x, y, width, 2, GRAY);
    // Draw the health bar foreground (green for positive health, red for baseAttack)
    DrawRectangle(x, y, width * (health / maxHealth), 2, GREEN);
}


void DrawGridCharacters(GridState &state, CombatState &combat) {
    // Sort characters by y position
    std::vector<Character *> sortedCharacters;
    for (auto &character: combat.playerCharacters) {
        sortedCharacters.push_back(character);
    }
    for (auto &character: combat.enemyCharacters) {
        sortedCharacters.push_back(character);
    }
    std::sort(sortedCharacters.begin(), sortedCharacters.end(), [&combat](Character *a, Character *b) {
        return GetAnimatedCharPos(combat, a).y < GetAnimatedCharPos(combat, b).y;
        //return a->sprite.player.position.y < b->sprite.player.position.y;
    });


    // Draw a highlight for the current character if not moving
    if (combat.currentCharacter != nullptr && (combat.turnState == TurnState::SelectAction || combat.turnState == TurnState::SelectEnemy)) {
        Vector2 charPos = GetAnimatedCharPos(combat, combat.currentCharacter);
        Color outlineColor = Fade(YELLOW, state.highlightAlpha);
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
        Vector2 charPos = GetAnimatedCharPos(combat, character);
        // Draw oval shadow underneath
        if(character->health > 0 && combat.turnState != TurnState::Victory && combat.turnState != TurnState::Defeat)
            DrawEllipse((int) charPos.x, (int) charPos.y, 6, 4, Fade(BLACK, 0.25f));

        if (IsCharacterVisible(combat, character)) {
            DrawCharacterSprite(character->sprite, charPos.x, charPos.y);
        } else {
            SetCharacterSpriteTint(character->sprite, {255, 255, 255, 64});
            DrawCharacterSprite(character->sprite, charPos.x, charPos.y);
            SetCharacterSpriteTint(character->sprite, WHITE); // Reset tint
        }

        // Draw health bar
        if(character->health > 0 && combat.turnState != TurnState::Victory && combat.turnState != TurnState::Defeat)
            DrawHealthBar(charPos.x - 8, charPos.y - 21, 15, character->health, character->maxHealth);
    }
}

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

static void DrawPathAndSelection(GridState &gridState, CombatState &combat) {
    if (gridState.mode == GridMode::SelectingTile) {
        DrawPathSelection(gridState, combat);
    }
    if (gridState.mode == GridMode::SelectingEnemyTarget) {
        DrawTargetSelection(gridState, combat, true);
    }
}

void UpdateGrid(GridState &gridState, CombatState &combat, float dt) {
    // Update the pulsing alpha
    if (gridState.increasing) {
        gridState.highlightAlpha = Lerp(gridState.highlightAlpha, 1.0f, dt * gridState.pulseSpeed);
        if (gridState.highlightAlpha >= 0.99f) {
            gridState.increasing = false;
        }
    } else {
        gridState.highlightAlpha = Lerp(gridState.highlightAlpha, 0.25f, dt * gridState.pulseSpeed);
        if (gridState.highlightAlpha <= 0.26f) {
            gridState.increasing = true;
        }
    }
    if (gridState.moving) {
        gridState.path.moveTime += dt;

        // Calculate the percentage of completion for the current step
        float t = gridState.path.moveTime / gridState.path.moveSpeed;

        if (gridState.path.currentStep < gridState.path.path.size() - 1) {
            // Get the current and next waypoint positions
            Vector2 start = GridToPixelPosition(
                    gridState.path.path[gridState.path.currentStep].x,
                    gridState.path.path[gridState.path.currentStep].y);
            Vector2 end = GridToPixelPosition(
                    gridState.path.path[gridState.path.currentStep + 1].x,
                    gridState.path.path[gridState.path.currentStep + 1].y);

            // Lerp the x and y components separately
            SetCharacterSpritePosX(combat.currentCharacter->sprite, Lerp(start.x, end.x, t));
            SetCharacterSpritePosY(combat.currentCharacter->sprite, Lerp(start.y, end.y, t));

            // Determine the direction of movement and set the appropriate animation
            if (fabs(end.x - start.x) > fabs(end.y - start.y)) {
                // Horizontal movement
                if (end.x > start.x) {
                    PlayCharacterSpriteAnim(combat.currentCharacter->sprite, SpriteAnimationType::WalkRight, true);
                    combat.currentCharacter->orientation = Orientation::Right;
                } else {
                    PlayCharacterSpriteAnim(combat.currentCharacter->sprite, SpriteAnimationType::WalkLeft, true);
                    combat.currentCharacter->orientation = Orientation::Left;
                }
            } else {
                // Vertical movement
                if (end.y > start.y) {
                    PlayCharacterSpriteAnim(combat.currentCharacter->sprite, SpriteAnimationType::WalkDown, true);
                    combat.currentCharacter->orientation = Orientation::Down;
                } else {
                    PlayCharacterSpriteAnim(combat.currentCharacter->sprite, SpriteAnimationType::WalkUp, true);
                    combat.currentCharacter->orientation = Orientation::Up;
                }
            }

            // Check if we have completed the current step
            if (gridState.path.moveTime >= gridState.path.moveSpeed) {
                gridState.path.moveTime = 0.0f;
                gridState.path.currentStep++;

                // If the last step is reached, stop moving
                if (gridState.path.currentStep >= gridState.path.path.size() - 1) {
                    StopSoundEffect(SoundEffectType::Footstep);
                    gridState.moving = false;
                    PauseCharacterSpriteAnim(combat.currentCharacter->sprite);

                    SetCharacterSpriteFrame(combat.currentCharacter->sprite, 0);
                    // set final position
                    auto finalPos = gridState.path.path[gridState.path.path.size() - 1];
                    SetCharacterSpritePos(combat.currentCharacter->sprite, GridToPixelPosition(finalPos.x, finalPos.y));

                    if (IsPlayerCharacter(combat, *combat.currentCharacter)) {
                        combat.turnState = TurnState::SelectAction;
                    } else {
                        combat.turnState = TurnState::EnemyTurn;
                    }
                }
            }
        }
    }

    // Update animations for all characters
    for (auto &character: combat.playerCharacters) {
        UpdateCharacterSprite(character->sprite, dt);
    }
    for (auto &character: combat.enemyCharacters) {
        UpdateCharacterSprite(character->sprite, dt);
    }
}


void DrawGrid(GridState &gridState, CombatState &combat) {
    DrawGridLines();
    // Draw tilemap layer 0
    DrawTileLayer(combat.tileMap, BOTTOM_LAYER, 0, 0);
    DrawBloodPools();
    DrawTileLayer(combat.tileMap, MIDDLE_LAYER, 0, 0);
    DrawPathAndSelection(gridState, combat);
    DrawGridCharacters(gridState, combat);
    DrawParticleManager(*gridState.particleManager);
    DrawTileLayer(combat.tileMap, TOP_LAYER, 0, 0);

    // get mouse position
    gridState.floatingStatsCharacter = nullptr;
    Vector2 mousePos = GetMousePosition();
    Vector2 gridPos = PixelToGridPosition(mousePos.x, mousePos.y);
    // check if mouse is over character
    for (auto &character: combat.turnOrder) {
        // skip dead
        if (character->health <= 0) {
            continue;
        }
        Vector2 gridPosCharacter = PixelToGridPosition(GetCharacterSpritePosX(character->sprite),
                                                       GetCharacterSpritePosY(character->sprite));
        if ((int) gridPosCharacter.x == (int) gridPos.x && (int) gridPosCharacter.y == (int) gridPos.y) {
            gridState.floatingStatsCharacter = character;
        }
    }
}


