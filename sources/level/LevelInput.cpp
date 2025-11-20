//
// Created by bison on 20-11-25.
//

#include "LevelInput.h"

#include "raylib.h"

#include "character/Character.h"
#include "Combat.h"
#include "ai/PathFinding.h"
#include "LevelCamera.h"
#include "audio/Sound.h"
#include "graphics/TileMap.h"
#include "graphics/Lighting.h"
#include "ui/Icons.h"
#include "game/Input.h"
#include "game/ActionSystem.h"
#include "LevelSystem.h"

static void HandleInputPathSelection(GameData& data, Level &level) {
    SpriteData& spriteData = data.spriteData;
    CharacterData& charData = data.charData;
    // check if mouse is over tile
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), level.camera.camera);
    Vector2 gridPos = PixelToGridPosition(mousePos.x, mousePos.y);
    data.ui.actionBar.previewApUse = -1;
    data.ui.level.validMovePath = false;
    if (!IsTileOccupied(spriteData, charData, level, static_cast<int>(gridPos.x), static_cast<int>(gridPos.y), -1)) {
        data.levelData.selectedTile = gridPos;
        // calculate a path and draw it as lines
        Path& path = data.ui.level.movePath;
        CharacterStats& stats = charData.stats[level.currentCharacter];
        Vector2i target = PixelToGridPositionI(static_cast<int>(mousePos.x), static_cast<int>(mousePos.y));
        if (CalcPath(spriteData, charData, level, path, PixelToGridPositionI((int) GetCharacterSpritePosX(spriteData, charData.sprite[level.currentCharacter]),
                                                                             (int) GetCharacterSpritePosY(spriteData, charData.sprite[level.currentCharacter])),
                     target, level.currentCharacter, IsTileOccupied)) {

            if (path.cost <= stats.AP) {
                data.ui.level.validMovePath = true;
                data.ui.actionBar.previewApUse = path.cost;
            }
            // Check for a mouse click
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && path.cost <= stats.AP) {
                data.levelData.mode = LevelMode::None;
                data.levelData.path = path;
                data.levelData.moving = true;
                stats.AP -= path.cost;
                // cap at zero
                if (stats.AP < 0) {
                    stats.AP = 0;
                }
                level.turnState = TurnState::Move;
                if(level.footStepsHandle != -1)
                    StopSfx(data.soundData, level.footStepsHandle);
                level.footStepsHandle = PlaySfx(data.soundData, "footstep", true);
                StartCameraPanToTargetPos(level.camera, mousePos, 250.0f);
            }
        }
    }
}

static void HandleMeleeTargetSelection(GameData& data, Level& level, WeaponTemplate* weaponTemplate, int targetId) {
    if (IsCharacterAdjacentToPlayer(data.spriteData, data.charData, data.ui.selectedCharacter, targetId)) {
        data.levelData.selectedCharacter = targetId;
        int weaponItemId = GetSelectedWeaponItemId(data, data.ui.selectedCharacter);
        CalcHitChance(data, data.ui.selectedCharacter, weaponItemId, -1, data.ui.level.attackInfo);
        data.ui.actionBar.previewApUse = data.ui.level.attackInfo.apCost;
        int ap = data.charData.stats[data.ui.selectedCharacter].AP;
        // Check for a mouse click
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && ap >= data.ui.level.attackInfo.apCost) {
            level.turnState = TurnState::Waiting;
            level.waitTime = 0.25f;
            level.selectedCharacter = data.levelData.selectedCharacter;
            level.nextState = TurnState::Attack;
            ResetLevelSystem(data.levelData);
        }
    }
}

static void HandleRangedTargetSelection(GameData& data, Level& level, WeaponTemplate* weaponTemplate, int targetId) {
    Vector2i start = GetCharacterGridPosI(data.spriteData, data.charData.sprite[data.ui.selectedCharacter]);
    Vector2i end = GetCharacterGridPosI(data.spriteData, data.charData.sprite[targetId]);
    if(HasLineOfSightFriendlies(data, level, start, end, weaponTemplate->range, data.ui.selectedCharacter)) {
        int weaponItemId = GetSelectedWeaponItemId(data, data.ui.selectedCharacter);
        CalcHitChance(data, data.ui.selectedCharacter, weaponItemId, data.ui.actionBar.selectedModeIdx, data.ui.level.attackInfo);
        data.ui.actionBar.previewApUse = data.ui.level.attackInfo.apCost;
        data.levelData.selectedCharacter = targetId;
        int ap = data.charData.stats[data.ui.selectedCharacter].AP;
        auto* weaponInstance = GetSelectedWeaponInstance(data, data.ui.selectedCharacter);
        int currentAmmo = weaponInstance->currentAmmo;

        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            if(currentAmmo < data.ui.level.attackInfo.ammoCost) {
                PlaySfx(data.soundData, "gunEmpty");
            } else {
                if (ap >= data.ui.level.attackInfo.apCost) {
                    level.turnState = TurnState::Waiting;
                    level.waitTime = 0.25f;
                    level.selectedCharacter = data.levelData.selectedCharacter;
                    level.nextState = TurnState::AttackRanged;
                    ResetLevelSystem(data.levelData);
                }
            }
        }
    }
}

static void HandleInputTargetSelection(GameData& data, Level& level, bool onlyEnemies) {
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), level.camera.camera);
    Vector2i gridPos = PixelToGridPositionI(mousePos.x, mousePos.y);
    data.levelData.selectedCharacter = -1;
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
                HandleMeleeTargetSelection(data, level, weaponTemplate, character);
            }
            else {
                switch (weaponTemplate->type) {
                    case WeaponType::Melee:
                        HandleMeleeTargetSelection(data, level, weaponTemplate, character);
                        break;
                    case WeaponType::Ranged:
                        HandleRangedTargetSelection(data, level, weaponTemplate, character);
                        break;
                }
            }
        }
    }
}

void HandleInputCombat(GameData& data, Level &level) {
    if(data.state == GameState::PLAY_LEVEL) {
        if (data.levelData.mode == LevelMode::SelectingTile) {
            HandleInputPathSelection(data, level);
        }
        if (data.levelData.mode == LevelMode::SelectingEnemyTarget) {
            HandleInputTargetSelection(data, level, true);
        }
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

            // consume event & open loot
            ConsumeEvent(evt);
            PushOpenLootInventory(data.actionQueue, invId);
            return true;
        }
    }

    return false;
}

static bool handleMovementClick(GameData& data,
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
    data.levelData.selectedTilePos = gridPos;

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
            data.levelData.hintText = "Talk to " + charData.name[npcId];
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
            data.levelData.hintText = "Too far away!";
        }
    }

    return false;
}

static bool handleExits(GameData& data, Level& level)
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
                    return true;
                }
            }
        }
    }
    return false;
}

void HandleInputRealtime(GameData& data, Level &level) {
    if(data.levelData.mode != LevelMode::Explore) {
        return;
    }
    SpriteData& spriteData = data.spriteData;
    CharacterData& charData = data.charData;

    data.levelData.selectedTilePos = {-1, -1};

    Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), level.camera.camera);
    Vector2i gridPos = PixelToGridPositionI((int) mouseWorld.x, (int) mouseWorld.y);

    int playerChar = data.ui.selectedCharacter;
    Vector2i playerPos = GetCharacterGridPosI(spriteData, charData.sprite[playerChar]);

    if (level.turnState == TurnState::None) {
        if(handleExits(data, level))
            return;
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
        if (handleMovementClick(data, level, playerPos, gridPos))
            return;
    }
    else
    {
        // dialogue handler
        if (handleDialogueClick(data, level, playerPos, gridPos))
            return;
    }
}