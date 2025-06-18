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

// Custom Blend Modes
#define RLGL_SRC_ALPHA 0x0302
#define RLGL_MIN 0x8007
#define RLGL_MAX 0x8008

#define MAX_BOXES     128
#define MAX_SHADOWS   MAX_BOXES*3         // MAX_BOXES *3. Each box can cast up to two shadow volumes for the edges it is away from, and one for the box itself


// Shadow geometry type
struct ShadowGeometry {
    Vector2 vertices[4];
};

// Light info type
struct LightInfo {
    float intensity;
    Color color;
    bool active;                // Is this light slot active?
    Vector2 position;           // Light position
    float radius;          // The distance the light touches
    Rectangle bounds;           // A cached rectangle of the light bounds to help with culling
    ShadowGeometry shadows[MAX_SHADOWS];
    int shadowCount;
    Texture2D texture;
};

struct LightingData {
    std::vector<LightInfo> lights;
    std::vector<Rectangle> boxes;
    RenderTexture lightMask;
    RenderTexture shadowMask;
    float ambient;
};

void MoveLight(LightInfo& light, float x, float y);
void InitLightingData(LightingData& data);
void UpdateLighting(LightingData& data, Camera2D camera);
void RenderLighting(LightingData& data);
void RenderShadows(LightingData& data);
void BuildShadowBoxes(LightingData& data, TileMap& tileMap);

#endif //SANDBOX_LIGHTING_H
