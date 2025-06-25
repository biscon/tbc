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

void AddLight(LightingData& data, std::string id, int x, int y, uint8_t intensity, float falloff, Color color, bool active) {
    LightSource light{};
    light.intensity = intensity;
    light.active = active;
    light.falloff = falloff;
    light.color = color;
    light.id = id;

    MoveLight(light, x, y);
    data.lights.push_back(light);
}


static void ResizeTileLighting(LightingData &data, int newWidth, int newHeight) {
    for (int x = 0; x < data.mapWidth; x++) {
        if(data.lightMapR != nullptr) delete[] data.lightMapR[x];
        if(data.lightMapG != nullptr) delete[] data.lightMapG[x];
        if(data.lightMapB != nullptr) delete[] data.lightMapB[x];
        if(data.visibilityMap != nullptr) delete[] data.visibilityMap[x];
    }
    if(data.lightMapR != nullptr) delete[] data.lightMapR;
    if(data.lightMapG != nullptr) delete[] data.lightMapG;
    if(data.lightMapB != nullptr) delete[] data.lightMapB;
    if(data.visibilityMap != nullptr) delete[] data.visibilityMap;

    data.mapWidth = newWidth;
    data.mapHeight = newHeight;

    data.lightMapR = new float*[newWidth];
    data.lightMapG = new float*[newWidth];
    data.lightMapB = new float*[newWidth];
    data.visibilityMap = new bool*[newWidth];
    for (int x = 0; x < newWidth; x++) {
        data.lightMapR[x] = new float[newHeight]();
        data.lightMapG[x] = new float[newHeight]();
        data.lightMapB[x] = new float[newHeight]();
        data.visibilityMap[x] = new bool[newHeight]();
    }
}

void PropagateLight(LightingData& data, const TileMap& map) {
    struct LightNode {
        int x;
        int y;
        float levelR, levelG, levelB;
        float falloff;
    };

    std::queue<LightNode> queue;
    //Vector4 ambient = ColorNormalize(data.ambient);

    Vector4 ambient = {
            data.ambient.r / 255.0f,
            data.ambient.g / 255.0f,
            data.ambient.b / 255.0f,
            1.0f
    };

    // Reset lightmaps
    for (int y = 0; y < data.mapHeight; y++) {
        for (int x = 0; x < data.mapWidth; x++) {
            data.lightMapR[x][y] = ambient.x * 15.0f;
            data.lightMapG[x][y] = ambient.y * 15.0f;
            data.lightMapB[x][y] = ambient.z * 15.0f;
        }
    }

    // Seed queue with lights
    for (const LightSource& light : data.lights) {
        if (light.x < 0 || light.y < 0 || light.x >= data.mapWidth || light.y >= data.mapHeight)
            continue;

        if(!light.active)
            continue;

        float intensity = static_cast<float>(light.intensity);
        float r = (light.color.r / 255.0f) * intensity;
        float g = (light.color.g / 255.0f) * intensity;
        float b = (light.color.b / 255.0f) * intensity;

        data.lightMapR[light.x][light.y] = r;
        data.lightMapG[light.x][light.y] = g;
        data.lightMapB[light.x][light.y] = b;

        queue.push({ light.x, light.y, r, g, b, light.falloff });
    }

    // 8-directional propagation
    const int dx[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
    const int dy[8] = { -1, -1, 0, 1, 1, 1, 0, -1 };

    while (!queue.empty()) {
        LightNode node = queue.front();
        queue.pop();

        if (node.levelR <= 0.01f && node.levelG <= 0.01f && node.levelB <= 0.01f)
            continue;

        for (int i = 0; i < 8; i++) {
            int nx = node.x + dx[i];
            int ny = node.y + dy[i];

            if (nx < 0 || ny < 0 || nx >= data.mapWidth || ny >= data.mapHeight)
                continue;
            if (GetTileAt(map, SHADOW_LAYER, nx, ny) > 0)
                continue;

            float stepCost = (dx[i] == 0 || dy[i] == 0) ? node.falloff : node.falloff * 1.41f;

            float nextR = node.levelR - stepCost;
            float nextG = node.levelG - stepCost;
            float nextB = node.levelB - stepCost;

            bool pushed = false;

            if (nextR > data.lightMapR[nx][ny]) {
                data.lightMapR[nx][ny] = nextR;
                pushed = true;
            }
            if (nextG > data.lightMapG[nx][ny]) {
                data.lightMapG[nx][ny] = nextG;
                pushed = true;
            }
            if (nextB > data.lightMapB[nx][ny]) {
                data.lightMapB[nx][ny] = nextB;
                pushed = true;
            }

            if (pushed) {
                queue.push({ nx, ny,
                             std::max(0.0f, nextR),
                             std::max(0.0f, nextG),
                             std::max(0.0f, nextB),
                             node.falloff });
            }
        }
    }
}

Color GetVertexLight(const LightingData& data, int vx, int vy) {
    float r = 0.0f, g = 0.0f, b = 0.0f;
    int count = 0;

    // Sample 2x2 tiles for smooth interpolation
    const int offsets[4][2] = { {0, 0}, {-1, 0}, {-1, -1}, {0, -1} };

    for (int i = 0; i < 4; i++) {
        int tx = vx + offsets[i][0];
        int ty = vy + offsets[i][1];

        if (tx >= 0 && ty >= 0 && tx < data.mapWidth && ty < data.mapHeight) {
            r += data.lightMapR[tx][ty] / 15.0f;
            g += data.lightMapG[tx][ty] / 15.0f;
            b += data.lightMapB[tx][ty] / 15.0f;

            count++;
        }
    }

    if (count > 0) {
        r /= count;
        g /= count;
        b /= count;
    }

    return {
            (unsigned char)(Clamp(r, 0.0f, 1.0f) * 255),
            (unsigned char)(Clamp(g, 0.0f, 1.0f) * 255),
            (unsigned char)(Clamp(b, 0.0f, 1.0f) * 255),
            255
    };
}

Color GetVertexLight(const LightingData& data, const TileMap& map, int vx, int vy) {
    float r = 0.0f, g = 0.0f, b = 0.0f;
    int count = 0;

    const int offsets[4][2] = { {0, 0}, {-1, 0}, {-1, -1}, {0, -1} };

    for (int i = 0; i < 4; i++) {
        int tx = vx + offsets[i][0];
        int ty = vy + offsets[i][1];

        if (tx >= 0 && ty >= 0 && tx < data.mapWidth && ty < data.mapHeight) {
            if (GetTileAt(map, SHADOW_LAYER, tx, ty) > 0)
                continue; // Skip solid (shadow-casting) tiles

            r += data.lightMapR[tx][ty] / 15.0f;
            g += data.lightMapG[tx][ty] / 15.0f;
            b += data.lightMapB[tx][ty] / 15.0f;
            count++;
        }
    }

    if (count > 0) {
        r /= count;
        g /= count;
        b /= count;
    }

    return {
            (unsigned char)(Clamp(r, 0.0f, 1.0f) * 255),
            (unsigned char)(Clamp(g, 0.0f, 1.0f) * 255),
            (unsigned char)(Clamp(b, 0.0f, 1.0f) * 255),
            255
    };
}

Color GetVertexLightWeighted(const LightingData& data, int vx, int vy) {
    float rSum = 0.0f, gSum = 0.0f, bSum = 0.0f;
    float weightSum = 0.0f;

    const int offsets[4][2] = { {0, 0}, {-1, 0}, {-1, -1}, {0, -1} };

    for (int i = 0; i < 4; i++) {
        int tx = vx + offsets[i][0];
        int ty = vy + offsets[i][1];

        if (tx >= 0 && ty >= 0 && tx < data.mapWidth && ty < data.mapHeight) {
            float r,g,b;
            r = data.lightMapR[tx][ty] / 15.0f;
            g = data.lightMapG[tx][ty] / 15.0f;
            b = data.lightMapB[tx][ty] / 15.0f;


            // Use the brightness as weight (or optionally max(r, g, b))
            float weight = (r + g + b) / 3.0f;

            // Avoid over-weighting very dark or zero-light tiles
            if (weight > 0.01f) {
                rSum += r * weight;
                gSum += g * weight;
                bSum += b * weight;
                weightSum += weight;
            }
        }
    }

    if (weightSum > 0.0f) {
        rSum /= weightSum;
        gSum /= weightSum;
        bSum /= weightSum;
    }

    return {
            (unsigned char)(Clamp(rSum, 0.0f, 1.0f) * 255),
            (unsigned char)(Clamp(gSum, 0.0f, 1.0f) * 255),
            (unsigned char)(Clamp(bSum, 0.0f, 1.0f) * 255),
            255
    };
}

static float SmoothVisibilityAlpha(LightingData &data, int x, int y) {
    int visibleCount = 0;
    int total = 0;

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && ny >= 0 && nx < data.mapWidth && ny < data.mapHeight) {
                total++;
                if (data.visibilityMap[nx][ny])
                    visibleCount++;
            }
        }
    }

    return 1.0f - (float)visibleCount / (float)total;
}

void UpdateVisibilityTexture(LightingData& data) {
    for (int y = 0; y < data.mapHeight; y++) {
        for (int x = 0; x < data.mapWidth; x++) {
            bool visible = data.visibilityMap[x][y];
            data.pixels[y * data.mapWidth + x] = visible
                                    ? (Color){ 0, 0, 0, 0 }       // fully transparent
                                    : (Color){ 0, 0, 0, 255 };    // fully opaque black
        }
    }

    UpdateTexture(data.visTexture, data.pixels);
}


void RenderVisibilityMap(LightingData &data) {
    float tileSize = 16.0f;
    DrawTexturePro(
            data.visTexture,
            (Rectangle){ 0, 0, (float) data.mapWidth, (float) data.mapHeight },
            (Rectangle){ 0, 0, data.mapWidth * tileSize, data.mapHeight * tileSize },
            (Vector2){ 0, 0 },
            0.0f,
            ColorAlpha(WHITE, 1.0f) // color tint, no alpha applied here
    );
}


Texture2D GenerateVisibilityTexture(int width, int height) {
    Image img = GenImageColor(width, height, BLANK); // Just to get format/dimensions
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img); // no longer needed
    return tex;
}

void InitLightingData(LightingData &data, const TileMap& map) {
    //data.ambient = DARKBLUE;
    Color torchLightWarm1 = { 255, 220, 140, 255 };  // Soft creamy yellow
    Color torchLightWarm2 = { 255, 230, 180, 255 };  // Brighter warm yellow-white
    Color torchLightWarm3 = { 255, 240, 200, 255 };  // Very pale yellowish-white

    data.ambient = { 5, 10, 30, 255 };
    data.lights.clear();
    ResizeTileLighting(data, map.width, map.height);
    /*
    AddLight(data, 11, 12, 15, 2.5f, torchLightWarm1);
    AddLight(data, 51, 12, 15, 2.0f, torchLightWarm2);
    AddLight(data, 31, 32, 15, 1.0f, torchLightWarm3);
     */

    if(IsTextureValid(data.visTexture)) {
        UnloadTexture(data.visTexture);
    }
    data.visTexture = GenerateVisibilityTexture(data.mapWidth, data.mapHeight);
    SetTextureFilter(data.visTexture, TEXTURE_FILTER_TRILINEAR);
    if(data.pixels != nullptr) {
        MemFree(data.pixels);
    }
    data.pixels = (Color*)MemAlloc(data.mapWidth * data.mapHeight * sizeof(Color));
}

