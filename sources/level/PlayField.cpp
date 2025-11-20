//
// Created by bison on 16-01-25.
//

#include <algorithm>
#include <cmath>
#include <queue>
#include "PlayField.h"
#include "raylib.h"
#include "ui/UI.h"
#include "raymath.h"
#include "graphics/ParticleSystem.h"
#include "level/Combat.h"
#include "graphics/TileMap.h"
#include "ai/PathFinding.h"
#include "audio/Sound.h"
#include "game/ActionSystem.h"

void CreatePlayField(PlayField &playField, ParticleManager* particleManager) {
    playField.particleManager = particleManager;
    playField.moving = false;
    playField.mode = PlayFieldMode::None;
    playField.selectedCharacter = -1;
    playField.selectedTile = {-1, -1};
    playField.path = {};
}

static void updateTurnBasedMove(GameData& data, PlayField &playField, Level &level, float dt) {
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

            CharacterSprite& sprite = data.charData.sprite[level.currentCharacter];
            // Lerp the x and y components separately
            SetCharacterSpritePosX(data.spriteData, sprite, Lerp(start.x, end.x, t));
            SetCharacterSpritePosY(data.spriteData, sprite, Lerp(start.y, end.y, t));

            // Determine the direction of movement and set the appropriate animation
            if (fabs(end.x - start.x) > fabs(end.y - start.y)) {
                // Horizontal movement
                if (end.x > start.x) {
                    PlayCharacterSpriteAnim(data.spriteData, sprite, SpriteAnimationType::WalkRight, true);
                    data.charData.orientation[level.currentCharacter] = Orientation::Right;
                } else {
                    PlayCharacterSpriteAnim(data.spriteData, sprite, SpriteAnimationType::WalkLeft, true);
                    data.charData.orientation[level.currentCharacter] = Orientation::Left;
                }
            } else {
                // Vertical movement
                if (end.y > start.y) {
                    PlayCharacterSpriteAnim(data.spriteData, sprite, SpriteAnimationType::WalkDown, true);
                    data.charData.orientation[level.currentCharacter] = Orientation::Down;
                } else {
                    PlayCharacterSpriteAnim(data.spriteData, sprite, SpriteAnimationType::WalkUp, true);
                    data.charData.orientation[level.currentCharacter] = Orientation::Up;
                }
            }

            // Check if we have completed the current step
            if (playField.path.moveTime >= playField.path.moveSpeed) {
                playField.path.moveTime = 0.0f;
                playField.path.currentStep++;

                // If the last step is reached, stop moving
                if (playField.path.currentStep >= playField.path.path.size() - 1) {
                    StopSfx(data.soundData, level.footStepsHandle);
                    level.footStepsHandle = -1;
                    playField.moving = false;
                    PauseCharacterSpriteAnim(data.spriteData, sprite);

                    SetCharacterSpriteFrame(data.spriteData, sprite, 0);
                    // set final position
                    auto finalPos = playField.path.path[playField.path.path.size() - 1];
                    SetCharacterSpritePos(data.spriteData, sprite, GridToPixelPosition(finalPos.x, finalPos.y));

                    ResetPlayField(playField);
                    if (IsPlayerCharacter(data.charData, level.currentCharacter)) {
                        level.turnState = TurnState::SelectDestination;
                        playField.mode = PlayFieldMode::SelectingTile;
                    } else {
                        level.turnState = TurnState::EnemyTurn;
                    }
                }
            }
        }
    }
}

static void updateActiveMovement(GameData& data, PlayField &playField, Level& level, float dt) {
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

            CharacterSprite& sprite = data.charData.sprite[move.character];
            // Lerp the x and y components separately
            SetCharacterSpritePosX(data.spriteData, sprite, Lerp(start.x, end.x, t));
            SetCharacterSpritePosY(data.spriteData, sprite, Lerp(start.y, end.y, t));

            // Determine the direction of movement and set the appropriate animation
            if (fabs(end.x - start.x) > fabs(end.y - start.y)) {
                // Horizontal movement
                if (end.x > start.x) {
                    PlayCharacterSpriteAnim(data.spriteData, sprite, SpriteAnimationType::WalkRight, true);
                    data.charData.orientation[move.character] = Orientation::Right;
                } else {
                    PlayCharacterSpriteAnim(data.spriteData, sprite, SpriteAnimationType::WalkLeft, true);
                    data.charData.orientation[move.character] = Orientation::Left;
                }
            } else {
                // Vertical movement
                if (end.y > start.y) {
                    PlayCharacterSpriteAnim(data.spriteData, sprite, SpriteAnimationType::WalkDown, true);
                    data.charData.orientation[move.character] = Orientation::Down;
                } else {
                    PlayCharacterSpriteAnim(data.spriteData, sprite, SpriteAnimationType::WalkUp, true);
                    data.charData.orientation[move.character] = Orientation::Up;
                }
            }

            // Check if we have completed the current step
            if (move.path.moveTime >= move.path.moveSpeed) {
                move.path.moveTime = 0.0f;
                move.path.currentStep++;

                // If the last step is reached, stop moving
                if (move.path.currentStep >= move.path.path.size() - 1) {
                    StopSfx(data.soundData, level.footStepsHandle);
                    level.footStepsHandle = -1;
                    PauseCharacterSpriteAnim(data.spriteData, sprite);
                    SetCharacterSpriteFrame(data.spriteData, sprite, 0);
                    // set final position
                    auto finalPos = move.path.path[move.path.path.size() - 1];
                    SetCharacterSpritePos(data.spriteData, sprite, GridToPixelPosition(finalPos.x, finalPos.y));
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

static void checkIfPartySpotted(GameData& data, PlayField &playField, Level &level) {
    for(auto& c : level.allCharacters) {
        if(data.charData.faction[c] != CharacterFaction::Enemy || data.charData.stats[c].HP <= 0) {
            continue;
        }
        Vector2i enemyGridPos = GetCharacterGridPosI(data.spriteData, data.charData.sprite[c]);
        for(auto& partyChar : level.partyCharacters) {
            Vector2i partyGridPos = GetCharacterGridPosI(data.spriteData, data.charData.sprite[partyChar]);
            if(HasLineOfSight(level, enemyGridPos, partyGridPos, 16)) {
                TraceLog(LOG_INFO, "Party last spotted by %s", data.charData.name[c].c_str());
                PushPartySpotted(data.actionQueue, c);
                return;
            }
        }
    }
}

void UpdatePlayField(GameData& data, PlayField &playField, Level &level, float dt) {
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
    updateActiveMovement(data, playField, level, dt);
    updateTurnBasedMove(data, playField, level, dt);

    // Update animations for all characters
    for (auto &character: level.allCharacters) {
        UpdateCharacterSprite(data.spriteData, data.charData.sprite[character], dt);
    }
    if(level.turnState == TurnState::None) {
        checkIfPartySpotted(data, playField, level);
        //checkLevelExits(data, level);
    }
    for(auto& entry : level.objects) {
        auto& obj = entry.second;
        UpdateSpriteAnimation(data.spriteData, obj.animPlayer, dt);
    }
    for(auto& entry : level.doors) {
        auto& door = entry.second;
        UpdateSpriteAnimation(data.spriteData, door.animPlayer, dt);
    }
}

void MoveCharacter(GameData& data, PlayField &playField, Level &level, int character, Vector2i target) {
    // calculate a path and draw it as lines
    Path path;
    Vector2i cCharPos = GetCharacterSpritePosI(data.spriteData, data.charData.sprite[character]);
    Vector2i cGridPos = PixelToGridPositionI(cCharPos.x, cCharPos.y);
    if (CalcPath(data.spriteData, data.charData, level, path, cGridPos, target, character, IsTileOccupiedEnemies)) {
        CharacterMove move;
        move.character = character;
        move.path = path;
        move.isDone = false;
        playField.activeMoves.push_back(move);
    } else {
        TraceLog(LOG_WARNING, "No path found");
    }
}

void MoveCharacterPartial(GameData& data, PlayField &playField, Level &level, int character, Vector2i target) {
    // calculate a path and draw it as lines
    Path path;
    Vector2i cCharPos = GetCharacterSpritePosI(data.spriteData, data.charData.sprite[character]);
    Vector2i cGridPos = PixelToGridPositionI(cCharPos.x, cCharPos.y);
    CalcPathWithRangePartial(data.spriteData, data.charData, level, path, cGridPos, target, 1, character, IsTileOccupiedEnemies);
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
