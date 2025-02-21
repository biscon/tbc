//
// Created by bison on 21-01-25.
//

#ifndef SANDBOX_PATHFINDING_H
#define SANDBOX_PATHFINDING_H

#include <vector>
#include "util/MathUtil.h"
#include "level/Level.h"

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
bool IsTileOccupied(Level &level, int x, int y, Character* exceptCharacter);
bool IsTileWalkable(Level &level, int x, int y);
bool InitPath(Level &level, Path &path, Vector2i start, Vector2i end, Character *exceptCharacter);
bool InitPathWithRange(Level &level, Path &path, Vector2i start, Vector2i end, int range, Character *exceptCharacter);
bool InitPathWithRangePartial(Level &level, Path &path, Vector2i start, Vector2i end, int range, Character *exceptCharacter);
bool InitPathIgnoreOccupied(Level &level, Path &path, Vector2i start, Vector2i end);
bool HasLineOfSight(Level &level, Vector2i start, Vector2i end);
bool HasLineOfSight(Level &level, Vector2i start, Vector2i end, int maxDist);
bool IsCharacterAdjacentToPlayer(Character &player, Character &character);
std::vector<Vector2i> FindFreePositionsCircular(Level &level, int x, int y, int radius);
std::vector<Character*> GetTargetsInLine(Level &level, Vector2i start, Vector2 direction, int range, Character* exceptCharacter);

#endif //SANDBOX_PATHFINDING_H
