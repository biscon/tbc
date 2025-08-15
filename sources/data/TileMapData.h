//
// Created by bison on 13-03-25.
//

#ifndef SANDBOX_TILEMAPDATA_H
#define SANDBOX_TILEMAPDATA_H

#include <vector>

const int BOTTOM_LAYER = 0;
const int MIDDLE_LAYER = 1;
const int LIGHT_LAYER = 2;
const int TOP_LAYER = 3;
const int NAV_LAYER = 4;
const int SHADOW_LAYER = 5;

enum class TileLayerType {
    TILE, IMAGE
};

struct TileLayerData {
    int width;
    int height;
    int* data;
};

struct ImageLayerData {
    int width;
    int height;
    Texture2D texture;
};

struct TileMapLayer {
    TileLayerType type;
    int dataIdx;
};

struct TileMap {
    int width;
    int height;
    int tileWidth;
    int tileHeight;
    int tileSet;
    std::vector<TileMapLayer> backLayers;
    std::vector<TileMapLayer> frontLayers;
    std::unordered_map<int, TileMapLayer> metaLayers;
    std::vector<TileLayerData> tileLayerData;
    std::vector<ImageLayerData> imageLayerData;
};

#endif //SANDBOX_TILEMAPDATA_H
