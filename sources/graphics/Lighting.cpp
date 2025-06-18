//
// Created by bison on 16-06-25.
//

#include "Lighting.h"
#include "data/GameData.h"
#include "util/MathUtil.h"
#include "ai/PathFinding.h"

static void drawOvalLight(Texture2D lightTexture, Vector2 center, float outerRadius) {
    // Size (oval radii)
    float radiusX = outerRadius;
    float radiusY = outerRadius * 0.5f; // squash factor

    // Destination rectangle where we stretch the texture
    Rectangle dest = {
            center.x,
            center.y,
            radiusX * 2,
            radiusY * 2
    };

    // Source is the full texture
    Rectangle source = {
            0, 0,
            (float)lightTexture.width,
            (float)lightTexture.height
    };

    // Origin is center
    Vector2 origin = {
            (float) lightTexture.width,
            (float) lightTexture.height
    };
    origin.x *= 0.5f;
    origin.y *= 0.5f;

    DrawTexturePro(lightTexture, source, dest, origin, 0.0f, WHITE);
}

// Draw the light and shadows to the mask for a light
/*
static void DrawLightMask(LightingData& data, LightInfo& light, Camera2D camera) {
    // Render light

    BeginTextureMode(light.mask);
    ClearBackground(WHITE);

    // Force the blend mode to only set the alpha of the destination
    rlSetBlendFactors(RLGL_SRC_ALPHA, RLGL_SRC_ALPHA, RLGL_MIN);
    rlSetBlendMode(BLEND_CUSTOM);

    Vector2 screenLightPos = GetWorldToScreen2D(light.position, camera);
    screenLightPos = ceilv(screenLightPos); // Round to nearest pixel

    // If we are valid, then draw the light radius to the alpha mask
    if (light.valid) {
        DrawCircleGradient((int) screenLightPos.x, (int) screenLightPos.y, light.outerRadius, ColorAlpha(WHITE, 0), WHITE);
    }


    //if (light.valid) drawOvalLight(data.ovalTexture, screenLightPos, light.outerRadius);

    rlDrawRenderBatchActive();

    // Cut out the shadows from the light radius by forcing the alpha to maximum
    rlSetBlendMode(BLEND_ALPHA);
    rlSetBlendFactors(RLGL_SRC_ALPHA, RLGL_SRC_ALPHA, RLGL_MAX);
    rlSetBlendMode(BLEND_CUSTOM);

    Vector2 vertices[4];

    // Draw the shadows to the alpha mask
    for (int i = 0; i < light.shadowCount; i++) {
        // convert vertices to screen space
        vertices[0] = GetWorldToScreen2D(light.shadows[i].vertices[0], camera);
        vertices[1] = GetWorldToScreen2D(light.shadows[i].vertices[1], camera);
        vertices[2] = GetWorldToScreen2D(light.shadows[i].vertices[2], camera);
        vertices[3] = GetWorldToScreen2D(light.shadows[i].vertices[3], camera);
        DrawTriangleFan(vertices, 4, WHITE);
    }

    rlDrawRenderBatchActive();

    // Go back to normal blend mode
    rlSetBlendMode(BLEND_ALPHA);
    EndTextureMode();

    // render shadow
    BeginTextureMode(light.shadow);
    ClearBackground(WHITE);
    // Force the blend mode to only set the alpha of the destination
    rlSetBlendFactors(RLGL_SRC_ALPHA, RLGL_SRC_ALPHA, RLGL_MIN);
    rlSetBlendMode(BLEND_CUSTOM);

    if (light.valid) {
        DrawCircleGradient((int) screenLightPos.x, (int) screenLightPos.y, light.outerRadius, ColorAlpha(WHITE, 0), WHITE);
    }
    //if (light.valid) drawOvalLight(data.ovalTexture, screenLightPos, light.outerRadius);

    rlDrawRenderBatchActive();
    // Go back to normal blend mode
    rlSetBlendMode(BLEND_ALPHA);
    EndTextureMode();
}
*/

void MoveLight(LightInfo& light, float x, float y) {
    light.position.x = x;
    light.position.y = y;

    // update the cached bounds
    light.bounds.x = x - light.radius;
    light.bounds.y = y - light.radius;
}

// Compute a shadow volume for the edge
// It takes the edge and projects it back by the light radius and turns it into a quad
static void ComputeShadowVolumeForEdge(LightInfo& light, Vector2 sp, Vector2 ep) {
    if (light.shadowCount >= MAX_SHADOWS) return;

    float extension = light.radius*2;

    Vector2 spVector = Vector2Normalize(Vector2Subtract(sp, light.position));
    Vector2 spProjection = Vector2Add(sp, Vector2Scale(spVector, extension));

    Vector2 epVector = Vector2Normalize(Vector2Subtract(ep, light.position));
    Vector2 epProjection = Vector2Add(ep, Vector2Scale(epVector, extension));

    light.shadows[light.shadowCount].vertices[0] = sp;
    light.shadows[light.shadowCount].vertices[1] = ep;
    light.shadows[light.shadowCount].vertices[2] = epProjection;
    light.shadows[light.shadowCount].vertices[3] = spProjection;

    light.shadowCount++;
}

void UpdateLight(LightingData& data, LightInfo& light, Camera2D camera) {
    if (!light.active) return;

    light.shadowCount = 0;

    for (int i = 0; i < data.boxes.size(); i++)
    {
        // Are we in a box? if so we are not valid
        if (CheckCollisionPointRec(light.position, data.boxes[i])) return;

        // If this box is outside our bounds, we can skip it
        if (!CheckCollisionRecs(light.bounds, data.boxes[i])) continue;

        // Check the edges that are on the same side we are, and cast shadow volumes out from them

        // Top
        Vector2 sp = (Vector2){ data.boxes[i].x, data.boxes[i].y };
        Vector2 ep = (Vector2){ data.boxes[i].x + data.boxes[i].width, data.boxes[i].y };

        if (light.position.y > ep.y) ComputeShadowVolumeForEdge(light, sp, ep);

        // Right
        sp = ep;
        ep.y += data.boxes[i].height;
        if (light.position.x < ep.x) ComputeShadowVolumeForEdge(light, sp, ep);

        // Bottom
        sp = ep;
        ep.x -= data.boxes[i].width;
        if (light.position.y < ep.y) ComputeShadowVolumeForEdge(light, sp, ep);

        // Left
        sp = ep;
        ep.y -= data.boxes[i].height;
        if (light.position.x > ep.x) ComputeShadowVolumeForEdge(light, sp, ep);

        // The box itself
        light.shadows[light.shadowCount].vertices[0] = (Vector2){ data.boxes[i].x, data.boxes[i].y };
        light.shadows[light.shadowCount].vertices[1] = (Vector2){ data.boxes[i].x, data.boxes[i].y + data.boxes[i].height };
        light.shadows[light.shadowCount].vertices[2] = (Vector2){ data.boxes[i].x + data.boxes[i].width, data.boxes[i].y + data.boxes[i].height };
        light.shadows[light.shadowCount].vertices[3] = (Vector2){ data.boxes[i].x + data.boxes[i].width, data.boxes[i].y };
        light.shadowCount++;
    }
}

void DrawLightOld(const LightingData& data, const LightInfo& light, Camera2D camera) {
    if (!light.active || light.intensity <= 0.0f || light.radius <= 0.0f)
        return;

    // Convert world position to screen
    Vector2 screenPos = GetWorldToScreen2D(light.position, camera);

    // Clamp intensity
    float alpha = Clamp(light.intensity, 0, 1.0f);

    // Scale light color by intensity (optional: could also modulate brightness in shader)
    Color centerColor = ColorAlpha(light.color, alpha);   // Bright center
    Color edgeColor = ColorAlpha(light.color, 0.0f);      // Transparent edge

    // NOTE: Raylib's DrawCircleGradient takes (edgeColor, centerColor)
    DrawCircleGradient((int)screenPos.x, (int)screenPos.y, light.radius, centerColor, edgeColor);
}

void DrawLight(const LightingData& data, const LightInfo& light, Camera2D camera) {
    if (!light.active || light.intensity <= 0.0f || light.radius <= 0.0f)
        return;

    Vector2 screenPos = GetWorldToScreen2D(light.position, camera);

    Color tint = ColorAlpha(light.color, light.intensity);

    DrawTexturePro(
            light.texture,
            (Rectangle){ 0, 0, (float) light.texture.width, (float) light.texture.height },
            (Rectangle){ screenPos.x - light.radius, screenPos.y - light.radius, light.radius * 2, light.radius * 2 },
            (Vector2){ 0, 0 },
            0.0f,
            WHITE
    );
}

void DrawShadow(const LightingData& data, const LightInfo& light, Camera2D camera) {
    if (!light.active || light.intensity <= 0.0f || light.radius <= 0.0f)
        return;
    Vector2 vertices[4];
    // Draw the shadows to the alpha mask
    for (int i = 0; i < light.shadowCount; i++) {
        // convert vertices to screen space
        vertices[0] = GetWorldToScreen2D(light.shadows[i].vertices[0], camera);
        vertices[1] = GetWorldToScreen2D(light.shadows[i].vertices[1], camera);
        vertices[2] = GetWorldToScreen2D(light.shadows[i].vertices[2], camera);
        vertices[3] = GetWorldToScreen2D(light.shadows[i].vertices[3], camera);
        DrawTriangleFan(vertices, 4, Fade(BLACK, 0.25f));
    }
    rlDrawRenderBatchActive();
}


void UpdateLighting(LightingData& data, Camera2D camera) {
    for(auto& light : data.lights) {
        UpdateLight(data, light, camera);
    }
    // Build up the light mask
    BeginTextureMode(data.lightMask);
    ClearBackground(ColorAlpha(BLACK, data.ambient));
    // Set blend mode for lights to add brightness
    rlSetBlendMode(BLEND_ADDITIVE); // OR a custom blend if needed
    for(auto& light : data.lights) {
        DrawLight(data, light, camera);
    }
    rlSetBlendMode(BLEND_ALPHA); // reset blend mode
    EndTextureMode();

    BeginTextureMode(data.shadowMask);
    ClearBackground(BLANK);
    // Set blend mode for lights to add brightness
    //rlSetBlendMode(BLEND_ADDITIVE); // OR a custom blend if needed
    for(auto& light : data.lights) {
        DrawShadow(data, light, camera);
    }
    rlSetBlendMode(BLEND_ALPHA); // reset blend mode
    EndTextureMode();
}


void RenderLighting(LightingData& data) {
    rlSetBlendMode(BLEND_MULTIPLIED);
    DrawTexturePro(
            data.lightMask.texture,
            { 0, 0, (float) data.lightMask.texture.width, -(float) data.lightMask.texture.height },
            { 0, 0, gameScreenWidthF, gameScreenHeightF },
            { 0, 0 },
            0,
            WHITE
    );
    rlSetBlendMode(BLEND_ALPHA);
}

void RenderShadows(LightingData& data) {
    //rlSetBlendMode(BLEND_MULTIPLIED);
    DrawTexturePro(
            data.shadowMask.texture,
            { 0, 0, (float) data.shadowMask.texture.width, -(float) data.shadowMask.texture.height },
            { 0, 0, gameScreenWidthF, gameScreenHeightF },
            { 0, 0 },
            0,
            WHITE
    );
    rlSetBlendMode(BLEND_ALPHA);
}

void BuildShadowBoxes(LightingData &data, TileMap &tileMap) {
    const int w = tileMap.width;
    const int h = tileMap.height;
    std::vector<std::vector<bool>> solid(h, std::vector<bool>(w, false));

    // Step 1: Build solid mask
    for (int ty = 0; ty < h; ty++) {
        for (int tx = 0; tx < w; tx++) {
            int tileIndex = GetTileAt(tileMap, SHADOW_LAYER, tx, ty);
            solid[ty][tx] = (tileIndex > 0);
        }
    }

    std::vector<std::vector<bool>> visited(h, std::vector<bool>(w, false));
    data.boxes.clear();

    // Step 2: Scan for rectangles
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (!solid[y][x] || visited[y][x]) continue;

            // Find width
            int xEnd = x;
            while (xEnd < w && solid[y][xEnd] && !visited[y][xEnd]) {
                xEnd++;
            }

            // Find height
            int yEnd = y + 1;
            bool done = false;
            while (yEnd < h && !done) {
                for (int i = x; i < xEnd; i++) {
                    if (!solid[yEnd][i] || visited[yEnd][i]) {
                        done = true;
                        break;
                    }
                }
                if (!done) yEnd++;
            }

            // Mark visited
            for (int iy = y; iy < yEnd; iy++) {
                for (int ix = x; ix < xEnd; ix++) {
                    visited[iy][ix] = true;
                }
            }

            // Create rectangle
            Vector2 topLeft = GridToPixelPosition(x, y);
            Rectangle rect = {
                    topLeft.x - 8,
                    topLeft.y - 8,
                    (float)(xEnd - x) * 16.0f,
                    (float)(yEnd - y) * 16.0f
            };

            data.boxes.push_back(rect);
            if (data.boxes.size() >= MAX_BOXES)
                return;
        }
    }
    TraceLog(LOG_INFO, "Generated %i shadow boxes.", data.boxes.size());
}

Texture2D GenerateRadialLightTexture(int size) {
    Image img = GenImageColor(size, size, BLANK);

    Color* pixels = LoadImageColors(img);

    Vector2 center = { size / 2.0f, size / 2.0f };
    float radius = size / 2.0f;

    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            float dx = (float)x - center.x;
            float dy = (float)y - center.y;
            float dist = sqrtf(dx*dx + dy*dy);

            float norm = dist / radius;  // 0.0 = center, 1.0 = edge

            // Falloff function: soft edge
            float alpha = 1.0f - norm;
            alpha = Clamp(alpha, 0.0f, 1.0f);
            alpha = powf(alpha, 1.5f); // exponent controls softness
            //pixels[y * size + x] = (Color){ 255, 255, 255, (unsigned char)(alpha * 255) };

            unsigned char a = (unsigned char)(alpha * 255);
            pixels[y * size + x] = (Color){ a, a, a, a }; // RGB premultiplied by alpha
        }
    }

    UnloadImage(img);
    img.data = pixels; // Reassign our modified pixel data
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img); // Don't need the CPU copy anymore

    return tex;
}

Texture2D GenerateRadialLightTextureColored(int size, Color color, float intensity, float falloff) {
    intensity = Clamp(intensity, 0.0f, 1.0f);
    falloff = Clamp(falloff, 0.01f, 10.0f); // avoid division by 0 or absurd values

    Image img = GenImageColor(size, size, BLANK);
    Color* pixels = LoadImageColors(img);

    Vector2 center = { size / 2.0f, size / 2.0f };
    float radius = size / 2.0f;

    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            float dx = (float)x - center.x;
            float dy = (float)y - center.y;
            float dist = sqrtf(dx*dx + dy*dy);

            float norm = dist / radius;
            float alpha = Clamp(1.0f - norm, 0.0f, 1.0f);
            alpha = powf(alpha, falloff); // << falloff control

            float finalAlpha = alpha * intensity;

            unsigned char a = (unsigned char)(finalAlpha * 255);
            unsigned char r = (unsigned char)(color.r * finalAlpha);
            unsigned char g = (unsigned char)(color.g * finalAlpha);
            unsigned char b = (unsigned char)(color.b * finalAlpha);

            pixels[y * size + x] = (Color){ r, g, b, a };
        }
    }

    UnloadImage(img);
    img.data = pixels;
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);

    return tex;
}



void AddLight(LightingData& data, float x, float y, float radius, float intensity, float falloff, Color color) {
    LightInfo light{};
    light.intensity = intensity;
    light.active = true;
    light.radius = radius;
    light.bounds.width = radius * 2;
    light.bounds.height = radius * 2;
    light.color = color;
    light.texture = GenerateRadialLightTextureColored(256, color, intensity, falloff);
    //SetTextureFilter(light.texture, TEXTURE_FILTER_BILINEAR);
    MoveLight(light, x, y);
    data.lights.push_back(light);
}

void InitLightingData(LightingData &data) {
    if(IsRenderTextureValid(data.lightMask)) {
        UnloadRenderTexture(data.lightMask);
    }
    data.lightMask = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    if(IsRenderTextureValid(data.shadowMask)) {
        UnloadRenderTexture(data.shadowMask);
    }
    data.shadowMask = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    for(auto& light : data.lights) {
        if(IsTextureValid(light.texture)) {
            UnloadTexture(light.texture);
        }
    }
    data.lights.clear();

    Color TORCHLIGHT_COLOR = (Color){ 214, 142, 100, 255 };

    AddLight(data, 100, 100, 128, 1.0f, 0.75f, WHITE);
    AddLight(data, 800, 140, 75, 0.75f, 1.0f, WHITE);

    data.boxes.clear();
    data.ambient = 0.9f; // 80% darkness
    //data.boxes.push_back((Rectangle){ 150, 80, 40, 40 });
    //data.boxes.push_back((Rectangle){ 450, 80, 40, 40 });
}