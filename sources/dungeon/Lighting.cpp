//
// Created by bison on 30-03-24.
//

#include <cstdio>
#include <algorithm>
#include "Lighting.h"
#include "raymath.h"
#include "../util/stb_perlin.h"

static void printLightMap(Lighting& l) {
    // Print the resulting map
    for (int y = 0; y < l.height; y++) {
        for (int x = 0; x < l.width; x++) {
            printf("%2d ", l.lightMap[y * l.width + x]);
        }
        printf("\n");
    }
    // print light level values
    for(i32 i = 0; i < l.lightLevels.size(); i++) {
        printf("%d: %f\n", i, l.lightLevels[i]);
    }
}

static void precomputeLightLevels(Lighting& l, int divisions, float power = 2.2f) {
    l.lightLevels.clear();
    if (divisions < 1) return;

    for (int i = 0; i < divisions; ++i) {
        float t = static_cast<float>(i) / (divisions - 1); // Normalize to 0-1
        l.lightLevels.push_back(powf(t, power)); // Apply power curve
    }
}

// Function to propagate light in the lightMap
static void propagateLight(Lighting& l, i32 startX, i32 startY, const i32 map[], const i32 blockedMap[]) {
    // Directions for propagation (n, s, w, e, nw, ne, sw, se)
    i32 dx[] = {-1, 1,  0, 0, -1,  1, -1, 1};
    i32 dy[] = { 0, 0, -1, 1, -1, -1,  1, 1};

    i32 front = 0, rear = 0;

    // Add initial seed cell
    l.lightQueue[rear].x = startX;
    l.lightQueue[rear].y = startY;
    rear++;

    while (front < rear) {
        // Dequeue cell
        i32 x = l.lightQueue[front].x;
        i32 y = l.lightQueue[front].y;
        front++;

        // Calculate index for current cell
        i32 index = y * l.width + x;
        u8 currentBrightness = l.lightMap[index];

        // Propagate light to adjacent cells
        for (i32 i = 0; i < 4; i++) {
            i32 nx = x + dx[i];
            i32 ny = y + dy[i];
            i32 newIndex = ny * l.width + nx;

            // Check if the adjacent cell is within bounds
            if (nx >= 0 && nx < l.width && ny >= 0 && ny < l.height) {
                // Check if the cell is blocked
                if (blockedMap[newIndex] == 2) {
                    continue;
                }
                // Update brightness and add cell to queue
                if (l.lightMap[newIndex] < currentBrightness - 1) {
                    l.lightMap[newIndex] = currentBrightness - 1;
                    l.lightQueue[rear].x = nx;
                    l.lightQueue[rear].y = ny;
                    rear++;
                }
            }
        }
    }
}

float GetLightLevelAt(Lighting& l, i32 x, i32 y, i32 &count) {
    if (x < 0 || x >= l.width || y < 0 || y >= l.height) {
        return l.lightLevels[0];
    }
    auto lightIndex = l.lightMap[l.width * y + x];
    float level = l.lightLevels[lightIndex];
    if(level <= 0.025f) {
        //level = 0.025f;
    }
    count++;
    return level;
}

Color BlendColors(Color c1, Color c2, float t) {
    t = Clamp(t, 0.0f, 1.0f); // Ensure t is in range [0,1]
    return (Color){
            (unsigned char)((1 - t) * c1.r + t * c2.r),
            (unsigned char)((1 - t) * c1.g + t * c2.g),
            (unsigned char)((1 - t) * c1.b + t * c2.b),
            (unsigned char)((1 - t) * c1.a + t * c2.a)
    };
}

Color ApplyTorchColor(float intensity) {
    unsigned char r = (unsigned char)(255 * intensity);
    unsigned char g = (unsigned char)(200 * intensity);
    unsigned char b = (unsigned char)(120 * intensity);
    return (Color){r, g, b, 255}; // RGBA
}

float GetTorchFlicker(float baseIntensity) {
    if(baseIntensity < 0.75f)
        return baseIntensity;
    float flicker = (GetRandomValue(-10, 10) / 500.0f); // Small random change
    return Clamp(baseIntensity + flicker, 0.0f, 1.0f);
}

static inline void setLightColor(float level, Color& color) {
    level = Clamp(level, 0.0f, 1.0f);
    //Vector3 tint = {1.0f, 0.95f, 0.90f};
    //Vector3 tint = {1.0f, 1.0f, 1.0f};

    float intensity = level;
    color = ApplyTorchColor(intensity);
    /*
    Vector3 result = Vector3Multiply(tint, Vector3{level, level, level});
    color.r = (u8)(result.x * 255);
    color.g = (u8)(result.y * 255);
    color.b = (u8)(result.z * 255);
    color.a = 255;
        */
}

void GetLightColorAt(Lighting& l, i32 x, i32 y, Color& color) {
    i32 count = 0;
    float level = GetLightLevelAt(l, x, y, count);
    setLightColor(level, color);
}

// Top-Bottom
void SetLightColorTopLeft_TB(Lighting& r, i32 x, i32 y, Color& color) {
    i32 count = 0;
    float level = GetLightLevelAt(r, x, y, count) + GetLightLevelAt(r, x-1, y, count) + GetLightLevelAt(r, x-1, y-1, count) + GetLightLevelAt(r, x, y-1, count);
    level /= (float) count;
    setLightColor(level, color);
}

void SetLightColorTopRight_TB(Lighting& l, i32 x, i32 y, Color& color) {
    i32 count = 0;
    float level = GetLightLevelAt(l, x, y, count) + GetLightLevelAt(l, x+1, y, count) + GetLightLevelAt(l, x+1, y-1, count) + GetLightLevelAt(l, x, y-1, count);
    level /= (float) count;
    setLightColor(level, color);
}

void SetLightColorBottomLeft_TB(Lighting& l, i32 x, i32 y, Color& color) {
    i32 count = 0;
    float level = GetLightLevelAt(l, x, y, count) + GetLightLevelAt(l, x-1, y, count) + GetLightLevelAt(l, x-1, y+1, count) + GetLightLevelAt(l, x, y+1, count);
    level /= (float) count;
    setLightColor(level, color);
}

void SetLightColorBottomRight_TB(Lighting& l, i32 x, i32 y, Color& color) {
    i32 count = 0;
    float level = GetLightLevelAt(l, x, y, count) + GetLightLevelAt(l, x+1, y, count) + GetLightLevelAt(l, x+1, y+1, count) + GetLightLevelAt(l, x, y+1, count);
    level /= (float) count;
    setLightColor(level, color);
}

// Left-Right
void SetLightColorLeft_LR(Lighting& l, i32 x, i32 y, Color& color) {
    i32 count = 0;
    float level = GetLightLevelAt(l, x, y, count) + GetLightLevelAt(l, x, y+1, count);
    level /= (float) count;
    setLightColor(level, color);
}

void SetLightColorRight_LR(Lighting& l, i32 x, i32 y, Color& color) {
    i32 count = 0;
    float level = GetLightLevelAt(l, x, y, count) + GetLightLevelAt(l, x, y-1, count);
    level /= (float) count;
    setLightColor(level, color);
}

// Front-Back
void SetLightColorLeft_FB(Lighting& l, i32 x, i32 y, Color& color) {
    i32 count = 0;
    float level = GetLightLevelAt(l, x, y, count) + GetLightLevelAt(l, x-1, y, count);
    level /= (float) count;
    setLightColor(level, color);
}

void SetLightColorRight_FB(Lighting& l, i32 x, i32 y, Color& color) {
    i32 count = 0;
    float level = GetLightLevelAt(l, x, y, count) + GetLightLevelAt(l, x+1, y, count);
    level /= (float) count;
    setLightColor(level, color);
}

void BuildLightMap(Lighting& l, const i32 map[], const i32 blockedMap[]) {
    l.lightQueue.clear();
    l.lightQueue.reserve(l.width * l.height);
    l.lightMap.clear();
    l.lightMap.resize(l.width * l.height);
    // Iterate over the map
    for (int y = 0; y < l.height; y++) {
        for (int x = 0; x < l.width; x++) {
            // Get the current cell
            int currentCell = map[y * l.width + x];
            if (currentCell == 7) {
                l.lightMap[y * l.width + x] = 7;
                propagateLight(l, x, y, map, blockedMap);
            }
            if (currentCell == 8) {
                l.lightMap[y * l.width + x] = 5;
                propagateLight(l, x, y, map, blockedMap);
            }
            if (currentCell == 9) {
                l.lightMap[y * l.width + x] = 4;
                propagateLight(l, x, y, map, blockedMap);
            }
        }
    }
    printLightMap(l);
}

void InitLighting(Lighting &lighting, int32_t w, int32_t h) {
    precomputeLightLevels(lighting, 8, 2.5f);
    lighting.width = w;
    lighting.height = h;
}
