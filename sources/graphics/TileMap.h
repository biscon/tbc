//
// Created by bison on 21-01-25.
//

#ifndef SANDBOX_TILEMAP_H
#define SANDBOX_TILEMAP_H

#include <vector>
#include "SpriteAnimation.h"

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

void LoadTileMap(TileMap &tileMap, const char* filename, int tileSet);
void UnloadTileMap(TileMap &tileMap);
int GetTileAt(TileMap &tileMap, int layer, int x, int y);
void DrawTileLayer(SpriteSheetData& sheetData, TileMap &tileMap, int layer, int x, int y);

#endif //SANDBOX_TILEMAP_H
