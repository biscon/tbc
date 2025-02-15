//
// Created by bison on 02-02-25.
//

#include "DungeonMap.h"
#include "util/cute_tiled.h"
#include "raylib.h"
#include <cstring>

void LoadDungeonMap(DungeonMap &dungeonMap, const std::string &filename) {
    int fileLength = GetFileLength(filename.c_str());
    if(fileLength == 0) {
        TraceLog(LOG_ERROR, "Failed to load dungeon map %s", filename.c_str());
        return;
    }
    unsigned char* buffer = LoadFileData(filename.c_str(), &fileLength);

    cute_tiled_map_t* map = cute_tiled_load_map_from_memory(buffer, fileLength, nullptr);
    if(map == nullptr) {
        TraceLog(LOG_ERROR, "Failed to parse dungeon map %s", filename.c_str());
        return;
    }
    UnloadFileData(buffer);
    dungeonMap.width = map->width;
    dungeonMap.height = map->height;
    TraceLog(LOG_ERROR,"width: %d, height: %d s: %d", dungeonMap.width, dungeonMap.height, sizeof(dungeonMap.layers));
    size_t numElements = sizeof(dungeonMap.layers) / sizeof(dungeonMap.layers[0]);
    for(int i = 0; i < numElements; i++) {
        dungeonMap.layers[i] = new int[dungeonMap.width * dungeonMap.height];
    }

    // loop over the map's layers
    cute_tiled_layer_t* layer = map->layers;
    while (layer)
    {
        int* data = layer->data;
        int data_count = layer->data_count;
        if(strcmp(layer->name.ptr, "nav") == 0) {
            TraceLog(LOG_INFO, "Loading nav layer: %d", data_count);
            memcpy(dungeonMap.layers[NAV_LAYER], data, sizeof(int) * data_count);
        } else if(strcmp(layer->name.ptr, "walls") == 0) {
            TraceLog(LOG_INFO, "Loading walls layer");
            memcpy(dungeonMap.layers[WALLS_LAYER], data, sizeof(int) * data_count);
        } else if(strcmp(layer->name.ptr, "control") == 0) {
            TraceLog(LOG_INFO, "Loading control layer");
            memcpy(dungeonMap.layers[CONTROL_LAYER], data, sizeof(int) * data_count);
        } else if(strcmp(layer->name.ptr, "lights") == 0) {
            TraceLog(LOG_INFO, "Loading lights layer");
            memcpy(dungeonMap.layers[LIGHTS_LAYER], data, sizeof(int) * data_count);
        }
        else if(strcmp(layer->name.ptr, "objects") == 0) {
            TraceLog(LOG_INFO, "Loading objects layer");
            memcpy(dungeonMap.layers[OBJECTS_LAYER], data, sizeof(int) * data_count);
        }
        layer = layer->next;
    }

    cute_tiled_free_map(map);
}

void UnloadDungeonMap(DungeonMap &map) {
    size_t numElements = sizeof(map.layers) / sizeof(map.layers[0]);
    for(int i = 0; i < numElements; i++) {
        delete[] map.layers[i];
    }
}

int GetDungeonTile(DungeonMap &map, int layer, int x, int y) {
    if(x < 0 || x >= map.width || y < 0 || y >= map.height) {
        return -1;
    }
    size_t numElements = sizeof(map.layers) / sizeof(map.layers[0]);
    if(layer < 0 || layer >= numElements) {
        return -1;
    }
    return map.layers[layer][y * map.width + x];
}
