//
// Created by bison on 29-01-25.
//

#include <unordered_map>
#include <stack>
#include "GameMode.h"
#include "raylib.h"

static std::unordered_map<GameModes, GameMode> gameModes;
static std::stack<GameMode*> gameModeStack;
static GameModeFlags gameModeFlags;

static float fadeAlpha = 0.0f;
static bool fading = false;
static bool fadeIn = false;
static const float fadeSpeed = 2.75f;
static GameMode* pendingGameMode = nullptr;

void StartFadeOut(GameMode* newMode) {
    fading = true;
    fadeIn = false;
    fadeAlpha = 0.0f;
    pendingGameMode = newMode;
}

void StartFadeIn() {
    fading = true;
    fadeIn = true;
    fadeAlpha = 1.0f;
}

void CreateGameMode(GameModes gm, void (*Init)(), void (*Update)(float), void (*HandleInput)(), void (*Render)(),
                    void (*PreRender)(), void (*Shutdown)(), void (*Pause)(), void (*Resume)()) {
    GameMode mode{};
    mode.Init = Init;
    mode.Update = Update;
    mode.HandleInput = HandleInput;
    mode.Render = Render;
    mode.PreRender = PreRender;
    mode.Destroy = Shutdown;
    mode.Pause = Pause;
    mode.Resume = Resume;
    gameModes[gm] = mode;
}

GameMode* GetGameMode(GameModes gm) {
    if (gameModes.find(gm) == gameModes.end()) {
        return nullptr;
    }
    return &gameModes[gm];
}

void PushGameMode(GameModes gm) {
    GameMode* mode = GetGameMode(gm);
    if (mode == nullptr) {
        TraceLog(LOG_ERROR, "GameMode not found: %d", gm);
        return;
    }
    if (!gameModeStack.empty()) {
        TraceLog(LOG_INFO, "Pausing current game mode");
        gameModeStack.top()->Pause();
        StartFadeOut(mode);
    } else {
        gameModeStack.push(mode);
        TraceLog(LOG_INFO, "Pushing and resuming game mode: %d", gm);
        mode->Resume();
        StartFadeIn();
    }
}

void PopGameMode() {
    if (!gameModeStack.empty()) {
        TraceLog(LOG_INFO, "Pausing current game mode");
        gameModeStack.top()->Pause();
        StartFadeOut(nullptr);
    }
}

void UpdateGameMode(float dt) {
    if (!gameModeStack.empty()) {
        gameModeStack.top()->Update(dt);
    }

    if (fading) {
        fadeAlpha += (fadeIn ? -fadeSpeed : fadeSpeed) * dt;
        if (fadeAlpha >= 1.0f && !fadeIn) {
            // Mid-transition: switch game modes
            if (pendingGameMode) {
                gameModeStack.push(pendingGameMode);
                TraceLog(LOG_INFO, "Switching to new game mode");
                pendingGameMode->Resume();
            } else {
                gameModeStack.pop();
                if (!gameModeStack.empty()) {
                    TraceLog(LOG_INFO, "Resuming previous game mode");
                    gameModeStack.top()->Resume();
                } else {
                    TraceLog(LOG_INFO, "No game mode, quitting game.");
                    RequestQuitGame();
                }
            }
            StartFadeIn();
        }
        if (fadeAlpha <= 0.0f && fadeIn) {
            fading = false;
            fadeAlpha = 0.0f;
            pendingGameMode = nullptr;
        }
    }
}

void HandleInputGameMode() {
    if (!gameModeStack.empty()) {
        gameModeStack.top()->HandleInput();
    }
}

void RenderGameMode() {
    if (!gameModeStack.empty()) {
        gameModeStack.top()->Render();
    }

    // Render fade effect
    if (fading || fadeAlpha > 0.0f) {
        DrawRectangle(0, 0, 480, 270, Fade(BLACK, fadeAlpha));
    }
}

void PreRenderGameMode() {
    if (!gameModeStack.empty()) {
        gameModeStack.top()->PreRender();
    }
}

void DestroyGameMode() {
    for (auto& mode : gameModes) {
        mode.second.Destroy();
    }
}

GameModeFlags& GetGameModeFlags() {
    return gameModeFlags;
}

void RequestQuitGame() {
    gameModeFlags.quitGame = true;
}

void InitGameMode() {
    gameModeFlags.quitGame = false;
    for (auto& mode : gameModes) {
        mode.second.Init();
    }
}
