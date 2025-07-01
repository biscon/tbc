//
// Created by bison on 01-07-25.
//

#ifndef SANDBOX_PLAYFIELDDATA_H
#define SANDBOX_PLAYFIELDDATA_H

#include "PathFindingData.h"
#include "graphics/ParticleSystem.h"

enum class PlayFieldMode {
    None,
    SelectingTile,
    SelectingEnemyTarget,
    Explore,
};

struct CharacterMove {
    int character;
    Path path;
    bool isDone;
};

struct PlayField {
    bool moving;
    PlayFieldMode mode;
    Vector2 selectedTile;
    Vector2i selectedTilePos;
    int selectedCharacter;
    Path path;
    // Static variables for pulsing highlight
    float highlightAlpha = 0.25f; // Current alpha value
    bool increasing = true;       // Direction of the alpha
    float pulseSpeed = 4.0f; // Speed of the pulsing
    ParticleManager* particleManager;
    std::vector<CharacterMove> activeMoves;
    std::string hintText;
};

#endif //SANDBOX_PLAYFIELDDATA_H
