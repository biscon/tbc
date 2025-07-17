//
// Created by bison on 29-01-25.
//

#include <unordered_map>
#include <stack>
#include "GameMode.h"
#include "raylib.h"
#include "data/GameData.h"
#include "ui/Icons.h"

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

void CreateGameMode(GameModes gm, void (*Init)(GameData&), void (*Update)(GameData&, float), void (*HandleInput)(GameData&), void (*RenderLevel)(GameData&), void (*RenderUi)(GameData&),
                    void (*PreRender)(GameData&), void (*Shutdown)(GameData&), void (*Pause)(GameData&), void (*Resume)(GameData&)) {
    GameMode mode{};
    mode.Init = Init;
    mode.Update = Update;
    mode.HandleInput = HandleInput;
    mode.RenderLevel = RenderLevel;
    mode.RenderUi = RenderUi;
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

void PushGameMode(GameData& data, GameModes gm) {
    GameMode* mode = GetGameMode(gm);
    if (mode == nullptr) {
        TraceLog(LOG_ERROR, "GameMode not found: %d", gm);
        return;
    }
    if (!gameModeStack.empty()) {
        TraceLog(LOG_INFO, "Pausing current game mode");
        gameModeStack.top()->Pause(data);
        StartFadeOut(mode);
    } else {
        gameModeStack.push(mode);
        TraceLog(LOG_INFO, "Pushing and resuming game mode: %d", gm);
        mode->Resume(data);
        StartFadeIn();
    }
}

void PopGameMode(GameData& data) {
    if (!gameModeStack.empty()) {
        TraceLog(LOG_INFO, "Pausing current game mode");
        gameModeStack.top()->Pause(data);
        StartFadeOut(nullptr);
    }
}

void UpdateGameMode(GameData& data, float dt) {
    if (!gameModeStack.empty()) {
        gameModeStack.top()->Update(data, dt);
    }

    if (fading) {
        fadeAlpha += (fadeIn ? -fadeSpeed : fadeSpeed) * dt;
        if (fadeAlpha >= 1.0f && !fadeIn) {
            // Mid-transition: switch game modes
            if (pendingGameMode) {
                gameModeStack.push(pendingGameMode);
                TraceLog(LOG_INFO, "Switching to new game mode");
                pendingGameMode->Resume(data);
            } else {
                gameModeStack.pop();
                if (!gameModeStack.empty()) {
                    TraceLog(LOG_INFO, "Resuming previous game mode");
                    gameModeStack.top()->Resume(data);
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

void HandleInputGameMode(GameData& data) {
    data.ui.currentCursorIcon = ICON_CURSOR;
    if (!gameModeStack.empty()) {
        gameModeStack.top()->HandleInput(data);
    }
}

void RenderLevelGameMode(GameData& data) {
    if (!gameModeStack.empty()) {
        gameModeStack.top()->RenderLevel(data);
    }

    // Render fade effect
    if (fading || fadeAlpha > 0.0f) {
        DrawRectangle(0, 0, gameScreenWidth, gameScreenHeight, Fade(BLACK, fadeAlpha));
    }
}

void RenderUiGameMode(GameData& data) {
    if (!gameModeStack.empty()) {
        gameModeStack.top()->RenderUi(data);
    }

    // Render fade effect
    if (fading || fadeAlpha > 0.0f) {
        DrawRectangle(0, 0, gameScreenWidth, gameScreenHeight, Fade(BLACK, fadeAlpha));
    }

    if(data.ui.currentCursorIcon != -1 && IsCursorOnScreen()) {
        Vector2 mouse = GetMousePosition();
        if(data.ui.currentCursorIcon != ICON_CURSOR) {
            mouse.x -= 8;
            mouse.y -= 8;
        }
        DrawIcon(data, (int) mouse.x, (int) mouse.y, WHITE, data.ui.currentCursorIcon);
    }
}

void PreRenderGameMode(GameData& data) {
    if (!gameModeStack.empty()) {
        gameModeStack.top()->PreRender(data);
    }
}

void DestroyGameMode(GameData& data) {
    for (auto& mode : gameModes) {
        mode.second.Destroy(data);
    }
}

GameModeFlags& GetGameModeFlags() {
    return gameModeFlags;
}

void RequestQuitGame() {
    gameModeFlags.quitGame = true;
}

void InitGameMode(GameData& data) {
    gameModeFlags.quitGame = false;
    for (auto& mode : gameModes) {
        mode.second.Init(data);
    }
}
