//
// Created by bison on 16-01-25.
//

#ifndef SANDBOX_GRID_H
#define SANDBOX_GRID_H

#include "util/MathUtil.h"
#include "ai/PathFinding.h"
#include "graphics/TileMap.h"
#include "graphics/ParticleSystem.h"


enum class GridMode {
    Normal,
    SelectingTile,
    SelectingEnemyTarget,
};

struct GridState {
    bool moving;
    GridMode mode;
    Vector2 selectedTile;
    Character* selectedCharacter;
    Path path;
    // Static variables for pulsing highlight
    float highlightAlpha = 0.25f; // Current alpha value
    bool increasing = true;       // Direction of the alpha
    float pulseSpeed = 4.0f; // Speed of the pulsing
    Character* floatingStatsCharacter;
    ParticleManager* particleManager;
};

void InitGrid(GridState &gridState, ParticleManager* particleManager);
void UpdateGrid(GridState &gridState, LevelState &combat, float dt);
void DrawGrid(GridState &gridState, LevelState &combat);
void SetInitialGridPositions(GridState &gridState, LevelState &combat);

#endif //SANDBOX_GRID_H
