//
// Created by bison on 16-01-25.
//

#ifndef SANDBOX_GRID_H
#define SANDBOX_GRID_H

#include "Combat.h"
#include "MathUtil.h"
#include "PathFinding.h"
#include "TileMap.h"
#include "ParticleSystem.h"


enum class GridMode {
    Normal,
    SelectingTile,
    SelectingEnemyTarget,
};

struct GridState {
    SpriteAnimationManager* animationManager;
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
    TileMap* tileMap;
    ParticleManager* particleManager;
};

void InitGrid(GridState &gridState, SpriteAnimationManager &animationManager, ParticleManager* particleManager);
void UpdateGrid(GridState &gridState, CombatState &combat, float dt);
void DrawGrid(GridState &gridState, CombatState &combat);
void SetInitialGridPositions(GridState &gridState, CombatState &combat);

#endif //SANDBOX_GRID_H
