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
    Color color;
};

struct LightingData {
    std::vector<LightSource> lights;
    float** lightMapR = nullptr;
    float** lightMapG = nullptr;
    float** lightMapB = nullptr;
    int mapWidth;
    int mapHeight;
    Color ambient;
};

#endif //SANDBOX_LIGHTINGDATA_H
