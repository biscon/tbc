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

void PropagateLight(LightingData& data, TileMap& map) {
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
            if (GetTileAt(map, LIGHT_LAYER, nx, ny) > 0)
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

Color GetVertexLight(const LightingData& data, TileMap& map, int vx, int vy) {
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

Color GetVertexLightWeighted(const LightingData& data, float vx, float vy) {
    // floor and frac
    int x0 = (int)floor(vx);
    int y0 = (int)floor(vy);
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    float fx = vx - (float)x0;
    float fy = vy - (float)y0;

    // inner lambda: apply your original weighting logic at an integer vertex
    auto sampleWeighted = [&](int vxInt, int vyInt) -> std::array<float,3> {
        float rSum = 0.0f, gSum = 0.0f, bSum = 0.0f;
        float weightSum = 0.0f;

        const int offsets[4][2] = { {0, 0}, {-1, 0}, {-1, -1}, {0, -1} };

        for (int i = 0; i < 4; i++) {
            int tx = vxInt + offsets[i][0];
            int ty = vyInt + offsets[i][1];

            if (tx >= 0 && ty >= 0 && tx < data.mapWidth && ty < data.mapHeight) {
                float r = data.lightMapR[tx][ty] / 15.0f;
                float g = data.lightMapG[tx][ty] / 15.0f;
                float b = data.lightMapB[tx][ty] / 15.0f;

                float weight = (r + g + b) / 3.0f; // brightness as weight
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

        return { rSum, gSum, bSum };
    };

    // sample four corners
    auto c00 = sampleWeighted(x0, y0);
    auto c10 = sampleWeighted(x1, y0);
    auto c01 = sampleWeighted(x0, y1);
    auto c11 = sampleWeighted(x1, y1);

    // bilinear blend
    float r = (1 - fx) * (1 - fy) * c00[0] +
              (    fx) * (1 - fy) * c10[0] +
              (1 - fx) * (    fy) * c01[0] +
              (    fx) * (    fy) * c11[0];

    float g = (1 - fx) * (1 - fy) * c00[1] +
              (    fx) * (1 - fy) * c10[1] +
              (1 - fx) * (    fy) * c01[1] +
              (    fx) * (    fy) * c11[1];

    float b = (1 - fx) * (1 - fy) * c00[2] +
              (    fx) * (1 - fy) * c10[2] +
              (1 - fx) * (    fy) * c01[2] +
              (    fx) * (    fy) * c11[2];

    return {
            (unsigned char)(Clamp(r, 0.0f, 1.0f) * 255),
            (unsigned char)(Clamp(g, 0.0f, 1.0f) * 255),
            (unsigned char)(Clamp(b, 0.0f, 1.0f) * 255),
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
            WHITE // color tint, no alpha applied here
    );
}


Texture2D GenerateVisibilityTexture(int width, int height) {
    Image img = GenImageColor(width, height, BLANK); // Just to get format/dimensions
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img); // no longer needed
    return tex;
}

void InitLightingData(LightingData &data, TileMap& map) {
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



Color CalcOutdoorAmbientColor(float hour) {
    if(hour < 0) hour += 24;
    if(hour >= 24) hour -= 24;

    struct KeyPoint { float hour; Color col; };
    static const KeyPoint keys[] = {
            {0.0f,   {5,10,30,255}},
            {4.0f,   {5,10,30,255}},
            {5.0f,   {100,60,40,255}},
            {6.0f,   {255,155,66,255}},
            {7.0f,   {255,200,150,255}},
            {12.0f,  {255,255,255,255}},
            {17.0f,  {255,200,150,255}},
            {19.0f,  {155,94,66,255}},
            {20.0f,  {100,60,40,255}},
            {21.0f,  {20,30,60,255}},
            {24.0f,  {5,10,30,255}}
    };

    int numKeys = sizeof(keys)/sizeof(keys[0]);

    // Find the segment
    for(int i = 0; i < numKeys - 1; i++) {
        if(hour >= keys[i].hour && hour < keys[i+1].hour) {
            float t = (hour - keys[i].hour) / (keys[i+1].hour - keys[i].hour);
            return LerpColor(keys[i].col, keys[i+1].col, t);
        }
    }

    // Handle wrap-around: from last key to first key (23:59 -> 0:00)
    float t = (hour - keys[numKeys - 1].hour) / ((24.0f + keys[0].hour) - keys[numKeys - 1].hour);
    return LerpColor(keys[numKeys - 1].col, keys[0].col, t);
}

struct KeyPoint { float hour; Color col; };

static const KeyPoint keysOvercast[] = {
        {0.0f,   {10, 10, 20, 255}},   // midnight - very dark bluish gray
        {4.0f,   {15, 15, 30, 255}},   // early morning - slightly lighter
        {6.0f,   {50, 60, 70, 255}},   // dawn - dim gray-blue
        {9.0f,   {90, 100, 110, 255}}, // morning - brighter but dull
        {12.0f,  {120, 130, 140, 255}},// noon - flat, pale gray
        {15.0f,  {100, 110, 120, 255}},// afternoon - slightly darker
        {18.0f,  {70, 80, 90, 255}},   // evening - dim gray-blue
        {20.0f,  {40, 50, 60, 255}},   // dusk - darker gray
        {22.0f,  {20, 25, 35, 255}},   // late night - almost black-gray
        {24.0f,  {10, 10, 20, 255}}    // wrap around
};

static const KeyPoint keysSunny[] = {
        {0.0f,   {5,10,30,255}},
        {4.0f,   {5,10,30,255}},
        {5.0f,   {100,60,40,255}},
        {6.0f,   {255,155,66,255}},
        {7.0f,   {255,200,150,255}},
        {12.0f,  {255,255,255,255}},
        {17.0f,  {255,200,150,255}},
        {19.0f,  {155,94,66,255}},
        {20.0f,  {100,60,40,255}},
        {21.0f,  {20,30,60,255}},
        {24.0f,  {5,10,30,255}}
};


Color CalcOutdoorAmbientColorCubic(float hour) {
    if(hour < 0) hour += 24;
    if(hour >= 24) hour -= 24;

    auto& keys = keysOvercast;

    int numKeys = sizeof(keys)/sizeof(keys[0]);

    for(int i = 0; i < numKeys - 1; i++) {
        if(hour >= keys[i].hour && hour < keys[i+1].hour) {
            float t = (hour - keys[i].hour) / (keys[i+1].hour - keys[i].hour);
            t = EaseInOutCubic(t); // apply cubic easing
            return LerpColor(keys[i].col, keys[i+1].col, t);
        }
    }

    // Wrap-around last key -> first key
    float t = (hour - keys[numKeys - 1].hour) / ((24.0f + keys[0].hour) - keys[numKeys - 1].hour);
    t = EaseInOutCubic(t);
    return LerpColor(keys[numKeys - 1].col, keys[0].col, t);
}
