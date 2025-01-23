//
// Created by bison on 21-01-25.
//

#include "TileMap.h"

#define CUTE_TILED_IMPLEMENTATION
#include "util/cute_tiled.h"

void LoadTileMap(TileMap &tileMap, const char *filename, SpriteSheet* tileSet) {
    tileMap.tileSet = tileSet;
    int fileLength = GetFileLength(filename);
    if(fileLength == 0) {
        TraceLog(LOG_ERROR, "Failed to load tile map %s", filename);
        return;
    }
    unsigned char* buffer = LoadFileData(filename, &fileLength);

    cute_tiled_map_t* map = cute_tiled_load_map_from_memory(buffer, fileLength, nullptr);
    if(map == nullptr) {
        TraceLog(LOG_ERROR, "Failed to parse tile map %s", filename);
        return;
    }
    UnloadFileData(buffer);

    tileMap.width = map->width;
    tileMap.height = map->height;
    tileMap.tileWidth = map->tilewidth;
    tileMap.tileHeight = map->tileheight;

    // loop over the map's layers
    cute_tiled_layer_t* layer = map->layers;
    while (layer)
    {
        int* data = layer->data;
        int data_count = layer->data_count;

        TileMapLayer mapLayer{};
        mapLayer.width = layer->height;
        mapLayer.height = layer->height;
        mapLayer.data = (int*) malloc(sizeof(int) * data_count);
        memcpy(mapLayer.data, data, sizeof(int) * data_count);
        tileMap.layers.push_back(mapLayer);

        layer = layer->next;
    }

    TraceLog(LOG_INFO, "Loaded tile map %s (%dx%d), parsed %d layers", filename, tileMap.width, tileMap.height, tileMap.layers.size());
    cute_tiled_free_map(map);
}

void UnloadTileMap(TileMap &tileMap) {
    // free tile layers
    for(auto &layer : tileMap.layers) {
        free(layer.data);
        layer.data = nullptr;
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

void DrawTileLayer(TileMap &tileMap, int layer, int x, int y) {
    if(layer < 0 || layer >= tileMap.layers.size()) {
        return;
    }
    for(int ty = 0; ty < tileMap.height; ty++) {
        for(int tx = 0; tx < tileMap.width; tx++) {
            int tileIndex = GetTileAt(tileMap, layer, tx, ty);
            if(tileIndex > 0) {
                DrawTextureRec(tileMap.tileSet->texture, tileMap.tileSet->frameRects[tileIndex-1], Vector2{(float)(x + tx * tileMap.tileWidth), (float)(y + ty * tileMap.tileHeight)}, WHITE);
            }
        }
    }

}
