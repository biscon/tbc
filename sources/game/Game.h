//
// Created by bison on 18-02-25.
//

#ifndef SANDBOX_GAME_H
#define SANDBOX_GAME_H

#include "data/GameData.h"

void CreateGame(GameData &game, const std::string& levelFileName);
void DestroyGame(GameData &game);
#endif //SANDBOX_GAME_H
