//
// Created by bison on 20-11-25.
//

#include <climits>
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
#include "graphics/CharSprite.h"

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
        if (CalcPath(spriteData, charData, level, path, PixelToGridPositionI((int) GetCharSpritePosX(spriteData, charData.sprite[level.currentCharacter]),
                                                                             (int) GetCharSpritePosY(spriteData, charData.sprite[level.currentCharacter])),
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
    Vector2i start = GetCharGridPosI(data.spriteData, data.charData.sprite[data.ui.selectedCharacter]);
    Vector2i end = GetCharGridPosI(data.spriteData, data.charData.sprite[targetId]);
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
        Vector2i charPos = GetCharGridPosI(data.spriteData, data.charData.sprite[character]);
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
            auto partyCharPos = GetCharGridPosI(data.spriteData, data.charData.sprite[partyChar]);
            if(tile == partyCharPos) {
                return true;
            }
        }
    }
    return false;
}

static Vector2i ChooseDoorInteractionPos(GameData& data,
                                         Level& level,
                                         const LevelDoor& door,
                                         Vector2i playerPos)
{
    SpriteData& spriteData = data.spriteData;
    CharacterData& charData = data.charData;
    int playerChar = data.ui.selectedCharacter;

    Vector2i bestPos = { -1, -1 };
    int bestCost = INT_MAX;
    float bestDist = 999999.0f;

    // --- Helper lambda so we don't repeat as much boilerplate ---
    auto tryPos = [&](Vector2i targetPos)
    {
        if (targetPos.x == -1) return; // undefined → ignore

        Path path;

        if (!CalcPath(spriteData,
                      charData,
                      level,
                      path,
                      playerPos,       // start
                      targetPos,       // target
                      playerChar,
                      IsTileOccupiedEnemies))
        {
            return; // unreachable → ignore
        }

        // Prefer lowest cost. If equal cost, prefer shortest Euclidean.
        float dist = Distance(playerPos, targetPos);

        if (path.cost < bestCost ||
            (path.cost == bestCost && dist < bestDist))
        {
            bestCost = path.cost;
            bestDist = dist;
            bestPos  = targetPos;
        }
    };

    // Try both candidates
    tryPos(door.interactionPos1);
    tryPos(door.interactionPos2);

    return bestPos;
}

static bool handleDoors(GameData& data, Level& level, Vector2i playerPos)
{
    SpriteData& spriteData = data.spriteData;

    // Iterate unhandled left-click events
    for (auto& ev : FilterEvents(data.inputData, true, InputEventType::MouseClick))
    {
        if (ev.mouse.button != MOUSE_LEFT_BUTTON)
            continue;

        Vector2 clickPos = ev.mouse.worldPos;

        // Check against all doors
        for (auto& entry : level.doors)
        {
            auto& door = entry.second;

            // Compute world rect for click detection
            Vector2 pos = GridToPixelPosition(door.gridPos.x, door.gridPos.y);
            auto frameInfo = GetFrameInfo(spriteData, door.animPlayer);

            Rectangle frameRectWorld = {
                    pos.x - 8.0f,
                    pos.y - 8.0f,
                    frameInfo.srcRect.width,
                    frameInfo.srcRect.height
            };

            // Click not on this door?
            if (!CheckCollisionPointRec(clickPos, frameRectWorld))
                continue;

            // Avoid door interactions if the player is physically overlapping the door
            if (playerInTheWay(data, door))
                continue;

            // Determine which interaction tile (A or B) is reachable
            Vector2i interactPos =
                    ChooseDoorInteractionPos(data, level, door, playerPos);

            // Neither side reachable
            if (interactPos.x == -1)
                continue;

            ConsumeEvent(ev);

            // Check if already close enough (1-tile radius)
            if (IsAnyPartyMemberNear(data, level, interactPos, 1.0f))
            {
                ClearPendingAction(data);
                PushDoorInteract(data.actionQueue, door.id);
                return true;
            }
            else
            {
                // ---- TOO FAR → MOVE FIRST, THEN INTERACT ----
                PushMoveParty(data.actionQueue, interactPos);

                SetPendingAction(
                        data,
                        GameAction{
                                ActionType::DoorInteract,
                                DoorInteractAction{ door.id }
                        }
                );

                return true;
            }
        }
    }

    return false;
}

static bool handleObjects(GameData& data, Level& level, Vector2i playerPos)
{
    for (InputEvent& evt : FilterEvents(data.inputData, true, InputEventType::MouseClick))
    {
        if (evt.mouse.button != MOUSE_LEFT_BUTTON)
            continue;

        Vector2 clickPos = evt.mouse.worldPos;

        for (auto& entry : level.objects)
        {
            LevelObject& obj = entry.second;

            // Compute world-space sprite rectangle
            Vector2 pos = GridToPixelPosition(obj.gridPos.x, obj.gridPos.y);
            auto frameInfo = GetFrameInfo(data.spriteData, obj.animPlayer);

            Rectangle frameRectWorld = {
                    pos.x - 8.0f,
                    pos.y - 8.0f,
                    frameInfo.srcRect.width,
                    frameInfo.srcRect.height
            };

            // Click must hit object sprite
            if (!CheckCollisionPointRec(clickPos, frameRectWorld))
                continue;

            // Object must have inventory
            auto& state = data.levelState[level.name];
            if (state.objectInventories.count(obj.id) == 0)
                continue;

            int invId = state.objectInventories.at(obj.id);

            // -----------------------------
            //    Interaction distance
            // -----------------------------
            // Use interactionPos if defined, otherwise just use object gridPos.
            Vector2i interactPos = obj.interactionPos;

            bool hasDefinedInteractionPos =
                    !(interactPos.x == -1 && interactPos.y == -1);

            if (!hasDefinedInteractionPos)
            {
                // if no interactionPos defined AND player too far → do nothing
                if (Distance(playerPos, obj.gridPos) >= 5)
                    continue;

                // in range: open immediately
                ClearPendingAction(data);
                ConsumeEvent(evt);
                PushOpenLootInventory(data.actionQueue, invId);
                return true;
            }

            // ---------------------------------------
            //   interactionPos *is defined*
            // ---------------------------------------
            const float maxDist = 1.0f;

            if (IsAnyPartyMemberNear(data, level, interactPos, maxDist))
            {
                // Already in range → open immediately
                ClearPendingAction(data);
                ConsumeEvent(evt);
                PushOpenLootInventory(data.actionQueue, invId);
                return true;
            }
            else
            {
                // Too far → schedule move + pending open-loot
                ConsumeEvent(evt);
                PushMoveParty(data.actionQueue, interactPos);
                SetPendingAction(
                        data,
                        GameAction{
                                ActionType::OpenLootInventory,
                                OpenLootInventoryAction{ invId }
                        }
                );

                return true;
            }
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

    Path path;
    if (!CalcPath(spriteData, charData, level, path,
                  playerPos,
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
        ClearPendingAction(data);
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
        Vector2i npcPos = GetCharGridPosI(spriteData, charData.sprite[npcId]);

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
                ClearPendingAction(data);
                ConsumeEvent(evt);
                PushInitiateDialogue(data.actionQueue, level.npcDialogueNodeIds[npcId]);
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
        if (evt.mouse.button != MOUSE_LEFT_BUTTON)
            continue;

        Vector2 world = evt.mouse.worldPos;
        Vector2i clickGrid = PixelToGridPositionI((int)world.x, (int)world.y);

        for (auto& exit : level.exits)
        {
            // Is click inside exit rect?
            if (clickGrid.x < exit.x || clickGrid.x >= exit.x + exit.width ||
                clickGrid.y < exit.y || clickGrid.y >= exit.y + exit.height)
                continue;

            // Click hits the exit → consume event
            ConsumeEvent(evt);

            // -----------------------------
            //   Handle undefined interactPos
            // -----------------------------
            Vector2i interactPos = exit.interactionPos;
            bool hasDefinedInteractionPos =
                    !(interactPos.x == -1 && interactPos.y == -1);

            if (!hasDefinedInteractionPos)
            {
                // undefined interactionPos means "do nothing"
                return true;
            }

            // -----------------------------
            //   Defined interactionPos path
            // -----------------------------

            const float maxDist = 1.0f;

            if (IsAnyPartyMemberNear(data, level, interactPos, maxDist))
            {
                ClearPendingAction(data);
                // Already in range → fire exit immediately
                PushExitLevel(data.actionQueue, exit.levelFile, exit.spawnPoint, exit.onEnterFunc);
                return true;
            }
            else
            {
                // Out of range → move + pending action
                PushMoveParty(data.actionQueue, interactPos);
                SetPendingAction(
                        data,
                        GameAction{
                                ActionType::ExitLevel,
                                ExitLevelAction{
                                        exit.levelFile,
                                        exit.spawnPoint,
                                        exit.onEnterFunc
                                }
                        }
                );

                return true;
            }
        }
    }
    return false;
}

static bool handleTriggers(GameData& data, Level& level)
{
    for (InputEvent& evt : FilterEvents(data.inputData, true, InputEventType::MouseClick))
    {
        if (evt.mouse.button != MOUSE_LEFT_BUTTON)
            continue;

        Vector2 world = evt.mouse.worldPos;
        Vector2i clickGrid = PixelToGridPositionI((int)world.x, (int)world.y);

        for (auto& entry : level.triggers)
        {
            LevelTrigger& tr = entry.second;
            // Is click inside exit rect?
            if (clickGrid.x < tr.x || clickGrid.x >= tr.x + tr.width ||
                clickGrid.y < tr.y || clickGrid.y >= tr.y + tr.height)
                continue;

            // Click hits the exit → consume event
            ConsumeEvent(evt);

            // -----------------------------
            //   Handle undefined interactPos
            // -----------------------------
            Vector2i interactPos = tr.interactionPos;
            bool hasDefinedInteractionPos =
                    !(interactPos.x == -1 && interactPos.y == -1);

            if (!hasDefinedInteractionPos)
            {
                // undefined interactionPos means "do nothing"
                return true;
            }

            // -----------------------------
            //   Defined interactionPos path
            // -----------------------------

            const float maxDist = 1.0f;

            if (IsAnyPartyMemberNear(data, level, interactPos, maxDist))
            {
                ClearPendingAction(data);
                PushActivateTrigger(data.actionQueue, tr.id);
                return true;
            }
            else
            {
                // Out of range → move + pending action
                PushMoveParty(data.actionQueue, interactPos);
                SetPendingAction(
                        data,
                        GameAction{
                                ActionType::ActivateTrigger,
                                ActivateTriggerAction{ tr.id }
                        }
                );

                return true;
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
    Vector2i playerPos = GetCharGridPosI(spriteData, charData.sprite[playerChar]);

    if (level.turnState == TurnState::None) {
        if(handleExits(data, level))
            return;
    }

    // Objects take priority
    if (handleObjects(data, level, playerPos))
        return;

    // Doors next
    if (handleDoors(data, level, playerPos))
        return;

    // Doors next
    if (handleTriggers(data, level))
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