//
// Created by bison on 21-01-25.
//

#ifndef SANDBOX_PATHFINDING_H
#define SANDBOX_PATHFINDING_H

#define CHECK_TILE_FUNC bool (*checkTile)(SpriteData& spriteData, CharacterData& charData, Level &level, int x, int y, int exceptCharacter)

#include "level/Level.h"
#include "data/PathFindingData.h"

Vector2 PixelToGridPosition(float pixelX, float pixelY);
Vector2i PixelToGridPositionI(int pixelX, int pixelY);
Vector2 GridToPixelPosition(int gridX, int gridY);
bool IsTileOccupied(SpriteData& spriteData, CharacterData& charData, Level &level, int x, int y, int exceptCharacter);
bool IsTileOccupiedEnemies(SpriteData& spriteData, CharacterData& charData, Level &level, int x, int y, int exceptCharacter);
bool IsTileWalkable(Level &level, int x, int y);
bool CalcPath(SpriteData& spriteData, CharacterData& charData, Level &level, Path &path, Vector2i start, Vector2i end, int exceptCharacter, CHECK_TILE_FUNC);
bool CalcPathWithRange(SpriteData& spriteData, CharacterData& charData, Level &level, Path &path, Vector2i start, Vector2i end, int range, int exceptCharacter, CHECK_TILE_FUNC);
bool CalcPathWithRangePartial(SpriteData& spriteData, CharacterData& charData, Level &level, Path &path, Vector2i start, Vector2i end, int range, int exceptCharacter, CHECK_TILE_FUNC);
bool CalcPathIgnoreOccupied(Level &level, Path &path, Vector2i start, Vector2i end);
bool HasLineOfSight(Level &level, Vector2i start, Vector2i end);
bool HasLineOfSight(Level &level, Vector2i start, Vector2i end, int maxDist);
bool HasLineOfSightToParty(SpriteData& spriteData, CharacterData& charData, Level &level, int charId);
bool IsCharacterAdjacentToPlayer(SpriteData& spriteData, CharacterData& charData, int player, int character);
std::vector<Vector2i> FindFreePositionsCircular(Level &level, int x, int y, int radius);
std::vector<int> GetTargetsInLine(SpriteData& spriteData, CharacterData& charData, Level &level, Vector2i start, Vector2 direction, int range, int exceptCharacter);

#endif //SANDBOX_PATHFINDING_H
