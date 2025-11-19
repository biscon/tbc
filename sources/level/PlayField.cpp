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
#include "graphics/BloodPool.h"
#include "graphics/ParticleSystem.h"
#include "level/Combat.h"
#include "graphics/TileMap.h"
#include "ai/PathFinding.h"
#include "graphics/Lighting.h"
#include "ui/Icons.h"
#include "Weather.h"
#include "audio/Sound.h"
#include "game/Input.h"
#include "game/ActionSystem.h"

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

void CreatePlayField(PlayField &playField, ParticleManager* particleManager) {
    playField.particleManager = particleManager;
    playField.moving = false;
    playField.mode = PlayFieldMode::None;
    playField.selectedCharacter = -1;
    playField.selectedTile = {-1, -1};
    playField.path = {};
}

// Function to draw the health bar
void DrawHealthBar(float x, float y, float width, float health, float maxHealth) {
    // Draw the health bar background (gray)
    DrawRectangle(x, y, width, 2, GRAY);
    // Draw the health bar foreground (green for positive health, red for baseAttack)
    DrawRectangle(x, y, width * (health / maxHealth), 2, GREEN);
}

static void DrawGridCharacters(GameData& data, Level &level, PlayField& playField) {
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
        if(playField.mode != PlayFieldMode::Explore) {
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

static void CheckLevelExits(GameData& data, Level& level)
{
    for (InputEvent& evt : FilterEvents(data.inputData, true, InputEventType::MouseClick))
    {
        if (evt.mouse.button != MOUSE_LEFT_BUTTON) continue;

        Vector2 world = evt.mouse.worldPos;
        Vector2i clickGrid = PixelToGridPositionI((int)world.x, (int)world.y);

        for (auto& exit : level.exits) {

            // click inside exit rect?
            if (clickGrid.x < exit.x || clickGrid.x >= exit.x + exit.width ||
                clickGrid.y < exit.y || clickGrid.y >= exit.y + exit.height)
                continue;

            // check character proximity
            for (auto& c : level.partyCharacters) {

                Vector2i cPos = GetCharacterGridPosI(
                        data.spriteData,
                        data.charData.sprite[c]
                );

                bool close = false;

                for (int y = exit.y; y < exit.y + exit.height && !close; ++y)
                    for (int x = exit.x; x < exit.x + exit.width && !close; ++x)
                        if (Distance(cPos, {x, y}) <= 1.0f)
                            close = true;

                if (close) {
                    ConsumeEvent(evt);
                    PushExitLevel(data.actionQueue, exit.levelFile, exit.spawnPoint);
                    return;
                }
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

static bool playerInTheWay(GameData& data, LevelDoor& door) {
    for(auto& tile : door.blockedTiles) {
        for(auto& partyChar : data.party) {
            auto partyCharPos = GetCharacterGridPosI(data.spriteData, data.charData.sprite[partyChar]);
            if(tile == partyCharPos) {
                return true;
            }
        }
    }
    return false;
}

static bool handleDoors(GameData& data, Level &level, Vector2i playerPos, Vector2 mousePos) {
    SpriteData& spriteData = data.spriteData;

    for (auto& entry : level.doors) {
        auto& door = entry.second;

        // door world pixel rect
        Vector2 pos = GridToPixelPosition(door.gridPos.x, door.gridPos.y);
        auto frameInfo = GetFrameInfo(data.spriteData, door.animPlayer);

        Rectangle frameRectWorld = {
                pos.x - 8.0f,
                pos.y - 8.0f,
                frameInfo.srcRect.width,
                frameInfo.srcRect.height
        };

        // Cheap distance check
        if (Distance(playerPos, door.gridPos) >= 5)
            continue;

        // Hover sets the cursor icon
        if (CheckCollisionPointRec(mousePos, frameRectWorld)) {
            data.ui.currentCursorIcon = ICON_INTERACT;
        }

        // Now process input events
        for (auto& ev : FilterEvents(data.inputData, true, InputEventType::MouseClick)) {
            if (ev.mouse.button != MOUSE_LEFT_BUTTON) continue;

            // Check if click is on this door
            if (!CheckCollisionPointRec(ev.mouse.worldPos, frameRectWorld))
                continue;

            // Extra block: avoid opening doors the player is blocking
            if (playerInTheWay(data, door))
                continue;

            // ---- Handle door interaction ----
            ConsumeEvent(ev);
            DoorSaveState &doorState = data.levelState[level.name].doors[door.id];

            if (!doorState.open) {
                TraceLog(LOG_INFO, "Opening door %s", door.id.c_str());
                doorState.open = true;
                SetReverseSpriteAnimation(spriteData, door.animPlayer, false);
                ResumeSpriteAnimation(spriteData, door.animPlayer);
                SetFrame(spriteData, door.animPlayer, 0);
            } else {
                TraceLog(LOG_INFO, "Closing door %s", door.id.c_str());
                doorState.open = false;
                SetReverseSpriteAnimation(spriteData, door.animPlayer, true);
                int anim = spriteData.player.animationIdx[door.animPlayer];
                int frames = (int)spriteData.anim.frames[anim].size();
                SetFrame(spriteData, door.animPlayer, frames - 1);
                ResumeSpriteAnimation(spriteData, door.animPlayer);
            }

            // Update tile layers
            SetTiles(level.tileMap, door.blockedTiles, NAV_LAYER, doorState.open ? 0 : 1);
            SetTiles(level.tileMap, door.shadowTiles, SHADOW_LAYER, doorState.open ? 0 : 1);
            SetTiles(level.tileMap, door.shadowTiles, LIGHT_LAYER, doorState.open ? 0 : 1);

            PropagateLight(level.lighting, level.tileMap);

            return true;    // one door handled
        }
    }

    return false;
}

static bool handleObjects(GameData& data, Level &level, Vector2i playerPos)
{
    // iterate only unhandled left-click events
    for (InputEvent& evt : FilterEvents(data.inputData, true, InputEventType::MouseClick))
    {
        if (evt.mouse.button != MOUSE_LEFT_BUTTON)
            continue;

        Vector2 clickPos = evt.mouse.worldPos; // always use event’s world pos

        for (auto& entry : level.objects)
        {
            auto& obj = entry.second;

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
            if (!CheckCollisionPointRec(clickPos, frameRectWorld))
                continue;

            if (Distance(playerPos, obj.gridPos) >= 5)
                continue;

            // object has inventory?
            auto& state = data.levelState[level.name];
            if (state.objectInventories.count(obj.id) == 0)
                continue;

            int invId = state.objectInventories.at(obj.id);

            // cursor update
            data.ui.currentCursorIcon = ICON_INTERACT;

            // consume event & open loot
            ConsumeEvent(evt);
            PushOpenLootInventory(data.actionQueue, invId);
            return true;
        }
    }

    return false;
}

static bool handleMovementClick(GameData& data,
                                PlayField& playField,
                                Level& level,
                                Vector2i playerPos,
                                Vector2i gridPos)
{
    SpriteData& spriteData = data.spriteData;
    CharacterData& charData = data.charData;

    int playerChar = data.ui.selectedCharacter;

    // Construct player sprite position in pixel space
    Vector2i playerPixel = {
            (int)GetCharacterSpritePosX(spriteData, charData.sprite[playerChar]),
            (int)GetCharacterSpritePosY(spriteData, charData.sprite[playerChar])
    };

    Path path;
    if (!CalcPath(spriteData, charData, level, path,
                  PixelToGridPositionI(playerPixel.x, playerPixel.y),
                  gridPos,
                  playerChar,
                  IsTileOccupiedEnemies))
    {
        return false;
    }

    // highlight tile
    playField.selectedTilePos = gridPos;

    // handle left-click event
    for (InputEvent& evt : FilterEvents(data.inputData, true, InputEventType::MouseClick))
    {
        if (evt.mouse.button != MOUSE_LEFT_BUTTON) continue;

        ConsumeEvent(evt);
        PushMoveParty(data.actionQueue, gridPos);
        return true;
    }

    return false;
}

static bool handleDialogueClick(GameData& data,
                                PlayField& playField,
                                Level& level,
                                Vector2i playerPos,
                                Vector2i gridPos)
{
    SpriteData& spriteData = data.spriteData;
    CharacterData& charData = data.charData;

    for (int npcId : level.npcCharacters)
    {
        Vector2i npcPos = GetCharacterGridPosI(spriteData, charData.sprite[npcId]);

        if (npcPos != gridPos)
            continue;

        // NPC found on clicked tile
        if (Distance(playerPos, npcPos) < 3)
        {
            playField.hintText = "Talk to " + charData.name[npcId];
            data.ui.currentCursorIcon = ICON_TALK;

            // click-to-talk
            for (InputEvent& evt : FilterEvents(data.inputData, true, InputEventType::MouseClick))
            {
                if (evt.mouse.button != MOUSE_LEFT_BUTTON) continue;

                ConsumeEvent(evt);
                PushInitiateDialogue(data.actionQueue, npcId, level.npcDialogueNodeIds[npcId]);
                return true;
            }
        }
        else
        {
            playField.hintText = "Too far away!";
        }
    }

    return false;
}



static void showExits(GameData& data, Level &level, Vector2 mousePos) {
    for(auto& exit : level.exits){
        Vector2 pos = GridToPixelPosition(exit.x, exit.y);
        Rectangle frameRectWorld = {pos.x - 8.0f, pos.y - 8.0f, (float) exit.width * 16, (float) exit.height * 16};
        if(CheckCollisionPointRec(mousePos, frameRectWorld)) {
            data.ui.currentCursorIcon = ICON_EXIT;
            return;
        }
    }
}

static void handleInputPlayFieldExploration(GameData& data,
                                            PlayField &playField,
                                            Level &level)
{
    SpriteData& spriteData = data.spriteData;
    CharacterData& charData = data.charData;

    playField.selectedTilePos = {-1, -1};

    Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), level.camera.camera);
    Vector2i gridPos = PixelToGridPositionI((int) mouseWorld.x, (int) mouseWorld.y);

    int playerChar = data.ui.selectedCharacter;
    Vector2i playerPos =
            GetCharacterGridPosI(spriteData, charData.sprite[playerChar]);

    // Show exits and check them (existing logic)
    showExits(data, level, mouseWorld);
    if (level.turnState == TurnState::None) {
        CheckLevelExits(data, level);
    }

    // Objects take priority
    if (handleObjects(data, level, playerPos))
        return;

    // Doors next
    if (handleDoors(data, level, playerPos, mouseWorld))
        return;

    // Movement or dialogue
    if (!IsTileOccupied(spriteData, charData, level,
                        gridPos.x, gridPos.y, -1))
    {
        // movement handler
        if (handleMovementClick(data, playField, level, playerPos, gridPos))
            return;
    }
    else
    {
        // dialogue handler
        if (handleDialogueClick(data, playField, level, playerPos, gridPos))
            return;
    }
}


void HandleInputPlayField(GameData& data, PlayField &playField, Level &level) {
    if(playField.mode == PlayFieldMode::Explore) {
        handleInputPlayFieldExploration(data, playField, level);
    }
}

void DrawPlayField(GameData& data, PlayField &playField, Level &level) {
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
    DrawGridCharacters(data, level, playField);

    //DrawWeather(level.weather, {200, 200, 255, 120});
    EndMode2D();

    DrawParticleManager(*playField.particleManager);

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
