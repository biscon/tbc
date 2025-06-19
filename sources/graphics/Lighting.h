//
// Created by bison on 16-06-25.
//

#ifndef SANDBOX_LIGHTING_H
#define SANDBOX_LIGHTING_H

#include <vector>
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "TileMap.h"
#include "util/MathUtil.h"
#include "data/LightingData.h"

void MoveLight(LightSource& light, int x, int y);
void AddLight(LightingData& data, std::string id, int x, int y, uint8_t intensity, float falloff, Color color, bool active);
void InitLightingData(LightingData& data, const TileMap& map);
void PropagateLight(LightingData& data, const TileMap& map);
Color GetVertexLight(const LightingData& data, int vx, int vy);
Color GetVertexLight(const LightingData& data, const TileMap& map, int vx, int vy);
Color GetVertexLightWeighted(const LightingData& data, int vx, int vy);

#endif //SANDBOX_LIGHTING_H
