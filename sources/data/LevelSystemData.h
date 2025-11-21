//
// Created by bison on 01-07-25.
//

#ifndef SANDBOX_LEVELSYSTEMDATA_H
#define SANDBOX_LEVELSYSTEMDATA_H

#include "PathFindingData.h"
#include "graphics/ParticleSystem.h"
#include "ActionData.h"

enum class LevelMode {
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

struct LevelSystemData {
    bool moving;
    LevelMode mode;
    Vector2 selectedTile;
    Vector2i selectedTilePos;
    int selectedCharacter;
    Path path;
    // Static variables for pulsing highlight
    float highlightAlpha = 0.25f; // Current alpha value
    bool increasing = true;       // Direction of the alpha
    float pulseSpeed = 8.0f; // Speed of the pulsing
    std::vector<CharacterMove> activeMoves;
    std::string hintText;
    PendingAction pendingAction;
};

#endif //SANDBOX_LEVELSYSTEMDATA_H
