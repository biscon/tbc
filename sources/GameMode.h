//
// Created by bison on 29-01-25.
//

#ifndef SANDBOX_GAMEMODE_H
#define SANDBOX_GAMEMODE_H

enum class GameModes {
    Menu,
    Combat,
    Dungeon,
};

struct GameMode {
    GameModes mode;
    void (*Init)();
    void (*Update)(float dt);
    void (*HandleInput)();
    void (*Render)();
    void (*PreRender)();
    void (*Destroy)();
};

void CreateGameMode(GameModes gm, void (*Init)(), void (*Update)(float dt), void (*HandleInput)(), void (*Render)(), void (*PreRender)(), void (*Shutdown)());
GameMode* GetGameMode(GameModes gm);
void SetGameMode(GameModes gm);
void UpdateGameMode(float dt);
void HandleInputGameMode();
void RenderGameMode();
void PreRenderGameMode();
void DestroyGameMode();


#endif //SANDBOX_GAMEMODE_H
