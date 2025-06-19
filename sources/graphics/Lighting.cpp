//
// Created by bison on 16-06-25.
//

#include "Lighting.h"
#include "data/GameData.h"
#include "util/MathUtil.h"
#include "ai/PathFinding.h"
#include <queue>

void MoveLight(LightSource& light, int x, int y) {
    light.x = x;
    light.y = y;
}

void AddLight(LightingData& data, int x, int y, uint8_t intensity, float falloff) {
    LightSource light{};
    light.intensity = intensity;
    light.active = true;
    light.falloff = falloff;

    MoveLight(light, x, y);
    data.lights.push_back(light);
}


static void ResizeTileLighting(LightingData &data, int newWidth, int newHeight) {
    data.mapWidth = newWidth;
    data.mapHeight = newHeight;

    data.lightMap.resize(data.mapWidth);

    for (int x = 0; x < data.mapWidth; x++) {
        data.lightMap[x].resize(data.mapHeight, 0);
    }
}

void PropagateLight(LightingData& data, const TileMap& map) {
    struct LightNode {
        int x;
        int y;
        float level;
        float falloff;
    };

    std::queue<LightNode> queue;

    // Reset lightmap
    for (int y = 0; y < data.mapHeight; y++) {
        for (int x = 0; x < data.mapWidth; x++) {
            data.lightMap[x][y] = 0;
        }
    }

    // Seed queue with lights
    for (size_t i = 0; i < data.lights.size(); i++) {
        const LightSource& light = data.lights[i];
        if (light.x >= 0 && light.y >= 0 && light.x < data.mapWidth && light.y < data.mapHeight) {
            data.lightMap[light.x][light.y] = light.intensity;
            queue.push({ light.x, light.y, (float)light.intensity, light.falloff });
        }
    }

    // 8-directional propagation
    const int dx[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
    const int dy[8] = { -1, -1, 0, 1, 1, 1, 0, -1 };

    while (!queue.empty()) {
        LightNode node = queue.front();
        queue.pop();

        if (node.level <= 1.0f)
            continue;

        for (int i = 0; i < 8; i++) {
            int nx = node.x + dx[i];
            int ny = node.y + dy[i];

            if (nx < 0 || ny < 0 || nx >= data.mapWidth || ny >= data.mapHeight)
                continue;
            if (GetTileAt(map, SHADOW_LAYER, nx, ny) > 0)
                continue;

            float stepCost = (dx[i] == 0 || dy[i] == 0) ? node.falloff : node.falloff * 1.41f;
            float nextLevel = node.level - stepCost;

            if (nextLevel > data.lightMap[nx][ny]) {
                data.lightMap[nx][ny] = nextLevel;
                queue.push({ nx, ny, nextLevel, node.falloff });
            }
        }
    }
}


void UpdateLighting(LightingData& data, Camera2D camera, const TileMap& map) {
    PropagateLight(data, map);
}

Color GetVertexLight(const LightingData& data, int vx, int vy) {
    float sum = 0.0f;
    int count = 0;

    // Sample the 4 surrounding tiles (bottom-right quadrant)
    const int offsets[4][2] = {
            { vx - 1, vy - 1 }, // top-left
            { vx,     vy - 1 }, // top-right
            { vx - 1, vy     }, // bottom-left
            { vx,     vy     }  // bottom-right
    };

    for (int i = 0; i < 4; i++) {
        int tx = offsets[i][0];
        int ty = offsets[i][1];

        if (tx >= 0 && ty >= 0 && tx < data.mapWidth && ty < data.mapHeight) {
            sum += (float)data.lightMap[tx][ty] / 15.0f; // normalize
            count++;
        }
    }

    float avg = (count > 0) ? (sum / count) : 0.0f;
    unsigned char brightness = (unsigned char)(avg * 255.0f);
    return (Color){ brightness, brightness, brightness, 255 };
}


void InitLightingData(LightingData &data, const TileMap& map) {
    data.lights.clear();
    ResizeTileLighting(data, map.width, map.height);
    AddLight(data, 11, 12, 15, 2.5f);
    AddLight(data, 51, 12, 15, 1.0f);
    PropagateLight(data, map);
}

