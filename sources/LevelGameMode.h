//
// Created by bison on 29-01-25.
//

#ifndef SANDBOX_LEVELGAMEMODE_H
#define SANDBOX_LEVELGAMEMODE_H

#include "GameMode.h"
#include "game/Game.h"

struct LevelGameModeState {
    Level level;
    ParticleManager particleManager;
    PlayField playField;
};

void SetupLevelGameMode(LevelGameModeState* state);

#endif //SANDBOX_LEVELGAMEMODE_H
