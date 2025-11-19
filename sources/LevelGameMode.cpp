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

static GameData* game;
static Level level;
static ParticleManager particleManager;
static PlayField playField{};

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

    // Example: press UP/DOWN to control rain intensity
    if (IsKeyDown(KEY_UP))   data.weatherData.intensity = fminf(data.weatherData.intensity + dt, 1.0f);
    if (IsKeyDown(KEY_DOWN)) data.weatherData.intensity = fmaxf(data.weatherData.intensity - dt, 0.0f);
    UpdateWeather(data, level.outdoor, dt);

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
    UpdateInput(data.inputData, level.camera.camera);
    if(ProcessActions(data, level, playField, data.actionQueue, GetFrameTime())) {
        PopGameMode(data);
        return;
    }
    //processEvents(data);
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
            Color ambient = CalcOutdoorAmbientColorCubic(level.hourOfDay, data.weatherData.weatherType);
            // lightning flash
            if (data.weatherData.lightningActive) {
                float s = data.weatherData.lightningStrength;
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
        DrawTextEx(data.smallFont1, TextFormat("Weather intensity: %.2f", data.weatherData.intensity), (Vector2) {1, 24}, 5, 1,
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
