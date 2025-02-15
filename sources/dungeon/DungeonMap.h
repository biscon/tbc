//
// Created by bison on 02-02-25.
//

#ifndef SANDBOX_DUNGEONMAP_H
#define SANDBOX_DUNGEONMAP_H

#include <string>

const int TILE_WALL1 = 3;
const int TILE_WALL2 = 4;
const int TILE_WALL3 = 5;
const int TILE_DOOR_Z = 6;
const int TILE_SHOP1 = 14;
const int TILE_FLOOR = 2;
const int TILE_CEILING = 300;
const int TILE_NONE = 1;
const int TILE_BLOCKED = 2;
const int TILE_WALL_TORCH_NORTH = 15;
const int TILE_WALL_TORCH_SOUTH = 16;
const int TILE_WALL_TORCH_EAST = 17;
const int TILE_WALL_TORCH_WEST = 18;
const int TILE_PILLAR = 19;
const int TILE_DOOR_X = 20;
const int TILE_RED_CARPET = 10;


const int NAV_LAYER = 0;
const int WALLS_LAYER = 1;
const int CONTROL_LAYER = 2;
const int LIGHTS_LAYER = 3;
const int OBJECTS_LAYER = 4;

struct DungeonMap {
    int width;
    int height;
    int* layers[5];
};

void LoadDungeonMap(DungeonMap& map, const std::string& filename);
void UnloadDungeonMap(DungeonMap& map);
int GetDungeonTile(DungeonMap &map, int layer, int x, int y);


#endif //SANDBOX_DUNGEONMAP_H
