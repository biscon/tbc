//
// Created by bison on 19-06-25.
//

#ifndef SANDBOX_LIGHTINGDATA_H
#define SANDBOX_LIGHTINGDATA_H

#include <cstdint>
#include <vector>
#include <string>

// Light info type
struct LightSource {
    std::string id;
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
    bool** visibilityMap = nullptr;
    Color* pixels = nullptr;
    int mapWidth;
    int mapHeight;
    Color ambient;
    Texture2D visTexture;
};

#endif //SANDBOX_LIGHTINGDATA_H
