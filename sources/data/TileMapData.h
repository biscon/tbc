//
// Created by bison on 13-03-25.
//

#ifndef SANDBOX_TILEMAPDATA_H
#define SANDBOX_TILEMAPDATA_H

#include <vector>

const int BOTTOM_LAYER = 0;
const int MIDDLE_LAYER = 1;
const int TOP_LAYER = 2;
const int NAV_LAYER = 3;

struct TileMapLayer {
    int width;
    int height;
    int* data;
};

struct TileMap {
    int width;
    int height;
    int tileWidth;
    int tileHeight;
    int tileSet;
    std::vector<TileMapLayer> layers;
};

#endif //SANDBOX_TILEMAPDATA_H
