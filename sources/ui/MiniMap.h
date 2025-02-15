//
// Created by bison on 08-02-25.
//

#ifndef SANDBOX_MINIMAP_H
#define SANDBOX_MINIMAP_H

#include "raylib.h"
#include "dungeon/DungeonMap.h"
#include "graphics/DungeonRenderer.h"

struct MiniMap {
    Vector2i position;
    int width;
    int height;
    DungeonMap* map;
    unsigned char *visitedCells;
};

void CreateMiniMap(MiniMap& miniMap, DungeonMap& map, int width, int height, const Vector2i& position);
void DestroyMiniMap(MiniMap& miniMap);
void DrawMiniMap(MiniMap& miniMap, DungeonState& dungeonState);

#endif //SANDBOX_MINIMAP_H
