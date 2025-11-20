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
#include "level/CombatEngine.h"
#include "ui/Dialogue.h"
#include "ui/PartySideBar.h"
#include "ui/Inventory.h"
#include "level/LevelCamera.h"
#include "graphics/Lighting.h"
#include "ui/ActionBar.h"
#include "ui/LootInventory.h"
#include "ai/PathFinding.h"
#include "level/Weather.h"
#include "game/Input.h"
#include "game/ActionSystem.h"

/*
static GameData* game;
static Level level;
static ParticleManager particleManager;
static PlayField playField{};
*/

void LevelInit(GameData& data) {
    GameMode* gm = GetGameMode(GameModes::Level);
    auto* state = static_cast<LevelGameModeState*>(gm->userData);
    
    CreateLevel(state->level);
    CreateLevelScreen(data);
    CreateParticleManager(state->particleManager, {0, 0}, gameScreenWidth, gameScreenHeight);

    CreatePlayField(state->playField, &state->particleManager);

    InitBloodRendering();
    InitInventory(data);
    InitActionBar(data);
}

void LevelDestroy(GameData& data) {
    GameMode* gm = GetGameMode(GameModes::Level);
    auto* state = static_cast<LevelGameModeState*>(gm->userData);
    
    DestroyParticleManager(state->particleManager);
    DestroyBloodRendering();
    DestroyLevelScreen(data);
    DestroyLevel(data.spriteData.sheet, state->level);
}


static void UpdateWorld(GameData& data, Level& level, PlayField& playField, float dt) {
    //level.hourOfDay = 1;
    if(IsKeyDown(KEY_SPACE)) {
        level.hourOfDay += dt;
        if (level.hourOfDay > 24)
            level.hourOfDay = 0;
    }

    // Example: press UP/DOWN to control rain intensity
    if (IsKeyDown(KEY_UP))   data.weatherData.intensity = fminf(data.weatherData.intensity + dt, 1.0f);
    if (IsKeyDown(KEY_DOWN)) data.weatherData.intensity = fmaxf(data.weatherData.intensity - dt, 0.0f);

    UpdateCamera(level.camera, dt);

    UpdateCombat(data, level, playField, dt);
    UpdateLevelScreen(data, level, dt);
    UpdatePlayField(data, playField, level, dt);
}

static void UpdateUI(GameData& data, float dt) {
    UpdateDialogue(data, dt);
    UpdatePartySideBar(data, dt);
    UpdateInventory(data, dt);
    UpdateLootInventory(data, dt);
    if(data.ui.showActionBar) {
        UpdateActionBar(data, dt);
    }

}

static void UpdateLighting(GameData& data, Level& level, float dt) {
    if(!level.outdoor) {
        UpdateVisibilityMap(data, level);
        UpdateVisibilityTexture(level.lighting);
    }
    Vector2i partyPos = GetCharacterGridPosI(data.spriteData, data.charData.sprite[data.ui.selectedCharacter]);
    if(!level.lighting.lights.empty()) {
        level.lighting.lights[0].x = partyPos.x;
        level.lighting.lights[0].y = partyPos.y;
    }
}

static void UpdateFX(GameData& data, Level& level, ParticleManager& particleManager, float dt) {
    UpdateParticleManager(particleManager, dt);
    UpdateWeather(data, level.outdoor, dt);
}

void LevelUpdate(GameData& data, float dt) {
    GameMode* gm = GetGameMode(GameModes::Level);
    auto* state = static_cast<LevelGameModeState*>(gm->userData);

    data.shaderTime += dt;
    UpdateWorld(data, state->level, state->playField, dt);
    UpdateFX(data, state->level, state->particleManager, dt);
    UpdateUI(data, dt);
    UpdateLighting(data, state->level, dt);
}

static bool HandleGlobalInput(GameData& data) {
    if (IsKeyPressed(KEY_ESCAPE) && (data.state != GameState::INVENTORY && data.state != GameState::LOOT_INVENTORY)) {
        PopGameMode(data);
        return true;
    }
    return false;
}

static bool HandleUIInput(GameData& data, Level& level, PlayField& playField) {
    if(data.state == GameState::DIALOGUE) {
        HandleDialogueInput(data);
        return true;
    }
    if(data.state == GameState::INVENTORY) {
        HandleInventoryInput(data);
        HandlePartySideBarInput(data);
        return true;
    }
    if(data.state == GameState::LOOT_INVENTORY) {
        HandleLootInventoryInput(data);
        return true;
    }
    if(data.ui.showActionBar) {
        if(HandleActionBarInput(data, level, playField)) {
            return true;
        }
    }
    if(HandlePartySideBarInput(data)) {
        return true;
    }
    return false;
}

static void HandleLevelMouseInput(GameData& data, Level& level, PlayField& playField) {
    HandleInputPlayField(data, playField, level);
    HandleInputLevelScreen(data, level, playField);
}

void LevelHandleInput(GameData& data) {
    GameMode* gm = GetGameMode(GameModes::Level);
    auto* state = static_cast<LevelGameModeState*>(gm->userData);
    
    UpdateInput(data.inputData, state->level.camera.camera);

    if(ProcessActions(data, state->level, state->playField, data.actionQueue, GetFrameTime())) {
        PopGameMode(data);
        return;
    }

    if (HandleGlobalInput(data))  return;
    if (HandleUIInput(data, state->level, state->playField))      return;

    HandleCameraInput(state->level.camera);
    HandleLevelMouseInput(data, state->level, state->playField);
}


void LevelRenderLevel(GameData& data) {
    GameMode* gm = GetGameMode(GameModes::Level);
    auto* state = static_cast<LevelGameModeState*>(gm->userData);
    
    Level& level = state->level;
    
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
            Color ambient = CalcOutdoorAmbientColorCubic(level.hourOfDay, data.weatherData.weatherType);
            // lightning flash
            if (data.weatherData.lightningActive) {
                float s = data.weatherData.lightningStrength;
                ambient = LerpColor(ambient, {220, 220, 255, 255}, s);
            }
            level.lighting.ambient = ambient;
        }
        PropagateLight(level.lighting, level.tileMap);
        DrawPlayField(data, state->playField, level);
        EndScissorMode();
    }
}

void LevelRenderUi(GameData& data) {
    GameMode* gm = GetGameMode(GameModes::Level);
    auto* state = static_cast<LevelGameModeState*>(gm->userData);
    
    DrawLevelScreen(data, state->level, state->playField);
    RenderPartySideBarUI(data);
    if(data.ui.showActionBar && data.state != GameState::INVENTORY) {
        RenderActionBarUI(data);
    }
    RenderFloatingStats(data, state->level);
    RenderDialogueUI(data);
    if(data.state == GameState::INVENTORY) {
        RenderInventoryUI(data);
    }
    if(data.state == GameState::LOOT_INVENTORY) {
        RenderLootInventoryUI(data);
    }
    if(data.settingsData.showFPS) {
        Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), state->level.camera.camera);
        Vector2i gridPos = PixelToGridPositionI(mousePos.x, mousePos.y);
        DrawTextEx(data.smallFont1, TextFormat("GridPos: %i,%i", gridPos.x, gridPos.y), (Vector2) {1, 8}, 5, 1,
                   YELLOW);
        DrawTextEx(data.smallFont1, TextFormat("Time: %.2f", state->level.hourOfDay), (Vector2) {1, 16}, 5, 1,
                   YELLOW);
        DrawTextEx(data.smallFont1, TextFormat("Weather intensity: %.2f", data.weatherData.intensity), (Vector2) {1, 24}, 5, 1,
                   YELLOW);
    }
}

void LevelPreRender(GameData& data) {
    GameMode* gm = GetGameMode(GameModes::Level);
    auto* state = static_cast<LevelGameModeState*>(gm->userData);
    
    PreRenderBloodPools(state->level);
    PreRenderParticleManager(state->level.lighting, state->particleManager, state->level.camera.camera);
}

void LevelPause(GameData& data) {

}

void LevelResume(GameData& data) {
    GameMode* gm = GetGameMode(GameModes::Level);
    auto* state = static_cast<LevelGameModeState*>(gm->userData);
    TraceLog(LOG_INFO, "LevelResume");
    Level& level = state->level;

    if(data.state == GameState::LOAD_LEVEL) {
        LoadLevel(data, level, data.levelFileName);
        AddPartyToLevel(data.spriteData, data.charData, level, data.party, "default");
        StartCameraPanToTargetCharTime(data.spriteData, data.charData, level.camera, data.party[0], 0.01f);
        data.state = GameState::PLAY_LEVEL;
        state->playField.mode = PlayFieldMode::Explore;
    }
    if(data.state == GameState::LOAD_LEVEL_FROM_SAVE) {
        LoadLevel(data, level, data.levelFileName);
        AddPartyToLevelNoPositioning(data.spriteData, data.charData, level, data.party);
        StartCameraPanToTargetCharTime(data.spriteData, data.charData, level.camera, data.party[0], 0.01f);
        data.state = GameState::PLAY_LEVEL;
        state->playField.mode = PlayFieldMode::Explore;
    }
}

void SetupLevelGameMode(LevelGameModeState *state) {
    CreateGameMode(GameModes::Level, LevelInit, LevelUpdate, LevelHandleInput, LevelRenderLevel, LevelRenderUi, LevelPreRender, LevelDestroy, LevelPause, LevelResume, state);
}
