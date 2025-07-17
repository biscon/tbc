//
// Created by bison on 29-01-25.
//

#ifndef SANDBOX_GAMEMODE_H
#define SANDBOX_GAMEMODE_H

#include "data/GameData.h"

enum class GameModes {
    Menu,
    Level,
};

struct GameMode {
    GameModes mode;
    void (*Init)(GameData& data);
    void (*Update)(GameData& data, float dt);
    void (*HandleInput)(GameData& data);
    void (*RenderLevel)(GameData& data);
    void (*RenderUi)(GameData& data);
    void (*PreRender)(GameData& data);
    void (*Destroy)(GameData& data);
    void (*Pause)(GameData& data);
    void (*Resume)(GameData& data);
};

struct GameModeFlags {
    bool quitGame = false;
};

void InitGameMode(GameData& data);
void CreateGameMode(GameModes gm, void (*Init)(GameData&), void (*Update)(GameData&, float), void (*HandleInput)(GameData&), void (*RenderLevel)(GameData&), void (*RenderUi)(GameData&),
                    void (*PreRender)(GameData&), void (*Shutdown)(GameData&), void (*Pause)(GameData&), void (*Resume)(GameData&));
GameMode* GetGameMode(GameModes gm);
void PopGameMode(GameData& data);
void PushGameMode(GameData& data, GameModes gm);
void UpdateGameMode(GameData& data, float dt);
void HandleInputGameMode(GameData& data);
void RenderLevelGameMode(GameData& data);
void RenderUiGameMode(GameData& data);
void PreRenderGameMode(GameData& data);
void DestroyGameMode(GameData& data);
void RequestQuitGame();
GameModeFlags& GetGameModeFlags();


#endif //SANDBOX_GAMEMODE_H
