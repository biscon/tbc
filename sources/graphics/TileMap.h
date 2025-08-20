//
// Created by bison on 21-01-25.
//

#ifndef SANDBOX_TILEMAP_H
#define SANDBOX_TILEMAP_H

#include <cstdint>
#include "data/SpriteData.h"
#include "data/TileMapData.h"
#include "data/LightingData.h"
#include "util/MathUtil.h"
#include "data/GameData.h"

void LoadTileMap(TileMap &tileMap, const std::string& filename, int tileSet);
void UnloadTileMap(TileMap &tileMap);
int GetTileAt(TileMap &tileMap, int layer, int x, int y);
//void DrawTileLayer(LightingData& lightData, SpriteSheetData& sheetData, TileMap &tileMap, int layer, int x, int y);
void SetTiles(TileMap &tileMap, const std::vector<Vector2i>& positions, int layer, int value);
void DrawLayers(GameData& data, LightingData& lightData, SpriteSheetData& sheetData, TileMap &tileMap, std::vector<TileMapLayer>& stack, int x, int y);

#endif //SANDBOX_TILEMAP_H
