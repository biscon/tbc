//
// Created by bison on 21-01-25.
//

#ifndef SANDBOX_TILEMAP_H
#define SANDBOX_TILEMAP_H

#include <cstdint>
#include "data/SpriteData.h"
#include "data/TileMapData.h"
#include "data/LightingData.h"

void LoadTileMap(TileMap &tileMap, const char* filename, int tileSet);
void UnloadTileMap(TileMap &tileMap);
int GetTileAt(const TileMap &tileMap, int layer, int x, int y);
void DrawTileLayer(LightingData& lightData, SpriteSheetData& sheetData, TileMap &tileMap, int layer, int x, int y);

#endif //SANDBOX_TILEMAP_H
