//
// Created by bison on 20-11-25.
//

#include "LevelSystem.h"
#include <algorithm>
#include <cmath>
#include "graphics/Animation.h"
#include "raylib.h"
#include "ui/UI.h"
#include "raymath.h"
#include "graphics/ParticleSystem.h"
#include "level/Combat.h"
#include "ai/PathFinding.h"
#include "audio/Sound.h"
#include "game/ActionSystem.h"
#include "ui/Icons.h"

static void UpdateAnimations(SpriteData& spriteData, CharacterData& charData, Level &level, float dt) {
    for (auto &anim : level.animations) {
        UpdateAnimation(spriteData, charData, anim, dt);
    }
    // Use erase-remove idiom to remove animations which are done
    level.animations.erase(
            std::remove_if(level.animations.begin(), level.animations.end(),
                           [](const Animation& anim) {
                               return anim.IsDone();
                           }),
            level.animations.end()
    );
}

void InitLevelSystem(LevelSystemData &systemData) {
    systemData.moving = false;
    systemData.mode = LevelMode::None;
    systemData.selectedCharacter = -1;
    systemData.selectedTile = {-1, -1};
    systemData.path = {};
}

static void updateTurnBasedMove(GameData& data, Level &level, float dt) {
    LevelSystemData& levelData = data.levelData;
    if (levelData.moving) {
        levelData.path.moveTime += dt;

        // Calculate the percentage of completion for the current step
        float t = levelData.path.moveTime / levelData.path.moveSpeed;

        if (levelData.path.currentStep < levelData.path.path.size() - 1) {
            // Get the current and next waypoint positions
            Vector2 start = GridToPixelPosition(
                    levelData.path.path[levelData.path.currentStep].x,
                    levelData.path.path[levelData.path.currentStep].y);
            Vector2 end = GridToPixelPosition(
                    levelData.path.path[levelData.path.currentStep + 1].x,
                    levelData.path.path[levelData.path.currentStep + 1].y);

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
            if (levelData.path.moveTime >= levelData.path.moveSpeed) {
                levelData.path.moveTime = 0.0f;
                levelData.path.currentStep++;

                // If the last step is reached, stop moving
                if (levelData.path.currentStep >= levelData.path.path.size() - 1) {
                    StopSfx(data.soundData, level.footStepsHandle);
                    level.footStepsHandle = -1;
                    levelData.moving = false;
                    PauseCharacterSpriteAnim(data.spriteData, sprite);

                    SetCharacterSpriteFrame(data.spriteData, sprite, 0);
                    // set final position
                    auto finalPos = levelData.path.path[levelData.path.path.size() - 1];
                    SetCharacterSpritePos(data.spriteData, sprite, GridToPixelPosition(finalPos.x, finalPos.y));

                    ResetLevelSystem(levelData);
                    if (IsPlayerCharacter(data.charData, level.currentCharacter)) {
                        level.turnState = TurnState::SelectDestination;
                        levelData.mode = LevelMode::SelectingTile;
                    } else {
                        level.turnState = TurnState::EnemyTurn;
                    }
                }
            }
        }
    }
}

static void updateRealtimeMovement(GameData& data, Level& level, float dt) {
    for(auto& move : data.levelData.activeMoves) {
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
                    ExecutePendingAction(data);
                }
            }
        }
    }
    // Use erase-remove idiom to remove animations which are done
    data.levelData.activeMoves.erase(
            std::remove_if(data.levelData.activeMoves.begin(), data.levelData.activeMoves.end(),
                           [](const CharacterMove& m) {
                               if(m.isDone)
                                   TraceLog(LOG_INFO, "Removing move from active list");
                               return m.isDone;
                           }),
            data.levelData.activeMoves.end()
    );
}

static void checkIfPartySpotted(GameData& data, Level &level) {
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

static void UpdateFloatingStats(GameData& data, Level &level) {
    // get mouse position
    data.ui.level.floatingStatsCharacter = -1;
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
            data.ui.level.floatingStatsCharacter = character;
        }
    }
}

static void UpdateMousePointer(GameData& data, Level &level) {
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), level.camera.camera);
    Vector2i gridPos = PixelToGridPositionI(mousePos.x, mousePos.y);

    for (auto& entry : level.doors) {
        auto &door = entry.second;
        // door world pixel rect
        Vector2 pos = GridToPixelPosition(door.gridPos.x, door.gridPos.y);
        auto frameInfo = GetFrameInfo(data.spriteData, door.animPlayer);
        Rectangle frameRectWorld = {
                pos.x - 8.0f,
                pos.y - 8.0f,
                frameInfo.srcRect.width,
                frameInfo.srcRect.height
        };
        // Hover sets the cursor icon
        if (CheckCollisionPointRec(mousePos, frameRectWorld)) {
            data.ui.currentCursorIcon = ICON_INTERACT;
        }
    }

    if(!HasLineOfSightToPartyLight(data.spriteData, data.charData, level, gridPos)) {
        return;
    }

    for (int npcId : level.npcCharacters) {
        Vector2i npcPos = GetCharacterGridPosI(data.spriteData, data.charData.sprite[npcId]);
        if (npcPos == gridPos) {
            data.ui.currentCursorIcon = ICON_TALK;
            break;
        }
    }

    for (auto& entry : level.objects) {
        auto &obj = entry.second;

        // position and bounds
        Vector2 pos = GridToPixelPosition(obj.gridPos.x, obj.gridPos.y);
        auto frameInfo = GetFrameInfo(data.spriteData, obj.animPlayer);

        Rectangle frameRectWorld = {
                pos.x - 8.0f,
                pos.y - 8.0f,
                frameInfo.srcRect.width,
                frameInfo.srcRect.height
        };

        // require click on object + proximity
        if (!CheckCollisionPointRec(mousePos, frameRectWorld))
            continue;
        // object has inventory?
        auto& state = data.levelState[level.name];
        if (state.objectInventories.count(obj.id) > 0) {
            data.ui.currentCursorIcon = ICON_INTERACT;
            break;
        }
    }

    for(auto& exit : level.exits){
        Vector2 pos = GridToPixelPosition(exit.x, exit.y);
        Rectangle frameRectWorld = {pos.x - 8.0f, pos.y - 8.0f, (float) exit.width * 16, (float) exit.height * 16};
        if(CheckCollisionPointRec(mousePos, frameRectWorld)) {
            data.ui.currentCursorIcon = ICON_EXIT;
            return;
        }
    }
}

void UpdateLevelSystem(GameData& data, Level &level, float dt) {
    UpdateAnimations(data.spriteData, data.charData, level, dt);
    LevelSystemData& systemData = data.levelData;
    // Update the pulsing alpha
    if (systemData.increasing) {
        systemData.highlightAlpha = Lerp(systemData.highlightAlpha, 1.0f, dt * systemData.pulseSpeed);
        if (systemData.highlightAlpha >= 0.99f) {
            systemData.increasing = false;
        }
    } else {
        systemData.highlightAlpha = Lerp(systemData.highlightAlpha, 0.25f, dt * systemData.pulseSpeed);
        if (systemData.highlightAlpha <= 0.26f) {
            systemData.increasing = true;
        }
    }
    updateRealtimeMovement(data, level, dt);
    updateTurnBasedMove(data, level, dt);

    // Update animations for all characters
    for (auto &character: level.allCharacters) {
        UpdateCharacterSprite(data.spriteData, data.charData.sprite[character], dt);
    }
    if(level.turnState == TurnState::None) {
        checkIfPartySpotted(data, level);
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

    UpdateFloatingStats(data, level);
    UpdateMousePointer(data, level);
}

void MoveCharacter(GameData& data, Level &level, int character, Vector2i target) {
    // calculate a path and draw it as lines
    Path path;
    Vector2i cCharPos = GetCharacterSpritePosI(data.spriteData, data.charData.sprite[character]);
    Vector2i cGridPos = PixelToGridPositionI(cCharPos.x, cCharPos.y);
    if (CalcPath(data.spriteData, data.charData, level, path, cGridPos, target, character, IsTileOccupiedEnemies)) {
        CharacterMove move;
        move.character = character;
        move.path = path;
        move.isDone = false;
        data.levelData.activeMoves.push_back(move);
    } else {
        TraceLog(LOG_WARNING, "No path found");
    }
}

void MoveCharacterPartial(GameData& data, Level &level, int character, Vector2i target) {
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
        data.levelData.activeMoves.push_back(move);
    } else {
        TraceLog(LOG_WARNING, "No path found");
    }
}

void ResetLevelSystem(LevelSystemData &playField) {
    playField.selectedCharacter = -1;
    playField.activeMoves.clear();
    playField.moving = false;
    playField.mode = LevelMode::None;
    playField.selectedCharacter = -1;
    playField.selectedTile = {-1, -1};
    playField.path = {};
}