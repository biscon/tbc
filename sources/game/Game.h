//
// Created by bison on 18-02-25.
//

#ifndef SANDBOX_GAME_H
#define SANDBOX_GAME_H

#include "character/Character.h"

enum class GameState {
    START_NEW_GAME,
    LOAD_LEVEL,
    PLAY_LEVEL
};

struct Game {
    std::vector<int> party;
    std::string levelFileName;
    GameState state;
    WeaponData weaponData;
    CharacterData charData;
    SpriteData spriteData;
};

void CreateGame(Game &game, const std::string& levelFileName);
void DestroyGame(Game &game);

#endif //SANDBOX_GAME_H
