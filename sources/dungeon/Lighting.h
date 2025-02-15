//
// Created by bison on 30-03-24.
//

#ifndef CRAWLER_LIGHTING_H
#define CRAWLER_LIGHTING_H

#include "defs.h"
#include "raylib.h"
#include <vector>

// Queue structure for keeping track of cells to propagate light
struct Cell {
    i32 x;
    i32 y;
};

struct Lighting {
    i32 width;
    i32 height;
    std::vector<u8> lightMap;
    std::vector<Cell> lightQueue;
    std::vector<float> lightLevels;
};

void InitLighting(Lighting& lighting, i32 w, i32 h);
void BuildLightMap(Lighting& l, const i32 map[], const i32 blockedMap[]);
float GetLightLevelAt(Lighting& l, i32 x, i32 y, i32 &count);
void GetLightColorAt(Lighting& l, i32 x, i32 y, Color& color);

// Top-Bottom
void SetLightColorTopLeft_TB(Lighting& r, i32 x, i32 y, Color& color);
void SetLightColorTopRight_TB(Lighting& l, i32 x, i32 y, Color& color);
void SetLightColorBottomLeft_TB(Lighting& l, i32 x, i32 y, Color& color);
void SetLightColorBottomRight_TB(Lighting& l, i32 x, i32 y, Color& color);

// Left-Right
void SetLightColorLeft_LR(Lighting& l, i32 x, i32 y, Color& color);
void SetLightColorRight_LR(Lighting& l, i32 x, i32 y, Color& color);

// Front-Back
void SetLightColorLeft_FB(Lighting& l, i32 x, i32 y, Color& color);
void SetLightColorRight_FB(Lighting& l, i32 x, i32 y, Color& color);
Color ApplyTorchColor(float intensity);

#endif //CRAWLER_LIGHTING_H
