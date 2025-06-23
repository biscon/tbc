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

Vector2 GetAnimatedCharPos(SpriteData& spriteData, CharacterData& charData, Level &level, int character) {
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



    // Draw characters
    for (auto &character: sortedCharacters) {
        Vector2 charPos = GetAnimatedCharPos(spriteData, charData, level, character);
        // Draw oval shadow underneath
        if(charData.stats[character].health > 0 && level.turnState != TurnState::Victory && level.turnState != TurnState::Defeat)
            DrawEllipse((int) charPos.x, (int) charPos.y, 6, 4, Fade(BLACK, 0.25f));

        CharacterSprite& charSprite = charData.sprite[character];
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
        if(stats.health > 0 && level.turnState != TurnState::Victory && level.turnState != TurnState::Defeat && level.turnState != TurnState::None) {
            DrawHealthBar(charPos.x - 8, charPos.y - 21, 15, (float) stats.health, (float) stats.maxHealth);
        } else if(std::count(level.partyCharacters.begin(), level.partyCharacters.end(), character)) {
            DrawHealthBar(charPos.x - 8, charPos.y - 21, 15, (float) stats.health, (float) stats.maxHealth);
        }
    }
}

static void DrawLevelObjects(SpriteData& spriteData, Level &level) {
    for(auto& entry : level.objects) {
        auto& obj = entry.second;
        Vector2 pos = GridToPixelPosition(obj.gridPos.x, obj.gridPos.y);
        if(obj.lit) {
            Vector2i& t = obj.gridPos;
            // NOTE: should really sample at all 4 corners if sprite is bigger than a tile
            Color v1 = GetVertexLight(level.lighting, level.tileMap, t.x, t.y);     // top-left corner
            Color v2 = GetVertexLight(level.lighting, level.tileMap, t.x+1, t.y);   // top-right
            Color v3 = GetVertexLight(level.lighting, level.tileMap, t.x+1, t.y+1); // bottom-right
            Color v4 = GetVertexLight(level.lighting, level.tileMap, t.x, t.y+1);   // bottom-left
            DrawSpriteAnimationColors(spriteData, obj.animPlayer, pos.x - 8.0f, pos.y - 8.0f, v1, v2, v3, v4);
        } else {
            DrawSpriteAnimation(spriteData, obj.animPlayer, pos.x - 8.0f, pos.y - 8.0f);
        }

    }
}

static void DrawDoors(SpriteData& spriteData, Level &level) {
    for(auto& entry : level.doors) {
        auto& door = entry.second;
        Vector2 pos = GridToPixelPosition(door.gridPos.x, door.gridPos.y);
        Vector2i& t = door.gridPos;
        // NOTE: should really sample at all 4 corners if sprite is bigger than a tile
        Color v1 = GetVertexLight(level.lighting, level.tileMap, t.x, t.y);     // top-left corner
        Color v2 = GetVertexLight(level.lighting, level.tileMap, t.x+1, t.y);   // top-right
        Color v3 = GetVertexLight(level.lighting, level.tileMap, t.x+1, t.y+1); // bottom-right
        Color v4 = GetVertexLight(level.lighting, level.tileMap, t.x, t.y+1);   // bottom-left
        DrawSpriteAnimationColors(spriteData, door.animPlayer, pos.x - 8.0f, pos.y - 8.0f, v1, v2, v3, v4);
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
                    StopSoundEffect(SoundEffectType::Footstep);
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
    for(auto& entry : level.objects) {
        auto& obj = entry.second;
        UpdateSpriteAnimation(spriteData, obj.animPlayer, dt);
    }
    for(auto& entry : level.doors) {
        auto& door = entry.second;
        UpdateSpriteAnimation(spriteData, door.animPlayer, dt);
    }
}

static void handleInputPlayFieldNormal(PlayField &playField, Level &level) {

}

static void handleInputPlayFieldSelectingTile(PlayField &playField, Level &level) {

}

static void handleInputPlayFieldSelectingEnemyTarget(PlayField &playField, Level &level) {

}

static bool handleDoors(SpriteData& spriteData, PlayField &playField, Level &level, Vector2i gridPos, Vector2i playerPos) {
    for(auto& entry : level.doors){
        auto& door = entry.second;
        for(auto& doorTile : door.blockedTiles) {
            if(gridPos == doorTile && IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && Distance(playerPos, gridPos) < 3 && playerPos != doorTile) {
                if(!door.open) {
                    TraceLog(LOG_INFO, "Opening door %s", door.id.c_str());
                    door.open = true;
                    SetReverseSpriteAnimation(spriteData, door.animPlayer, false);
                    ResumeSpriteAnimation(spriteData, door.animPlayer);
                    SetFrame(spriteData, door.animPlayer, 0);
                } else {
                    TraceLog(LOG_INFO, "Closing door %s", door.id.c_str());
                    door.open = false;
                    SetReverseSpriteAnimation(spriteData, door.animPlayer, true);
                    int anim = spriteData.player.animationIdx[door.animPlayer];
                    int frames = (int) spriteData.anim.frames[anim].size();
                    SetFrame(spriteData, door.animPlayer, frames-1);
                    ResumeSpriteAnimation(spriteData, door.animPlayer);
                }
                SetTiles(level.tileMap, door.blockedTiles, NAV_LAYER, door.open ? 0 : 1);
                SetTiles(level.tileMap, door.shadowTiles, SHADOW_LAYER, door.open ? 0 : 1);
                PropagateLight(level.lighting, level.tileMap);
                return true;
            }
        }
    }
    return false;
}

static void handleInputPlayFieldExploration(SpriteData& spriteData, CharacterData& charData, PlayField &playField, Level &level) {
    // check if mouse is over tile
    playField.selectedTilePos = {-1, -1};
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), level.camera.camera);
    Vector2i gridPos = PixelToGridPositionI(mousePos.x, mousePos.y);
    auto& playerChar = level.partyCharacters.front();
    Vector2i playerPos = GetCharacterGridPosI(spriteData, charData.sprite[playerChar]);

    if(handleDoors(spriteData, playField, level, gridPos, playerPos)) {
        return;
    }

    if (!IsTileOccupied(spriteData, charData, level, gridPos.x, gridPos.y, -1)) {
        // calculate a path and draw it as lines
        Path path;
        Vector2i target = PixelToGridPositionI(static_cast<int>(mousePos.x), static_cast<int>(mousePos.y));
        if (CalcPath(spriteData, charData, level, path, PixelToGridPositionI((int) GetCharacterSpritePosX(spriteData, charData.sprite[playerChar]),
                                                       (int) GetCharacterSpritePosY(spriteData, charData.sprite[playerChar])),
                     target, playerChar, IsTileOccupiedEnemies)) {
            playField.selectedTilePos = gridPos;
            if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                PublishMovePartyEvent(*playField.eventQueue, gridPos);
            }
        }
    } else {

        for(int npcId : level.npcCharacters) {
            Vector2i npcPos = GetCharacterGridPosI(spriteData, charData.sprite[npcId]);
            if(npcPos == gridPos) {
                if(Distance(playerPos, npcPos) < 3) {
                    playField.hintText = "Talk to " + charData.name[npcId];
                    if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                        PublishInitiateDialogueEvent(*playField.eventQueue, npcId, level.npcDialogueNodeIds[npcId]);
                    }
                } else {
                    playField.hintText = "Too far away!";
                }
            }
        }
    }
}

void HandleInputPlayField(SpriteData& spriteData, CharacterData& charData, PlayField &playField, Level &level) {
    switch(playField.mode) {
        case PlayFieldMode::None:handleInputPlayFieldNormal(playField, level);break;
        case PlayFieldMode::SelectingTile:handleInputPlayFieldSelectingTile(playField, level);break;
        case PlayFieldMode::SelectingEnemyTarget:handleInputPlayFieldSelectingEnemyTarget(playField, level);break;
        case PlayFieldMode::Explore:
            handleInputPlayFieldExploration(spriteData, charData, playField, level);break;
    }
}

void DrawPlayField(SpriteData& spriteData, CharacterData& charData, PlayField &playField, Level &level) {
    // Bottom layer
    BeginMode2D(level.camera.camera);
    DrawTileLayer(level.lighting, spriteData.sheet, level.tileMap, BOTTOM_LAYER, 0, 0);
    EndMode2D();

    DrawBloodPools();

    // Middle layer
    BeginMode2D(level.camera.camera);
    DrawTileLayer(level.lighting, spriteData.sheet, level.tileMap, MIDDLE_LAYER, 0, 0);
    DrawLevelObjects(spriteData, level);
    DrawTileLayer(level.lighting, spriteData.sheet, level.tileMap, LIGHT_LAYER, 0, 0);
    DrawDoors(spriteData, level);
    EndMode2D();


    BeginMode2D(level.camera.camera);
    DrawGridCharacters(spriteData, charData, playField, level);
    EndMode2D();


    DrawParticleManager(*playField.particleManager);

    // Top layer
    BeginMode2D(level.camera.camera);
    DrawTileLayer(level.lighting, spriteData.sheet, level.tileMap, TOP_LAYER, 0, 0);
    EndMode2D();

    //RenderLighting(level.lighting);
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
