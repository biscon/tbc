//
// Created by bison on 29-01-25.
//

#ifndef SANDBOX_GAMEMODE_H
#define SANDBOX_GAMEMODE_H

enum class GameModes {
    Menu,
    Level,
};

struct GameMode {
    GameModes mode;
    void (*Init)();
    void (*Update)(float dt);
    void (*HandleInput)();
    void (*RenderLevel)();
    void (*RenderUi)();
    void (*PreRender)();
    void (*Destroy)();
    void (*Pause)();
    void (*Resume)();
};

struct GameModeFlags {
    bool quitGame = false;
};

void InitGameMode();
void CreateGameMode(GameModes gm, void (*Init)(), void (*Update)(float), void (*HandleInput)(), void (*RenderLevel)(), void (*RenderUi)(),
                    void (*PreRender)(), void (*Shutdown)(), void (*Pause)(), void (*Resume)());
GameMode* GetGameMode(GameModes gm);
void PopGameMode();
void PushGameMode(GameModes gm);
void UpdateGameMode(float dt);
void HandleInputGameMode();
void RenderLevelGameMode();
void RenderUiGameMode();
void PreRenderGameMode();
void DestroyGameMode();
void RequestQuitGame();
GameModeFlags& GetGameModeFlags();


#endif //SANDBOX_GAMEMODE_H
