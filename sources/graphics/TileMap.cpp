//
// Created by bison on 21-01-25.
//

#include <fstream>
#include "TileMap.h"

//#define CUTE_TILED_IMPLEMENTATION
//#include "util/cute_tiled.h"
#include "rlgl.h"
#include "Lighting.h"
#include "Rendering.h"

using json = nlohmann::json;

void LoadTileMap(TileMap &tileMap, const std::string& filename, int tileSet) {
    tileMap.layers.clear();
    tileMap.tileSet = tileSet;

    std::ifstream file(filename);
    if (!file) {
        TraceLog(LOG_ERROR, "Failed to open map file: %s", filename.c_str());
        std::abort();
        return;
    }

    json j;
    file >> j;
    tileMap.width = j["width"].get<int>();
    tileMap.height = j["height"].get<int>();
    tileMap.tileWidth = j["tilewidth"].get<int>();
    tileMap.tileHeight = j["tileheight"].get<int>();

    for (auto &jLayer : j["layers"]) {
        std::string type = jLayer["type"].get<std::string>();
        std::string name = jLayer["name"].get<std::string>();
        TraceLog(LOG_INFO, "Reading map layer %s (type = %s)", name.c_str(), type.c_str());
        if(type == "tilelayer") {
            TileMapLayer mapLayer{};
            mapLayer.width = tileMap.width;
            mapLayer.height = tileMap.height;
            mapLayer.data = (int*) malloc(sizeof(int) * (tileMap.width * tileMap.height));
            int index = 0;
            for (int value : jLayer["data"]) {
                mapLayer.data[index] = value;
                index++;
            }
            if(name == "bottom") tileMap.layers[BOTTOM_LAYER] = mapLayer;
            if(name == "middle") tileMap.layers[MIDDLE_LAYER] = mapLayer;
            if(name == "light") tileMap.layers[LIGHT_LAYER] = mapLayer;
            if(name == "top") tileMap.layers[TOP_LAYER] = mapLayer;
            if(name == "nav") tileMap.layers[NAV_LAYER] = mapLayer;
            if(name == "shadow") tileMap.layers[SHADOW_LAYER] = mapLayer;
        }
    }

    TraceLog(LOG_INFO, "Loaded tile map %s (%dx%d), parsed %d layers", filename.c_str(), tileMap.width, tileMap.height, tileMap.layers.size());
}

void UnloadTileMap(TileMap &tileMap) {
    // free tile layers
    for(auto &layer : tileMap.layers) {
        free(layer.second.data);
        layer.second.data = nullptr;
    }
}

int GetTileAt(TileMap &tileMap, int layer, int x, int y) {
    if(layer < 0 || layer >= tileMap.layers.size()) {
        return -1;
    }
    if(x < 0 || x >= tileMap.width || y < 0 || y >= tileMap.height) {
        return -1;
    }
    return tileMap.layers[layer].data[y * tileMap.width + x];
}


void DrawTileLayer(LightingData& lightData, SpriteSheetData& sheetData, TileMap &tileMap, int layer, int x, int y) {
    if(layer < 0 || layer >= tileMap.layers.size()) {
        return;
    }
    for(int ty = 0; ty < tileMap.height; ty++) {
        for(int tx = 0; tx < tileMap.width; tx++) {
            int tileIndex = GetTileAt(tileMap, layer, tx, ty);
            if(tileIndex > 0) {
                auto& texture = sheetData.texture[tileMap.tileSet];
                auto& texRect = sheetData.frameRects[tileMap.tileSet][tileIndex-1];
                auto dstRect = Rectangle{(float)(x + tx * tileMap.tileWidth), (float)(y + ty * tileMap.tileHeight), 16, 16};


                /*
                Color v1 = GetVertexLight(lightData, tileMap, tx, ty);     // top-left corner
                Color v2 = GetVertexLight(lightData, tileMap, tx+1, ty);   // top-right
                Color v3 = GetVertexLight(lightData, tileMap, tx+1, ty+1); // bottom-right
                Color v4 = GetVertexLight(lightData, tileMap, tx, ty+1);   // bottom-left
                */

                Color v1 = GetVertexLightWeighted(lightData, tx, ty);     // top-left corner
                Color v2 = GetVertexLightWeighted(lightData, tx+1, ty);   // top-right
                Color v3 = GetVertexLightWeighted(lightData, tx+1, ty+1); // bottom-right
                Color v4 = GetVertexLightWeighted(lightData, tx, ty+1);   // bottom-left

                DrawTexturedQuadWithVertexColors(texture, texRect, dstRect, v1, v2, v3, v4);

                //DrawTexturedQuadWithVertexColors(texture, texRect, dstRect, color, color, color, color);
                //DrawTextureRec(sheetData.texture[tileMap.tileSet], sheetData.frameRects[tileMap.tileSet][tileIndex-1], Vector2{(float)(x + tx * tileMap.tileWidth), (float)(y + ty * tileMap.tileHeight)}, tint);
            }
        }
    }
}

void SetTiles(TileMap &tileMap, const std::vector<Vector2i> &positions, int layer, int value) {
    for(const auto& pos : positions) {
        if(layer < 0 || layer >= tileMap.layers.size()) {
            TraceLog(LOG_ERROR, "Tilemap layer %i is out of bounds", layer);
            return;
        }
        if(pos.x < 0 || pos.x >= tileMap.width || pos.y < 0 || pos.y >= tileMap.height) {
            TraceLog(LOG_ERROR, "Tilemap %i,%i is out of bounds", pos.x, pos.y);
            return;
        }
        tileMap.layers[layer].data[pos.y * tileMap.width + pos.x] = value;
    }
}
