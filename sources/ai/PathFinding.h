//
// Created by bison on 21-01-25.
//

#ifndef SANDBOX_PATHFINDING_H
#define SANDBOX_PATHFINDING_H

#include "combat/CombatState.h"
#include "util/MathUtil.h"

const int GRID_WIDTH = 30;
const int GRID_HEIGHT = 13;
const float GRID_CENTER_OFFSET = 8.0f; // Offset for sprite alignment

struct Path {
    std::vector<Vector2i> path;
    int cost; // cost of the path in distance travelled
    int currentStep;
    float moveTime;
    float moveSpeed;
};

Vector2 PixelToGridPosition(float pixelX, float pixelY);
Vector2i PixelToGridPositionI(int pixelX, int pixelY);
Vector2 GridToPixelPosition(int gridX, int gridY);
bool IsTileOccupied(CombatState &combat, int x, int y, Character* exceptCharacter);
bool IsTileWalkable(CombatState &combat, int x, int y);
bool InitPath(CombatState &combat, Path &path, Vector2i start, Vector2i end, Character *exceptCharacter);
bool InitPathWithRange(CombatState &combat, Path &path, Vector2i start, Vector2i end, int range, Character *exceptCharacter);
bool InitPathWithRangePartial(CombatState &combat, Path &path, Vector2i start, Vector2i end, int range, Character *exceptCharacter);
bool InitPathIgnoreOccupied(CombatState &combat, Path &path, Vector2i start, Vector2i end);
bool HasLineOfSight(CombatState &combat, Vector2i start, Vector2i end);
bool IsCharacterAdjacentToPlayer(Character &player, Character &character);
std::vector<Vector2i> FindFreePositionsCircular(CombatState &combat, int x, int y, int radius);
std::vector<Character*> GetTargetsInLine(CombatState &combat, Vector2i start, Vector2 direction, int range, Character* exceptCharacter);

#endif //SANDBOX_PATHFINDING_H
