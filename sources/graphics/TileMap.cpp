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

static void LoadLayers(TileMap &tileMap, std::vector<TileMapLayer>& stack, const json& jLayers) {
    for (auto &jLayer : jLayers) {
        std::string type = jLayer["type"].get<std::string>();
        std::string name = jLayer["name"].get<std::string>();
        if(type == "tilelayer") {
            TraceLog(LOG_INFO, "Reading tile layer %s (type = %s)", name.c_str(), type.c_str());
            TileLayerData layerData{};
            TileMapLayer layer{};
            layer.type = TileLayerType::TILE;
            layerData.width = jLayer["width"].get<int>();
            layerData.height = jLayer["height"].get<int>();
            layerData.data = (int*) malloc(sizeof(int) * (layerData.width * layerData.height));
            int index = 0;
            for (int value : jLayer["data"]) {
                layerData.data[index] = value;
                index++;
            }
            tileMap.tileLayerData.push_back(layerData);
            layer.dataIdx = static_cast<int>(tileMap.tileLayerData.size());
            stack.push_back(layer);
        }
        if(type == "imagelayer") {
            TraceLog(LOG_INFO, "Reading image layer %s (type = %s)", name.c_str(), type.c_str());
            ImageLayerData layerData{};
            TileMapLayer layer{};
            layer.type = TileLayerType::IMAGE;
            layerData.width = jLayer["imagewidth"].get<int>();
            layerData.height = jLayer["imageheight"].get<int>();
            layerData.x = jLayer["x"].get<int>();
            layerData.y = jLayer["y"].get<int>();
            std::string imagefile = jLayer["image"].get<std::string>();
            layerData.texture = LoadTexture(imagefile.c_str());

            layer.dataIdx = static_cast<int>(tileMap.imageLayerData.size());
            tileMap.imageLayerData.push_back(layerData);
            stack.push_back(layer);
        }
    }
}

static void LoadMetaLayers(TileMap& tileMap, const json& jLayers) {
    for (auto &jLayer : jLayers) {
        std::string type = jLayer["type"].get<std::string>();
        std::string name = jLayer["name"].get<std::string>();
        if(type == "tilelayer") {
            TraceLog(LOG_INFO, "Reading meta layer %s (type = %s)", name.c_str(), type.c_str());
            TileLayerData layerData{};
            TileMapLayer layer{};
            layer.type = TileLayerType::TILE;
            layerData.width = jLayer["width"].get<int>();
            layerData.height = jLayer["height"].get<int>();
            layerData.data = (int*) malloc(sizeof(int) * (layerData.width * layerData.height));
            int index = 0;
            for (int value : jLayer["data"]) {
                layerData.data[index] = value;
                index++;
            }
            layer.dataIdx = static_cast<int>(tileMap.tileLayerData.size());
            tileMap.tileLayerData.push_back(layerData);

            if(name == "nav") tileMap.metaLayers[NAV_LAYER] = layer;
            if(name == "shadow") tileMap.metaLayers[SHADOW_LAYER] = layer;
        }
    }
}

void LoadTileMap(TileMap &tileMap, const std::string& filename, int tileSet) {
    UnloadTileMap(tileMap);
    tileMap.backLayers.clear();
    tileMap.frontLayers.clear();
    tileMap.metaLayers.clear();
    tileMap.tileLayerData.clear();
    tileMap.imageLayerData.clear();
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
        if(type == "group" && name == "back_layers") {
            LoadLayers(tileMap, tileMap.backLayers, jLayer["layers"]);
        }
        if(type == "group" && name == "front_layers") {
            LoadLayers(tileMap, tileMap.frontLayers, jLayer["layers"]);
        }
        if(type == "group" && name == "meta_layers") {
            LoadMetaLayers(tileMap, jLayer["layers"]);
        }
        /*
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
        */
    }

    TraceLog(LOG_INFO, "Loaded tile map %s (%dx%d)", filename.c_str(), tileMap.width, tileMap.height);
}

void UnloadTileMap(TileMap &tileMap) {
    // free tile layers
    for(auto &layerData : tileMap.tileLayerData) {
        free(layerData.data);
        layerData.data = nullptr;
    }
}

int GetTileAt(TileMap &tileMap, int layer, int x, int y) {
    TileLayerData& layerData = tileMap.tileLayerData[tileMap.metaLayers[layer].dataIdx];
    if(x < 0 || x >= layerData.width || y < 0 || y >= layerData.height) {
        return -1;
    }
    return layerData.data[y * layerData.width + x];
}

static void DrawTileLayer(LightingData& lightData, SpriteSheetData& sheetData, TileMap& tileMap, TileMapLayer& tileMapLayer, int x, int y) {
    TileLayerData& layerData = tileMap.tileLayerData[tileMapLayer.dataIdx];
    for(int ty = 0; ty < tileMap.height; ty++) {
        for(int tx = 0; tx < tileMap.width; tx++) {
            int tileIndex = layerData.data[ty * tileMap.width + tx];
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

void DrawLayers(LightingData& lightData, SpriteSheetData& sheetData, TileMap &tileMap, std::vector<TileMapLayer>& stack, int x, int y) {
    if(stack.empty())
        return;
    for(auto& layer : stack) {
        if(layer.type == TileLayerType::TILE) {
            DrawTileLayer(lightData, sheetData, tileMap, layer, x, y);
        }
        if(layer.type == TileLayerType::IMAGE) {

        }
    }
}

void SetTiles(TileMap &tileMap, const std::vector<Vector2i> &positions, int layer, int value) {
    TileLayerData& layerData = tileMap.tileLayerData[tileMap.metaLayers[layer].dataIdx];
    for(const auto& pos : positions) {
        if(pos.x < 0 || pos.x >= layerData.width || pos.y < 0 || pos.y >= layerData.height) {
            TraceLog(LOG_ERROR, "Tilemap %i,%i is out of bounds", pos.x, pos.y);
            return;
        }

        layerData.data[pos.y * tileMap.width + pos.x] = value;
    }
}
