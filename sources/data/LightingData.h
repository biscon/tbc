//
// Created by bison on 19-06-25.
//

#ifndef SANDBOX_LIGHTINGDATA_H
#define SANDBOX_LIGHTINGDATA_H

#include <cstdint>
#include <vector>

// Light info type
struct LightSource {
    uint8_t intensity;          // max 15
    bool active;                // Is this light slot active?
    int x, y;
    float falloff;
};

struct LightingData {
    std::vector<LightSource> lights;
    std::vector<std::vector<uint8_t>> lightMap; // 0–15, like Minecraft
    int mapWidth;
    int mapHeight;
};

#endif //SANDBOX_LIGHTINGDATA_H
