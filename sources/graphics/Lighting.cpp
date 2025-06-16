//
// Created by bison on 16-06-25.
//

#include "Lighting.h"
#include "data/GameData.h"
#include "util/MathUtil.h"
#include "ai/PathFinding.h"

// Draw the light and shadows to the mask for a light
static void DrawLightMask(LightInfo& light, Camera2D camera) {
    // Use the light mask
    BeginTextureMode(light.mask);

    ClearBackground(WHITE);

    // Force the blend mode to only set the alpha of the destination
    rlSetBlendFactors(RLGL_SRC_ALPHA, RLGL_SRC_ALPHA, RLGL_MIN);
    rlSetBlendMode(BLEND_CUSTOM);

    Vector2 screenLightPos = GetWorldToScreen2D(light.position, camera);
    screenLightPos = ceilv(screenLightPos); // Round to nearest pixel

    // If we are valid, then draw the light radius to the alpha mask
    if (light.valid) DrawCircleGradient((int) screenLightPos.x, (int) screenLightPos.y, light.outerRadius, ColorAlpha(WHITE, 0), WHITE);

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
}

void MoveLight(LightInfo& light, float x, float y) {
    light.position.x = x;
    light.position.y = y;

    // update the cached bounds
    light.bounds.x = x - light.outerRadius;
    light.bounds.y = y - light.outerRadius;
}

void AddLight(LightingData& data, float x, float y, float radius) {
    LightInfo light{};
    light.active = true;
    light.valid = false;  // The light must prove it is valid
    light.mask = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    light.outerRadius = radius;
    light.bounds.width = radius * 2;
    light.bounds.height = radius * 2;

    MoveLight(light, x, y);
    data.lights.push_back(light);

    // Force the render texture to have something in it
    //DrawLightMask(light);
}

// Compute a shadow volume for the edge
// It takes the edge and projects it back by the light radius and turns it into a quad
static void ComputeShadowVolumeForEdge(LightInfo& light, Vector2 sp, Vector2 ep) {
    if (light.shadowCount >= MAX_SHADOWS) return;

    float extension = light.outerRadius*2;

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
    light.valid = false;

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
    light.valid = true;
    DrawLightMask(light, camera);
}

void InitLightingData(LightingData &data) {
    for(auto& light : data.lights) {
        if(IsRenderTextureValid(light.mask)) {
            UnloadRenderTexture(light.mask);
        }
    }
    if(IsRenderTextureValid(data.lightMask)) {
        UnloadRenderTexture(data.lightMask);
    }
    data.lightMask = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    data.lights.clear();
    AddLight(data, 100, 100, 100);
    //AddLight(data, 200, 200, 100);

    data.boxes.clear();
    //data.boxes.push_back((Rectangle){ 150, 80, 40, 40 });
    //data.boxes.push_back((Rectangle){ 450, 80, 40, 40 });
}

void UpdateLighting(LightingData& data, Camera2D camera) {
    for(auto& light : data.lights) {
        UpdateLight(data, light, camera);
    }
    // Build up the light mask
    BeginTextureMode(data.lightMask);

    ClearBackground(BLACK);

    // Force the blend mode to only set the alpha of the destination
    rlSetBlendFactors(RLGL_SRC_ALPHA, RLGL_SRC_ALPHA, RLGL_MIN);
    rlSetBlendMode(BLEND_CUSTOM);
    //rlSetBlendMode(BLEND_ADDITIVE);

    // Merge in all the light masks
    for (int i = 0; i < data.lights.size(); i++) {
        if (data.lights[i].active) DrawTextureRec(data.lights[i].mask.texture, (Rectangle){ 0, 0, gameScreenWidthF, -gameScreenHeightF }, Vector2Zero(), WHITE);
    }

    rlDrawRenderBatchActive();

    // Go back to normal blend
    rlSetBlendMode(BLEND_ALPHA);
    EndTextureMode();
}

void RenderLighting(LightingData& data, Camera2D camera) {
    // Overlay the shadows from all the lights

    DrawTexturePro(
            data.lightMask.texture,
            { 0, 0, (float) data.lightMask.texture.width, -(float) data.lightMask.texture.height },
            { 0, 0, gameScreenWidthF, gameScreenHeightF },
            { 0, 0 },
            0,
            Fade(BLACK, 0.65f)
    );

    // Draw the lights
    for (int i = 0; i < data.lights.size(); i++)
    {
        Vector2 screenPos = GetWorldToScreen2D(data.lights[i].position, camera);
        screenPos = ceilv(screenPos); // Round to nearest pixel
        if (data.lights[i].active) DrawCircle((int) screenPos.x, (int) screenPos.y, 5, (i == 0)? YELLOW : WHITE);
    }
}

void BuildShadowBoxes(LightingData &data, TileMap &tileMap) {
    const int w = tileMap.width;
    const int h = tileMap.height;
    std::vector<std::vector<bool>> solid(h, std::vector<bool>(w, false));

    // Step 1: Build solid mask
    for (int ty = 0; ty < h; ty++) {
        for (int tx = 0; tx < w; tx++) {
            int tileIndex = GetTileAt(tileMap, NAV_LAYER, tx, ty);
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

