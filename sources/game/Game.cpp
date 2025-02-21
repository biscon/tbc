//
// Created by bison on 18-02-25.
//

#include "Game.h"

void CreateGame(Game &game, const std::string& levelFileName) {
    game.state = GameState::START_NEW_GAME;
    game.levelFileName = levelFileName;
}

void DestroyGame(Game &game) {

}
