//
// Created by bison on 16-01-25.
//

#ifndef SANDBOX_GRID_H
#define SANDBOX_GRID_H

#include "Combat.h"
#include "mathutil.h"

const int GRID_WIDTH = 30;
const int GRID_HEIGHT = 13;
const float GRID_CENTER_OFFSET = 8.0f; // Offset for sprite alignment

enum class GridMode {
    Normal,
    SelectingTile,
    SelectingEnemyTarget,
};

struct GridPath {
    std::vector<Vector2i> path;
    int cost; // cost of the path in distance travelled
    int currentStep;
    float moveTime;
    float moveSpeed;
};

struct GridState {
    SpriteAnimationManager* animationManager;
    bool moving;
    GridMode mode;
    Vector2 selectedTile;
    Character* selectedCharacter;
    GridPath path;
    // Static variables for pulsing highlight
    float highlightAlpha = 0.25f; // Current alpha value
    bool increasing = true;       // Direction of the alpha
    float pulseSpeed = 4.0f; // Speed of the pulsing
    Character* floatingStatsCharacter;
};

void InitGrid(GridState &gridState, SpriteAnimationManager &animationManager);
void UpdateGrid(GridState &gridState, CombatState &combat, float dt);
void DrawGrid(GridState &gridState, CombatState &combat);
void SetInitialGridPositions(GridState &gridState, CombatState &combat);

bool IsTileOccupied(CombatState &combat, int x, int y, Character* exceptCharacter);
bool IsTileWalkable(CombatState &combat, int x, int y);
bool InitPath(CombatState &combat, GridPath &path, Vector2i start, Vector2i end);
bool InitPathIgnoreOccupied(CombatState &combat, GridPath &path, Vector2i start, Vector2i end);

#endif //SANDBOX_GRID_H
