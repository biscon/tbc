//
// Created by bison on 29-01-25.
//

#include <unordered_map>
#include "GameMode.h"
#include "raylib.h"

static std::unordered_map<GameModes, GameMode> gameModes;
static GameMode* currentGameMode = nullptr;

void
CreateGameMode(GameModes gm, void (*Init)(), void (*Update)(float), void (*HandleInput)(), void (*Render)(),
               void (*PreRender)(), void (*Shutdown)()) {
    GameMode mode{};
    mode.Init = Init;
    mode.Update = Update;
    mode.HandleInput = HandleInput;
    mode.Render = Render;
    mode.PreRender = PreRender;
    mode.Destroy = Shutdown;
    gameModes[gm] = mode;
}

GameMode *GetGameMode(GameModes gm) {
    // check if present
    if(gameModes.find(gm) == gameModes.end()) {
        return nullptr;
    }
    return &gameModes[gm];
}

void SetGameMode(GameModes gm) {
    GameMode* mode = GetGameMode(gm);
    if(mode == nullptr) {
        TraceLog(LOG_ERROR, "GameMode not found: %d", gm);
        return;
    }
    if(currentGameMode != nullptr) {
        TraceLog(LOG_INFO, "Destroying current game mode");
        currentGameMode->Destroy();
    }
    currentGameMode = mode;
    TraceLog(LOG_INFO, "Setting game mode: %d", gm);
    currentGameMode->Init();
}

void UpdateGameMode(float dt) {
    if(currentGameMode != nullptr) {
        currentGameMode->Update(dt);
    }
}

void HandleInputGameMode() {
    if(currentGameMode != nullptr) {
        currentGameMode->HandleInput();
    }
}

void RenderGameMode() {
    if(currentGameMode != nullptr) {
        currentGameMode->Render();
    }
}

void PreRenderGameMode() {
    if(currentGameMode != nullptr) {
        currentGameMode->PreRender();
    }
}

void DestroyGameMode() {
    if(currentGameMode != nullptr) {
        currentGameMode->Destroy();
    }
}
