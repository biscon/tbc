//
// Created by bison on 29-01-25.
//

#include <cassert>
#include "LevelGameMode.h"
#include "character/Character.h"
#include "level/Level.h"
#include "level/LevelScreen.h"
#include "graphics/BloodPool.h"
#include "rcamera.h"
#include "util/GameEventQueue.h"
#include "level/CombatEngine.h"
#include "ui/Dialogue.h"
#include "graphics/Animation.h"
#include "ui/PartySideBar.h"
#include "ui/Inventory.h"
#include "level/LevelCamera.h"
#include "graphics/Lighting.h"
#include "ui/ActionBar.h"
#include "ui/Icons.h"
#include "ui/LootInventory.h"
#include "ai/PathFinding.h"
#include "level/Weather.h"
#include "audio/Sound.h"

static GameData* game;
static Level level;
static ParticleManager particleManager;
static PlayField playField{};

static void moveParty(Vector2i target) {
    TraceLog(LOG_INFO, "MoveParty event,target: %d,%d", target.x, target.y);
    playField.activeMoves.clear();
    MoveCharacter(*game, playField, level, game->ui.selectedCharacter, target);
    // move the rest partially
    for(int i = 0; i < (int)level.partyCharacters.size(); i++) {
        if(level.partyCharacters[i] != game->ui.selectedCharacter) {
            MoveCharacterPartial(*game, playField, level, level.partyCharacters[i], target);
        }
    }
}

static void processEvents(GameData& data) {
    GameEvent event{};
    while(GetNextEvent(game->ui.eventQueue, event)) {
        switch(event.type) {
            case GameEventType::MoveParty: {
                StartCameraPanToTilePos(level.camera, event.moveParty.target, 250.0f);
                moveParty(event.moveParty.target);
                StopSfx(data.soundData, level.footStepsHandle);
                level.footStepsHandle = PlaySfx(data.soundData, "footstep", true);

                /*
                Animation anim{};
                SetupFancyTextAnimation(anim,
                                        "The system is compromised!",
                                        300,
                                        2.0f,   // holdDuration
                                        0.5f,     // initialDelay
                                        0.05f,    // letterPause (reveal speed)
                                        1.0f);    // fadeOut
                level.animations.push_back(anim);
                 */
                break;
            }
            case GameEventType::PartySpotted: {
                game->ui.inCombat = true;
                playField.mode = PlayFieldMode::None;
                StartCombat(game->spriteData, game->charData, level, event.partySpotted.spotter);
                break;
            }
            case GameEventType::EndCombat: {
                level.turnState = TurnState::None;
                playField.mode = PlayFieldMode::Explore;
                game->ui.inCombat = false;
                for(auto& c : level.partyCharacters) {
                    // Set initial animation to paused
                    CharacterSprite& sprite = game->charData.sprite[c];
                    StartPausedCharacterSpriteAnim(game->spriteData, sprite, SpriteAnimationType::WalkDown, true);
                    game->charData.orientation[c] = Orientation::Down;
                    game->charData.statusEffects[c].clear();
                    // revive dead party chars
                    if(game->charData.stats[c].HP <= 0) {
                        game->charData.stats[c].HP = CalculateCharHealth(game->charData.stats[c]);
                        SetCharacterSpriteRotation(game->spriteData, sprite, 0);
                    }
                }
                break;
            }
            case GameEventType::ExitLevel: {
                TraceLog(LOG_INFO, "ExitLevel: %s, spawnPoint: %s", event.exitLevelEvent.levelFile, event.exitLevelEvent.spawnPoint);
                game->levelFileName = std::string(event.exitLevelEvent.levelFile);
                ResetPlayField(playField);
                LoadLevel(*game, level, game->levelFileName);
                std::string spawnPoint = std::string(event.exitLevelEvent.spawnPoint);
                AddPartyToLevel(game->spriteData, game->charData, level, game->party, spawnPoint);
                StartCameraPanToTargetCharTime(game->spriteData, game->charData, level.camera, game->party[0], 0.01f);
                game->state = GameState::PLAY_LEVEL;
                playField.mode = PlayFieldMode::Explore;
                break;
            }
            case GameEventType::InitiateDialogue: {
                playField.mode = PlayFieldMode::None;
                game->state = GameState::DIALOGUE;
                TraceLog(LOG_INFO, "InitiateDialogue: npcId = %i, dialogueNodeId = %i", event.initiateDialogueEvent.npcId, event.initiateDialogueEvent.dialogueNodeId);
                InitiateDialogue(*game, event.initiateDialogueEvent.dialogueNodeId, event.initiateDialogueEvent.npcId);
                break;
            }
            case GameEventType::EndDialogue: {
                TraceLog(LOG_INFO, "EndDialogue: npcId = %i", event.endDialogueEvent.npcId);
                playField.mode = PlayFieldMode::Explore;
                game->state = GameState::PLAY_LEVEL;
                break;
            }
            case GameEventType::StartQuest: {
                const Quest& quest = game->questData.quests[event.startQuestEvent.questId];
                Animation textAnim1{};
                Animation textAnim2{};
                SetupFancyTextAnimation(textAnim1, "Quest started:", 10, 285, 2.0f, 0.5f, 0.05f, 1.0f);
                SetupFancyTextAnimation(textAnim2, quest.title.c_str(), 20, 300, 2.0f, 1.0f, 0.05f, 1.0f);
                level.animations.push_back(textAnim1);
                level.animations.push_back(textAnim2);
                break;
            }
            case GameEventType::OpenInventory: {
                //playField.mode = PlayFieldMode::None;
                game->state = GameState::INVENTORY;
                game->ui.selectedCharacter = event.openInventoryEvent.charId;
                //InitInventory(*game);
                break;
            }
            case GameEventType::CloseInventory: {
                //playField.mode = PlayFieldMode::Explore;
                game->state = GameState::PLAY_LEVEL;
                break;
            }
            case GameEventType::OpenMenu: {
                PopGameMode(*game);
                break;
            }
            case GameEventType::OpenActionBar: {
                game->ui.showActionBar = true;
                // preselect move
                game->ui.actionBar.selectedActionIdx = 0;
                game->ui.actionBar.selectedModeIdx = 0;
                ExecuteAction(*game, ActionBarAction::Move, level, playField, true);
                break;
            }
            case GameEventType::CloseActionBar: {
                game->ui.showActionBar = false;
                break;
            }
            case GameEventType::OpenLootInventory: {
                data.state = GameState::LOOT_INVENTORY;
                data.ui.lootInventory.inventoryId = event.openLootInventoryEvent.invId;
                InitLootInventory(data);
                break;
            }
            case GameEventType::CloseLootInventory: {
                //playField.mode = PlayFieldMode::Explore;
                data.state = GameState::PLAY_LEVEL;
                break;
            }
            default:
                break;
        }
    }
}

static void handleCameraMovement() {
    float dt = GetFrameTime();
    float speed = 8.0f;
    float accelerationTime = 0.75f;
    float decelerationTime = 0.75f;

    float acceleration = speed / accelerationTime;
    float deceleration = speed / decelerationTime;

    // X-axis movement (A/D)
    if (!level.camera.cameraLockX) {
        if (IsKeyDown(KEY_A)) {
            level.camera.cameraVelocity.x -= acceleration * dt;
            if (level.camera.cameraVelocity.x < -speed) {
                level.camera.cameraVelocity.x = -speed;
            }
        } else if (IsKeyDown(KEY_D)) {
            level.camera.cameraVelocity.x += acceleration * dt;
            if (level.camera.cameraVelocity.x > speed) {
                level.camera.cameraVelocity.x = speed;
            }
        } else {
            // Decelerate when no input
            if (level.camera.cameraVelocity.x > 0.0f) {
                level.camera.cameraVelocity.x -= deceleration * dt;
                if (level.camera.cameraVelocity.x < 0.0f) level.camera.cameraVelocity.x = 0.0f;
            } else if (level.camera.cameraVelocity.x < 0.0f) {
                level.camera.cameraVelocity.x += deceleration * dt;
                if (level.camera.cameraVelocity.x > 0.0f) level.camera.cameraVelocity.x = 0.0f;
            }
        }
    } else {
        // Reset velocity if axis is locked
        level.camera.cameraVelocity.x = 0.0f;
    }

    // Y-axis movement (W/S)
    if (!level.camera.cameraLockY) {
        if (IsKeyDown(KEY_W)) {
            level.camera.cameraVelocity.y -= acceleration * dt;
            if (level.camera.cameraVelocity.y < -speed) {
                level.camera.cameraVelocity.y = -speed;
            }
        } else if (IsKeyDown(KEY_S)) {
            level.camera.cameraVelocity.y += acceleration * dt;
            if (level.camera.cameraVelocity.y > speed) {
                level.camera.cameraVelocity.y = speed;
            }
        } else {
            // Decelerate when no input
            if (level.camera.cameraVelocity.y > 0.0f) {
                level.camera.cameraVelocity.y -= deceleration * dt;
                if (level.camera.cameraVelocity.y < 0.0f) level.camera.cameraVelocity.y = 0.0f;
            } else if (level.camera.cameraVelocity.y < 0.0f) {
                level.camera.cameraVelocity.y += deceleration * dt;
                if (level.camera.cameraVelocity.y > 0.0f) level.camera.cameraVelocity.y = 0.0f;
            }
        }
    } else {
        // Reset velocity if axis is locked
        level.camera.cameraVelocity.y = 0.0f;
    }
}

void LevelInit(GameData& data) {
    CreateLevel(level);
    CreateLevelScreen(data);
    CreateParticleManager(particleManager, {0, 0}, gameScreenWidth, gameScreenHeight);

    CreatePlayField(playField, &particleManager);

    InitBloodRendering();
    InitInventory(data);
    InitActionBar(data);
}

void LevelDestroy(GameData& data) {
    DestroyParticleManager(particleManager);
    DestroyBloodRendering();
    DestroyLevelScreen(data);
    DestroyLevel(data.spriteData.sheet, level);
}

void LevelUpdate(GameData& data, float dt) {
    data.shaderTime += dt;

    if(IsKeyDown(KEY_SPACE)) {
        level.hourOfDay += dt;
        if (level.hourOfDay > 24)
            level.hourOfDay = 0;
    }

    //level.hourOfDay = 1;

    if(level.outdoor) {
        // Example: press UP/DOWN to control rain intensity
        if (IsKeyDown(KEY_UP))   level.weather.intensity = fminf(level.weather.intensity + dt, 1.0f);
        if (IsKeyDown(KEY_DOWN)) level.weather.intensity = fmaxf(level.weather.intensity - dt, 0.0f);
        UpdateWeather(level.weather, dt);
    }

    UpdateCamera(level.camera, dt);
    UpdateCombat(*game, level, playField, dt);
    UpdateParticleManager(particleManager, dt);
    UpdateLevelScreen(*game, level, dt);
    UpdatePlayField(*game, playField, level, dt);
    UpdateDialogue(*game, dt);
    UpdatePartySideBar(*game, dt);
    UpdateInventory(*game, dt);
    UpdateLootInventory(*game, dt);
    if(game->ui.showActionBar) {
        UpdateActionBar(*game, dt);
    }

    if(!level.outdoor) {
        UpdateVisibilityMap(*game, level);
        UpdateVisibilityTexture(level.lighting);
    }
    Vector2i partyPos = GetCharacterGridPosI(data.spriteData, data.charData.sprite[data.ui.selectedCharacter]);
    if(!level.lighting.lights.empty()) {
        level.lighting.lights[0].x = partyPos.x;
        level.lighting.lights[0].y = partyPos.y;
    }
    //PropagateLight(level.lighting, level.tileMap);
}

void LevelHandleInput(GameData& data) {
    processEvents(data);
    if (IsKeyPressed(KEY_ESCAPE) && (game->state != GameState::INVENTORY && game->state != GameState::LOOT_INVENTORY)) {
        PopGameMode(data);
        return;
    }
    if(game->state == GameState::DIALOGUE) {
        HandleDialogueInput(*game);
        return;
    }
    if(game->state == GameState::INVENTORY) {
        HandleInventoryInput(*game);
        HandlePartySideBarInput(*game);
        return;
    }
    if(game->state == GameState::LOOT_INVENTORY) {
        HandleLootInventoryInput(*game);
        return;
    }
    handleCameraMovement();
    if(game->ui.showActionBar) {
        if(HandleActionBarInput(*game, level, playField)) {
            return;
        }
    }
    if(HandlePartySideBarInput(*game)) {
       return;
    }
    HandleInputPlayField(*game, playField, level);
    HandleInputLevelScreen(*game, level, playField);
}

void LevelRenderLevel(GameData& data) {
    /*
    int charId = game->party[0];
    Vector2i pos = GetCharacterGridPosI(game->spriteData, game->charData.sprite[charId]);
    MoveLight(level.lighting.lights[0], pos.x, pos.y);
     */

    ClearBackground(BLACK);

    Vector2 topLeftWorld = { 0, 0 };
    Vector2 bottomRightWorld = {
            (float)level.camera.worldWidth,
            (float)level.camera.worldHeight
    };

// Convert world bounds to screen coordinates using the camera
    Vector2 topLeftScreen = GetWorldToScreen2D(topLeftWorld, level.camera.camera);
    Vector2 bottomRightScreen = GetWorldToScreen2D(bottomRightWorld, level.camera.camera);

// Compute screen-space rectangle
    int scissorX = (int)topLeftScreen.x;
    int scissorY = (int)topLeftScreen.y;
    int scissorW = (int)(bottomRightScreen.x - topLeftScreen.x);
    int scissorH = (int)(bottomRightScreen.y - topLeftScreen.y);

// Clamp to stay within screen bounds
    if (scissorX < 0) {
        scissorW += scissorX;
        scissorX = 0;
    }
    if (scissorY < 0) {
        scissorH += scissorY;
        scissorY = 0;
    }
    if (scissorX + scissorW > gameScreenWidth) {
        scissorW = gameScreenWidth - scissorX;
    }
    if (scissorY + scissorH > gameScreenHeight) {
        scissorH = gameScreenHeight - scissorY;
    }

    // Only apply scissor if there's a valid area
    if (scissorW > 0 && scissorH > 0) {
        BeginScissorMode(scissorX, scissorY, scissorW, scissorH);
        if(level.outdoor) {
            Color ambient = CalcOutdoorAmbientColorCubic(level.hourOfDay);
            // lightning flash
            if (level.weather.lightningActive) {
                float s = level.weather.lightningStrength;
                ambient = LerpColor(ambient, {220, 220, 255, 255}, s);
            }
            level.lighting.ambient = ambient;
        }
        PropagateLight(level.lighting, level.tileMap);
        DrawPlayField(*game, playField, level);
        EndScissorMode();
    }
}

void LevelRenderUi(GameData& data) {
    DrawLevelScreen(*game, level, playField);
    RenderPartySideBarUI(*game);
    if(game->ui.showActionBar && game->state != GameState::INVENTORY) {
        RenderActionBarUI(*game);
    }
    RenderFloatingStats(*game, level);
    RenderDialogueUI(*game);
    if(game->state == GameState::INVENTORY) {
        RenderInventoryUI(*game);
    }
    if(game->state == GameState::LOOT_INVENTORY) {
        RenderLootInventoryUI(*game);
    }
    if(data.settingsData.showFPS) {
        Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), level.camera.camera);
        Vector2i gridPos = PixelToGridPositionI(mousePos.x, mousePos.y);
        DrawTextEx(data.smallFont1, TextFormat("GridPos: %i,%i", gridPos.x, gridPos.y), (Vector2) {1, 8}, 5, 1,
                   YELLOW);
        DrawTextEx(data.smallFont1, TextFormat("Time: %.2f", level.hourOfDay), (Vector2) {1, 16}, 5, 1,
                   YELLOW);
        DrawTextEx(data.smallFont1, TextFormat("Weather intensity: %.2f", level.weather.intensity), (Vector2) {1, 24}, 5, 1,
                   YELLOW);
    }
}

void LevelPreRender(GameData& data) {
    PreRenderBloodPools(level);
    PreRenderParticleManager(level.lighting, particleManager, level.camera.camera);
}

void LevelPause(GameData& data) {

}

void LevelResume(GameData& data) {
    TraceLog(LOG_INFO, "LevelResume");

    if(game->state == GameState::LOAD_LEVEL) {
        LoadLevel(*game, level, game->levelFileName);
        AddPartyToLevel(game->spriteData, game->charData, level, game->party, "default");
        StartCameraPanToTargetCharTime(game->spriteData, game->charData, level.camera, game->party[0], 0.01f);
        game->state = GameState::PLAY_LEVEL;
        playField.mode = PlayFieldMode::Explore;
    }
    if(game->state == GameState::LOAD_LEVEL_FROM_SAVE) {
        LoadLevel(*game, level, game->levelFileName);
        AddPartyToLevelNoPositioning(game->spriteData, game->charData, level, game->party);
        StartCameraPanToTargetCharTime(game->spriteData, game->charData, level.camera, game->party[0], 0.01f);
        game->state = GameState::PLAY_LEVEL;
        playField.mode = PlayFieldMode::Explore;
    }
}

void SetupLevelGameMode(GameData* gameState) {
    game = gameState;
    CreateGameMode(GameModes::Level, LevelInit, LevelUpdate, LevelHandleInput, LevelRenderLevel, LevelRenderUi, LevelPreRender, LevelDestroy, LevelPause, LevelResume);
}
