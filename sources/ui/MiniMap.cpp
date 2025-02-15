//
// Created by bison on 08-02-25.
//

#include <cmath>
#include <cstring>
#include <queue>
#include "MiniMap.h"

void CreateMiniMap(MiniMap &miniMap, DungeonMap &map, int width, int height, const Vector2i &position) {
    miniMap.position = position;
    miniMap.width = width;
    miniMap.height = height;
    miniMap.map = &map;
    miniMap.visitedCells = new unsigned char[map.width * map.height];
    memset(miniMap.visitedCells, 0, map.width * map.height);
}

static void FloodFillReveal(MiniMap &miniMap, int startX, int startY) {
    if (miniMap.visitedCells[startY * miniMap.map->width + startX] == 1) return; // Already visited
    std::queue<Vector2i> toVisit;
    toVisit.push({ startX, startY });

    while (!toVisit.empty()) {
        Vector2i current = toVisit.front();
        toVisit.pop();

        int x = current.x;
        int y = current.y;

        if (x < 0 || x >= miniMap.map->width || y < 0 || y >= miniMap.map->height)
            continue; // Out of bounds

        if (miniMap.visitedCells[y * miniMap.map->width + x] == 1)
            continue; // Already marked

        if (GetDungeonTile(*miniMap.map, NAV_LAYER, x, y) == TILE_BLOCKED
            || GetDungeonTile(*miniMap.map, OBJECTS_LAYER, x, y) == TILE_DOOR_X
            || GetDungeonTile(*miniMap.map, OBJECTS_LAYER, x, y) == TILE_DOOR_Z)
            continue; // Stop at walls and doors

        // Mark as visited
        miniMap.visitedCells[y * miniMap.map->width + x] = 1;

        // Add neighbors to queue
        toVisit.push({ x + 1, y });
        toVisit.push({ x - 1, y });
        toVisit.push({ x, y + 1 });
        toVisit.push({ x, y - 1 });
    }
}


void DrawMiniMap(MiniMap &miniMap, DungeonState &dungeonState) {
    int cellWidth = (int)ceilf((float)miniMap.width / (float)miniMap.map->width);
    int cellHeight = (int)ceilf((float)miniMap.height / (float)miniMap.map->height);

    // Draw minimap border
    DrawRectangle(miniMap.position.x, miniMap.position.y, miniMap.width, miniMap.height, DARKGRAY);

    // Update visited cells based on player's current position
    int playerGridX = static_cast<int>(dungeonState.player.position.x / CUBE_SIZE);
    int playerGridY = static_cast<int>(dungeonState.player.position.y / CUBE_SIZE);
    FloodFillReveal(miniMap, playerGridX, playerGridY);

    // Mark current player cell as visited
    if (playerGridX >= 0 && playerGridX < miniMap.map->width && playerGridY >= 0 && playerGridY < miniMap.map->height) {
        miniMap.visitedCells[playerGridY * miniMap.map->width + playerGridX] = 1;
    }

    for (int y = 0; y < miniMap.map->height; y++) {
        for (int x = 0; x < miniMap.map->width; x++) {
            // Skip drawing if cell is not visited
            if (miniMap.visitedCells[y * miniMap.map->width + x] == 0) {
                continue;
            }

            int tile = GetDungeonTile(*miniMap.map, NAV_LAYER, x, y);
            Color tileColor = (tile == TILE_BLOCKED) ? DARKGRAY : GRAY;
            DrawRectangle(miniMap.position.x + x * cellWidth, miniMap.position.y + y * cellHeight, cellWidth, cellHeight, tileColor);

            // Draw doors
            if (GetDungeonTile(*miniMap.map, OBJECTS_LAYER, x, y) == TILE_DOOR_Z) {
                DrawLine(miniMap.position.x + x * cellWidth, miniMap.position.y + y * cellHeight + (cellHeight / 2),
                         miniMap.position.x + x * cellWidth + cellWidth, miniMap.position.y + y * cellHeight + (cellHeight / 2), LIGHTGRAY);
            }

            if (GetDungeonTile(*miniMap.map, OBJECTS_LAYER, x, y) == TILE_DOOR_X) {
                DrawLine(miniMap.position.x + x * cellWidth + (cellWidth / 2), miniMap.position.y + y * cellHeight,
                         miniMap.position.x + x * cellWidth + (cellWidth / 2), miniMap.position.y + y * cellHeight + cellHeight, LIGHTGRAY);
            }
        }
    }

    // Draw player marker
    Vector2 charPos = {
            (float) miniMap.position.x + playerGridX * cellWidth + (cellWidth / 2),
            (float) miniMap.position.y + playerGridY * cellHeight + (cellHeight / 2)
    };

    float radius = fminf(cellWidth, cellHeight) * 0.3f;
    DrawCircleV(charPos, radius, BLUE);

    // Draw direction indicator
    float lineLength = fminf(cellWidth, cellHeight) * 0.5f;
    Vector2 directions[4] = {
            {0, -lineLength}, // North
            {lineLength, 0},  // East
            {0, lineLength},  // South
            {-lineLength, 0}  // West
    };

    Vector2 facingDir = {charPos.x + directions[dungeonState.playerFacing].x, charPos.y + directions[dungeonState.playerFacing].y};
    DrawLineEx(charPos, facingDir, 2.0f, WHITE);

}


void DestroyMiniMap(MiniMap &miniMap) {
    delete[] miniMap.visitedCells;
    miniMap.visitedCells = nullptr;
    miniMap.map = nullptr;
}

