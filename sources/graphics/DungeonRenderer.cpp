//
// Created by bison on 31-01-25.
//

#include <algorithm>
#include "DungeonRenderer.h"
#include "rlgl.h"
#include "raymath.h"
#include "util/MathUtil.h"

const float headBobAmplitude = 0.025f;  // Amplitude of the head bob effect

Vector3 GetMovedBackPosition(DungeonState& state, float moveBack) {
    // Get forward vector (normalized direction from position to target)
    Vector3 forward = Vector3Normalize(Vector3Subtract(state.camera.target, state.camera.position));

    // Move back by moveBack distance (opposite direction of forward)
    return Vector3Subtract(state.camera.position, Vector3Scale(forward, moveBack));
}

void StartMove(DungeonState& state, Vector2 newPosition) {
    state.moveTarget = newPosition;
    state.moveStart = state.player.position;
    state.isMoving = true;
    state.moveTimer = state.moveDuration;
    TraceLog(LOG_INFO, "StartMove: %f, %f", newPosition.x, newPosition.y);
}

void UpdateMove(DungeonState& state, float deltaTime) {
    if (!state.isMoving) return;
    state.moveTimer -= deltaTime;
    if (state.moveTimer <= 0.0f) {
        state.isMoving = false;
        state.moveTimer = 0.0f;
        TraceLog(LOG_INFO, "Move completed: %f, %f", state.player.position.x, state.player.position.y);
    }
    float t = 1.0f - (state.moveTimer / state.moveDuration);
    state.player.position = Vector2Lerp(state.moveStart, state.moveTarget, t);
    //TraceLog(LOG_INFO, "player.position: %f,%f t: %f  moveTarget: %f,%f", player.position.x, player.position.y, t, moveTarget.x, moveTarget.y);
    // uhm bob, bob bob headbob
    state.player.headOffset = headBobAmplitude * sinf(t * 2.0f * PI);
}

void StartTurn(DungeonState& state, int direction) {
    if (!state.isTurning) {
        state.isTurning = true;
        state.turnAngle = 0.0f;
        state.targetAngle = 90.0f;
        state.turnDirection = direction; // 1 = right (clockwise), -1 = left (counterclockwise)
    }
}

void UpdateTurn(DungeonState& state, float deltaTime) {
    if (!state.isTurning) return;

    // Get direction vector from camera position to target
    Vector3 forward = Vector3Subtract(state.camera.target, state.camera.position);

    // Convert current forward vector to angle
    float angleRad = atan2f(forward.z, forward.x);

    // Compute rotation step (frame-rate independent)
    float deltaAngle = (float) state.turnDirection * state.turnSpeed * deltaTime * DEG2RAD;
    angleRad += deltaAngle;
    state.turnAngle += state.turnSpeed * deltaTime;

    // Compute new target position (rotated around Y axis)
    float distance = sqrtf(forward.x * forward.x + forward.z * forward.z);
    state.camera.target.x = state.camera.position.x + cosf(angleRad) * distance;
    state.camera.target.z = state.camera.position.z + sinf(angleRad) * distance;

    // Stop when we reach 90 degrees
    if (state.turnAngle >= state.targetAngle) {
        state.isTurning = false;
        state.turnAngle = state.targetAngle;  // Ensure exact final angle
        TraceLog(LOG_INFO, "Turn completed. Target angle: %f", state.targetAngle);
    }
}

static Color GetVertexColor(Vector3 cameraPos, Vector3 position, Vector3 normal) {
    float distance = Vector3Distance(cameraPos, position);

    // Constants for light attenuation
    float k_d = 0.2f;
    float k_q = 0.05f;

    //float k_d = 0.1f;  // Linear factor (adjust for smoother/darker falloff)
    //float k_q = 0.02f; // Quadratic factor (higher = stronger falloff)

    //float k_d = 0.05f;
    //float k_q = 0.01f;
    float shade = 1.0f / (1.0f + k_d * distance + k_q * (distance * distance));

    // Clamp to a minimum brightness
    if (shade < 0.0f) shade = 0.0f;

    Color wallColor = {
            (unsigned char)(shade * 255),
            (unsigned char)(shade * 255),
            (unsigned char)(shade * 255),
            255
    };
    float intensity = shade;
    wallColor = ApplyTorchColor(intensity);

    return wallColor;
}

static inline void SetGlColor(Color color) {
    rlColor4ub(color.r, color.g, color.b, color.a);
}

Color ColorBlend(Color c1, Color c2) {
    return c1;
    return (Color){
            (unsigned char)fmax(c1.r, c2.r),
            (unsigned char)fmax(c1.g, c2.g),
            (unsigned char)fmax(c1.b, c2.b),
            (unsigned char)fmax(c1.a, c2.a)
    };
}

static void DrawWallFaces(DungeonState& state, Vector3 position, float width, float height, float length, CubeFaces faces) {
    float x = position.x;
    float y = position.y;
    float z = position.z;

    Color tl;
    Color tr;
    Color bl;
    Color br;

    int mapX = (int)(x / CUBE_SIZE);
    int mapY = (int)(z / CUBE_SIZE);

    // Set desired texture to be enabled while drawing following vertex data
    //rlSetTexture(texture.id);
    rlBegin(RL_QUADS);
    Color wallColor;
    //rlColor4ub(wallColor.r, wallColor.g, wallColor.b, wallColor.a);

    SetLightColorLeft_FB(state.lighting, mapX, mapY, tl);
    SetLightColorRight_FB(state.lighting, mapX, mapY, tr);
    SetLightColorLeft_FB(state.lighting, mapX, mapY, bl);
    SetLightColorRight_FB(state.lighting, mapX, mapY, br);

    if (faces.front != TILE_NONE) {
        rlSetTexture(state.textures[faces.front].id);
        // Front Face
        rlNormal3f(0.0f, 0.0f, -1.0f);       // Normal Pointing Towards Viewer

        rlTexCoord2f(0.0f, 0.0f);
        wallColor = GetVertexColor(state.camera.position, {x - width / 2, y + height / 2, z + length / 2}, {0, 0, -1});
        SetGlColor(ColorBlend(tl, wallColor));
        rlVertex3f(x - width / 2, y + height / 2, z + length / 2);  // Top Left Of The Texture and Quad
        rlTexCoord2f(1.0f, 0.0f);
        wallColor = GetVertexColor(state.camera.position, {x + width / 2, y + height / 2, z + length / 2}, {0, 0, -1});
        SetGlColor(ColorBlend(tr, wallColor));
        rlVertex3f(x + width / 2, y + height / 2, z + length / 2);  // Top Right Of The Texture and Quad
        rlTexCoord2f(1.0f, 1.0f);
        wallColor = GetVertexColor(state.camera.position, {x + width / 2, y - height / 2, z + length / 2}, {0, 0, -1});
        SetGlColor(ColorBlend(br, wallColor));
        rlVertex3f(x + width / 2, y - height / 2, z + length / 2);  // Bottom Right Of The Texture and Quad
        rlTexCoord2f(0.0f, 1.0f);
        wallColor = GetVertexColor(state.camera.position, {x - width / 2, y - height / 2, z + length / 2}, {0, 0, -1});
        SetGlColor(ColorBlend(bl, wallColor));
        rlVertex3f(x - width / 2, y - height / 2, z + length / 2);  // Bottom Left Of The Texture and Quad
    }

    if (faces.back != TILE_NONE) {
        rlSetTexture(state.textures[faces.back].id);
        // Back Face
        rlNormal3f(0.0f, 0.0f, 1.0f);      // Normal Pointing Away From Viewer

        rlTexCoord2f(0.0f, 1.0f);
        wallColor = GetVertexColor(state.camera.position, {x + width / 2, y - height / 2, z - length / 2}, {0, 0, 1});
        SetGlColor(ColorBlend(br, wallColor));
        rlVertex3f(x + width / 2, y - height / 2, z - length / 2);  // Bottom Left Of The Texture and Quad
        rlTexCoord2f(0.0f, 0.0f);
        wallColor = GetVertexColor(state.camera.position, {x + width / 2, y + height / 2, z - length / 2}, {0, 0, 1});
        SetGlColor(ColorBlend(tr, wallColor));
        rlVertex3f(x + width / 2, y + height / 2, z - length / 2);  // Top Left Of The Texture and Quad
        rlTexCoord2f(1.0f, 0.0f);
        wallColor = GetVertexColor(state.camera.position, {x - width / 2, y + height / 2, z - length / 2}, {0, 0, 1});
        SetGlColor(ColorBlend(tl, wallColor));
        rlVertex3f(x - width / 2, y + height / 2, z - length / 2);  // Top Right Of The Texture and Quad
        rlTexCoord2f(1.0f, 1.0f);
        wallColor = GetVertexColor(state.camera.position, {x - width / 2, y - height / 2, z - length / 2}, {0, 0, 1});
        SetGlColor(ColorBlend(bl, wallColor));
        rlVertex3f(x - width / 2, y - height / 2, z - length / 2);  // Bottom Right Of The Texture and Quad
    }

    SetLightColorTopLeft_TB(state.lighting, mapX, mapY, tl);
    SetLightColorTopRight_TB(state.lighting, mapX, mapY, tr);
    SetLightColorBottomLeft_TB(state.lighting, mapX, mapY, bl);
    SetLightColorBottomRight_TB(state.lighting, mapX, mapY, br);

    if (faces.top != TILE_NONE) {
        rlSetTexture(state.textures[faces.top].id);
        // Top Face
        rlNormal3f(0.0f, -1.0f, 0.0f);       // Normal Pointing Up

        rlTexCoord2f(1.0f, 0.0f);
        wallColor = GetVertexColor(state.camera.position, {x + width / 2, y + height / 2, z - length / 2}, {0, -1, 0});
        SetGlColor(ColorBlend(tr, wallColor));
        rlVertex3f(x + width / 2, y + height / 2, z - length / 2);  // Top Right Of The Texture and Quad
        rlTexCoord2f(1.0f, 1.0f);
        wallColor = GetVertexColor(state.camera.position, {x + width / 2, y + height / 2, z + length / 2}, {0, -1, 0});
        SetGlColor(ColorBlend(br, wallColor));
        rlVertex3f(x + width / 2, y + height / 2, z + length / 2);  // Bottom Right Of The Texture and Quad
        rlTexCoord2f(0.0f, 1.0f);
        wallColor = GetVertexColor(state.camera.position, {x - width / 2, y + height / 2, z + length / 2}, {0, -1, 0});
        SetGlColor(ColorBlend(bl, wallColor));
        rlVertex3f(x - width / 2, y + height / 2, z + length / 2);  // Bottom Left Of The Texture and Quad
        rlTexCoord2f(0.0f, 0.0f);
        wallColor = GetVertexColor(state.camera.position, {x - width / 2, y + height / 2, z - length / 2}, {0, -1, 0});
        SetGlColor(ColorBlend(tl, wallColor));
        rlVertex3f(x - width / 2, y + height / 2, z - length / 2);  // Top Left Of The Texture and Quad
    }

    if (faces.bottom != TILE_NONE) {
        rlSetTexture(state.textures[faces.bottom].id);
        // Bottom Face
        rlNormal3f(0.0f, 1.0f, 0.0f);      // Normal Pointing Down

        rlTexCoord2f(1.0f, 1.0f);
        wallColor = GetVertexColor(state.camera.position, {x - width / 2, y - height / 2, z + length / 2}, {0, 1, 0});
        SetGlColor(ColorBlend(bl, wallColor));
        rlVertex3f(x - width / 2, y - height / 2, z + length / 2);  // Bottom Right Of The Texture and Quad
        rlTexCoord2f(0.0f, 1.0f);
        wallColor = GetVertexColor(state.camera.position, {x + width / 2, y - height / 2, z + length / 2}, {0, 1, 0});
        SetGlColor(ColorBlend(br, wallColor));
        rlVertex3f(x + width / 2, y - height / 2, z + length / 2);  // Bottom Left Of The Texture and Quad
        rlTexCoord2f(0.0f, 0.0f);
        wallColor = GetVertexColor(state.camera.position, {x + width / 2, y - height / 2, z - length / 2}, {0, 1, 0});
        SetGlColor(ColorBlend(tr, wallColor));
        rlVertex3f(x + width / 2, y - height / 2, z - length / 2);  // Top Left Of The Texture and Quad
        rlTexCoord2f(1.0f, 0.0f);
        wallColor = GetVertexColor(state.camera.position, {x - width / 2, y - height / 2, z - length / 2}, {0, 1, 0});
        SetGlColor(ColorBlend(tl, wallColor));
        rlVertex3f(x - width / 2, y - height / 2, z - length / 2);  // Top Right Of The Texture and Quad
    }

    SetLightColorLeft_LR(state.lighting, mapX, mapY, tl);
    SetLightColorRight_LR(state.lighting, mapX, mapY, tr);
    SetLightColorLeft_LR(state.lighting, mapX, mapY, bl);
    SetLightColorRight_LR(state.lighting, mapX, mapY, br);

    if (faces.right != TILE_NONE) {
        rlSetTexture(state.textures[faces.right].id);
        // Right Face
        rlNormal3f(-1.0f, 0.0f, 0.0f);       // Normal Pointing Right

        rlTexCoord2f(1.0f, 1.0f);
        wallColor = GetVertexColor(state.camera.position, {x + width / 2, y - height / 2, z + length / 2}, {-1, 0, 0});
        SetGlColor(ColorBlend(bl, wallColor));
        rlVertex3f(x + width / 2, y - height / 2, z + length / 2);  // Bottom Left Of The Texture and Quad
        rlTexCoord2f(1.0f, 0.0f);
        wallColor = GetVertexColor(state.camera.position, {x + width / 2, y + height / 2, z + length / 2}, {-1, 0, 0});
        SetGlColor(ColorBlend(tl, wallColor));
        rlVertex3f(x + width / 2, y + height / 2, z + length / 2);  // Top Left Of The Texture and Quad
        rlTexCoord2f(0.0f, 0.0f);
        wallColor = GetVertexColor(state.camera.position, {x + width / 2, y + height / 2, z - length / 2}, {-1, 0, 0});
        SetGlColor(ColorBlend(tr, wallColor));
        rlVertex3f(x + width / 2, y + height / 2, z - length / 2);  // Top Right Of The Texture and Quad
        rlTexCoord2f(0.0f, 1.0f);
        wallColor = GetVertexColor(state.camera.position, {x + width / 2, y - height / 2, z - length / 2}, {-1, 0, 0});
        SetGlColor(ColorBlend(br, wallColor));
        rlVertex3f(x + width / 2, y - height / 2, z - length / 2);  // Bottom Right Of The Texture and Quad

    }

    if (faces.left != TILE_NONE) {
        rlSetTexture(state.textures[faces.left].id);
        // Left Face
        rlNormal3f(1.0f, 0.0f, 0.0f);      // Normal Pointing Left

        rlTexCoord2f(1.0f, 0.0f);
        wallColor = GetVertexColor(state.camera.position, {x - width / 2, y + height / 2, z - length / 2}, {1, 0, 0});
        SetGlColor(ColorBlend(tr, wallColor));
        rlVertex3f(x - width / 2, y + height / 2, z - length / 2);  // Top Left Of The Texture and Quad
        rlTexCoord2f(0.0f, 0.0f);
        wallColor = GetVertexColor(state.camera.position, {x - width / 2, y + height / 2, z + length / 2}, {1, 0, 0});
        SetGlColor(ColorBlend(tl, wallColor));
        rlVertex3f(x - width / 2, y + height / 2, z + length / 2);  // Top Right Of The Texture and Quad
        rlTexCoord2f(0.0f, 1.0f);
        wallColor = GetVertexColor(state.camera.position, {x - width / 2, y - height / 2, z + length / 2}, {1, 0, 0});
        SetGlColor(ColorBlend(bl, wallColor));
        rlVertex3f(x - width / 2, y - height / 2, z + length / 2);  // Bottom Right Of The Texture and Quad
        rlTexCoord2f(1.0f, 1.0f);
        wallColor = GetVertexColor(state.camera.position, {x - width / 2, y - height / 2, z - length / 2}, {1, 0, 0});
        SetGlColor(ColorBlend(br, wallColor));
        rlVertex3f(x - width / 2, y - height / 2, z - length / 2);  // Bottom Left Of The Texture and Quad
    }
    rlEnd();
    rlSetTexture(0);
}

// Use rect to convert texture coordinates
static void EmitTexCoord(Texture2D& texture, Rectangle& rect, float x, float y) {
    // Convert normalized coordinates (0 to 1) into the sub-region
    float u = rect.x / texture.width + (x * (rect.width / texture.width));
    float v = rect.y / texture.height + (y * (rect.height / texture.height));

    // Emit final coordinates
    rlTexCoord2f(u, v);
}

static void DrawThinWallLR(DungeonState& state, Vector3 position, float width, float height, float length, Texture2D& texture, Rectangle& source) {
    float x = position.x;
    float y = position.y;
    float z = position.z;

    int mapX = (int)(x / CUBE_SIZE);
    int mapY = (int)(z / CUBE_SIZE);

    Color lightColor = WHITE;
    GetLightColorAt(state.lighting, mapX, mapY, lightColor);

    rlBegin(RL_QUADS);
    Color wallColor;

    rlSetTexture(texture.id);

    // Right Face
    rlNormal3f(-1.0f, 0.0f, 0.0f);       // Normal Pointing Right
    EmitTexCoord(texture, source, 1.0f, 1.0f);
    wallColor = GetVertexColor(state.camera.position, {x + width / 2, y - height / 2, z + length / 2}, {-1, 0, 0});
    SetGlColor(ColorBlend(lightColor, wallColor));
    rlVertex3f(x + width / 2, y - height / 2, z + length / 2);  // Bottom Left Of The Texture and Quad
    EmitTexCoord(texture, source, 1.0f, 0.0f);
    wallColor = GetVertexColor(state.camera.position, {x + width / 2, y + height / 2, z + length / 2}, {-1, 0, 0});
    SetGlColor(ColorBlend(lightColor, wallColor));
    rlVertex3f(x + width / 2, y + height / 2, z + length / 2);  // Top Left Of The Texture and Quad
    EmitTexCoord(texture, source, 0.0f, 0.0f);
    wallColor = GetVertexColor(state.camera.position, {x + width / 2, y + height / 2, z - length / 2}, {-1, 0, 0});
    SetGlColor(ColorBlend(lightColor, wallColor));
    rlVertex3f(x + width / 2, y + height / 2, z - length / 2);  // Top Right Of The Texture and Quad
    EmitTexCoord(texture, source, 0.0f, 1.0f);
    wallColor = GetVertexColor(state.camera.position, {x + width / 2, y - height / 2, z - length / 2}, {-1, 0, 0});
    SetGlColor(ColorBlend(lightColor, wallColor));
    rlVertex3f(x + width / 2, y - height / 2, z - length / 2);  // Bottom Right Of The Texture and Quad

    // Left Face
    rlNormal3f(1.0f, 0.0f, 0.0f);      // Normal Pointing Left
    EmitTexCoord(texture, source, 1.0f, 0.0f);
    wallColor = GetVertexColor(state.camera.position, {x - width / 2, y + height / 2, z - length / 2}, {1, 0, 0});
    SetGlColor(ColorBlend(lightColor, wallColor));
    rlVertex3f(x - width / 2, y + height / 2, z - length / 2);  // Top Left Of The Texture and Quad
    EmitTexCoord(texture, source, 0.0f, 0.0f);
    wallColor = GetVertexColor(state.camera.position, {x - width / 2, y + height / 2, z + length / 2}, {1, 0, 0});
    SetGlColor(ColorBlend(lightColor, wallColor));
    rlVertex3f(x - width / 2, y + height / 2, z + length / 2);  // Top Right Of The Texture and Quad
    EmitTexCoord(texture, source, 0.0f, 1.0f);
    wallColor = GetVertexColor(state.camera.position, {x - width / 2, y - height / 2, z + length / 2}, {1, 0, 0});
    SetGlColor(ColorBlend(lightColor, wallColor));
    rlVertex3f(x - width / 2, y - height / 2, z + length / 2);  // Bottom Right Of The Texture and Quad
    EmitTexCoord(texture, source, 1.0f, 1.0f);
    wallColor = GetVertexColor(state.camera.position, {x - width / 2, y - height / 2, z - length / 2}, {1, 0, 0});
    SetGlColor(ColorBlend(lightColor, wallColor));
    rlVertex3f(x - width / 2, y - height / 2, z - length / 2);  // Bottom Left Of The Texture and Quad

    rlEnd();
    rlSetTexture(0);
}

static void DrawThinWallFB(DungeonState& state, Vector3 position, float width, float height, float length, Texture2D& texture, Rectangle& source) {
    float x = position.x;
    float y = position.y;
    float z = position.z;

    int mapX = (int)(x / CUBE_SIZE);
    int mapY = (int)(z / CUBE_SIZE);

    Color lightColor = WHITE;
    GetLightColorAt(state.lighting, mapX, mapY, lightColor);

    rlBegin(RL_QUADS);
    Color wallColor;

    rlSetTexture(texture.id);
    // Front Face
    rlNormal3f(0.0f, 0.0f, -1.0f);       // Normal Pointing Towards Viewer
    EmitTexCoord(texture, source, 0.0f, 0.0f);
    wallColor = GetVertexColor(state.camera.position, {x - width / 2, y + height / 2, z + length / 2}, {0, 0, -1});
    SetGlColor(ColorBlend(lightColor, wallColor));
    rlVertex3f(x - width / 2, y + height / 2, z + length / 2);  // Top Left Of The Texture and Quad
    EmitTexCoord(texture, source, 1.0f, 0.0f);
    wallColor = GetVertexColor(state.camera.position, {x + width / 2, y + height / 2, z + length / 2}, {0, 0, -1});
    SetGlColor(ColorBlend(lightColor, wallColor));
    rlVertex3f(x + width / 2, y + height / 2, z + length / 2);  // Top Right Of The Texture and Quad
    EmitTexCoord(texture, source, 1.0f, 1.0f);
    wallColor = GetVertexColor(state.camera.position, {x + width / 2, y - height / 2, z + length / 2}, {0, 0, -1});
    SetGlColor(ColorBlend(lightColor, wallColor));
    rlVertex3f(x + width / 2, y - height / 2, z + length / 2);  // Bottom Right Of The Texture and Quad
    EmitTexCoord(texture, source, 0.0f, 1.0f);
    wallColor = GetVertexColor(state.camera.position, {x - width / 2, y - height / 2, z + length / 2}, {0, 0, -1});
    SetGlColor(ColorBlend(lightColor, wallColor));
    rlVertex3f(x - width / 2, y - height / 2, z + length / 2);  // Bottom Left Of The Texture and Quad

    // Back Face
    rlNormal3f(0.0f, 0.0f, 1.0f);      // Normal Pointing Away From Viewer
    EmitTexCoord(texture, source, 0.0f, 1.0f);
    wallColor = GetVertexColor(state.camera.position, {x + width / 2, y - height / 2, z - length / 2}, {0, 0, 1});
    SetGlColor(ColorBlend(lightColor, wallColor));
    rlVertex3f(x + width / 2, y - height / 2, z - length / 2);  // Bottom Left Of The Texture and Quad
    EmitTexCoord(texture, source, 0.0f, 0.0f);
    wallColor = GetVertexColor(state.camera.position, {x + width / 2, y + height / 2, z - length / 2}, {0, 0, 1});
    SetGlColor(ColorBlend(lightColor, wallColor));
    rlVertex3f(x + width / 2, y + height / 2, z - length / 2);  // Top Left Of The Texture and Quad
    EmitTexCoord(texture, source, 1.0f, 0.0f);
    wallColor = GetVertexColor(state.camera.position, {x - width / 2, y + height / 2, z - length / 2}, {0, 0, 1});
    SetGlColor(ColorBlend(lightColor, wallColor));
    rlVertex3f(x - width / 2, y + height / 2, z - length / 2);  // Top Right Of The Texture and Quad
    EmitTexCoord(texture, source, 1.0f, 1.0f);
    wallColor = GetVertexColor(state.camera.position, {x - width / 2, y - height / 2, z - length / 2}, {0, 0, 1});
    SetGlColor(ColorBlend(lightColor, wallColor));
    rlVertex3f(x - width / 2, y - height / 2, z - length / 2);  // Bottom Right Of The Texture and Quad

    rlEnd();
    rlSetTexture(0);
}

static void DrawThinWallTB(DungeonState& state, Vector3 position, float width, float height, float length, Texture2D& texture, Rectangle& source) {
    float x = position.x;
    float y = position.y;
    float z = position.z;

    int mapX = (int)(x / CUBE_SIZE);
    int mapY = (int)(z / CUBE_SIZE);

    Color lightColor = WHITE;
    GetLightColorAt(state.lighting, mapX, mapY, lightColor);

    rlBegin(RL_QUADS);
    Color wallColor;

    rlSetTexture(texture.id);

    // Top Face
    rlNormal3f(0.0f, -1.0f, 0.0f);       // Normal Pointing Up
    EmitTexCoord(texture, source, 1.0f, 0.0f);
    wallColor = GetVertexColor(state.camera.position, {x + width / 2, y + height / 2, z - length / 2}, {0, -1, 0});
    SetGlColor(ColorBlend(lightColor, wallColor));
    rlVertex3f(x + width / 2, y + height / 2, z - length / 2);  // Top Right Of The Texture and Quad
    EmitTexCoord(texture, source, 1.0f, 1.0f);
    wallColor = GetVertexColor(state.camera.position, {x + width / 2, y + height / 2, z + length / 2}, {0, -1, 0});
    SetGlColor(ColorBlend(lightColor, wallColor));
    rlVertex3f(x + width / 2, y + height / 2, z + length / 2);  // Bottom Right Of The Texture and Quad
    EmitTexCoord(texture, source, 0.0f, 1.0f);
    wallColor = GetVertexColor(state.camera.position, {x - width / 2, y + height / 2, z + length / 2}, {0, -1, 0});
    SetGlColor(ColorBlend(lightColor, wallColor));
    rlVertex3f(x - width / 2, y + height / 2, z + length / 2);  // Bottom Left Of The Texture and Quad
    EmitTexCoord(texture, source, 0.0f, 0.0f);
    wallColor = GetVertexColor(state.camera.position, {x - width / 2, y + height / 2, z - length / 2}, {0, -1, 0});
    SetGlColor(ColorBlend(lightColor, wallColor));
    rlVertex3f(x - width / 2, y + height / 2, z - length / 2);  // Top Left Of The Texture and Quad


    // Bottom Face
    rlNormal3f(0.0f, 1.0f, 0.0f);      // Normal Pointing Down
    EmitTexCoord(texture, source, 1.0f, 1.0f);
    wallColor = GetVertexColor(state.camera.position, {x - width / 2, y - height / 2, z + length / 2}, {0, 1, 0});
    SetGlColor(ColorBlend(lightColor, wallColor));
    rlVertex3f(x - width / 2, y - height / 2, z + length / 2);  // Bottom Right Of The Texture and Quad
    EmitTexCoord(texture, source, 0.0f, 1.0f);
    wallColor = GetVertexColor(state.camera.position, {x + width / 2, y - height / 2, z + length / 2}, {0, 1, 0});
    SetGlColor(ColorBlend(lightColor, wallColor));
    rlVertex3f(x + width / 2, y - height / 2, z + length / 2);  // Bottom Left Of The Texture and Quad
    EmitTexCoord(texture, source, 0.0f, 0.0f);
    wallColor = GetVertexColor(state.camera.position, {x + width / 2, y - height / 2, z - length / 2}, {0, 1, 0});
    SetGlColor(ColorBlend(lightColor, wallColor));
    rlVertex3f(x + width / 2, y - height / 2, z - length / 2);  // Top Left Of The Texture and Quad
    EmitTexCoord(texture, source, 1.0f, 0.0f);
    wallColor = GetVertexColor(state.camera.position, {x - width / 2, y - height / 2, z - length / 2}, {0, 1, 0});
    SetGlColor(ColorBlend(lightColor, wallColor));
    rlVertex3f(x - width / 2, y - height / 2, z - length / 2);  // Top Right Of The Texture and Quad

    rlEnd();
    rlSetTexture(0);
}


void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color)
{
    float x = position.x;
    float y = position.y;
    float z = position.z;

    // Set desired texture to be enabled while drawing following vertex data
    rlSetTexture(texture.id);

    rlBegin(RL_QUADS);
    rlColor4ub(color.r, color.g, color.b, color.a);

    // Front Face
    rlNormal3f(0.0f, 0.0f, 1.0f);       // Normal Pointing Towards Viewer
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Right Of The Texture and Quad
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left Of The Texture and Quad

    // Back Face
    rlNormal3f(0.0f, 0.0f, -1.0f);      // Normal Pointing Away From Viewer
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Right Of The Texture and Quad
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Left Of The Texture and Quad

    // Top Face
    rlNormal3f(0.0f, 1.0f, 0.0f);       // Normal Pointing Up
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Bottom Left Of The Texture and Quad
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Bottom Right Of The Texture and Quad
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad

    // Bottom Face
    rlNormal3f(0.0f, -1.0f, 0.0f);      // Normal Pointing Down
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Top Right Of The Texture and Quad
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Top Left Of The Texture and Quad
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad

    // Right Face
    rlNormal3f(1.0f, 0.0f, 0.0f);       // Normal Pointing Right
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right Of The Texture and Quad
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Left Of The Texture and Quad
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad

    // Left Face
    rlNormal3f(-1.0f, 0.0f, 0.0f);      // Normal Pointing Left
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Left Of The Texture and Quad
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Right Of The Texture and Quad
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad

    rlEnd();

    rlSetTexture(0);
}

CellSide GetFacingSide(Camera3D camera, Vector2 mobDir) {
    // Compute camera's forward vector in 2D (XZ plane)
    Vector3 camFront = Vector3Subtract(camera.target, camera.position);
    Vector2 camForward = { -camFront.x, -camFront.z };

    // Compute cosine of the angle between camera forward and mob direction
    float cosAngle = Vector2DotProduct(camForward, mobDir);

    if (cosAngle > 0.707f) {  // Facing mostly north
        return CellSide::NORTH;
    } else if (cosAngle < -0.707f) {  // Facing mostly south
        return CellSide::SOUTH;
    }

    // Compute cross product to determine left/right
    float cross = camForward.x * mobDir.y - camForward.y * mobDir.x;

    if (cross < 0.0f) {
        return CellSide::WEST;
    } else {
        return CellSide::EAST;
    }
}

void DrawFacing(Camera3D camera, Vector2 normal, Texture2D texture, Vector3 position, Vector2 billboardSize, Color lightColor) {
    CellSide side = GetFacingSide(camera, normal);
    if(side == CellSide::NORTH || side == CellSide::SOUTH) {
        DrawBillboardRec(camera, texture, (Rectangle){0, 0, (float) texture.width/2, (float) texture.height}, position, billboardSize, lightColor);
    } else {
        if(side == CellSide::WEST)
            DrawBillboardRec(camera, texture, (Rectangle){(float) texture.width/2, 0, (float) texture.width/2, (float) texture.height}, position, billboardSize, lightColor);
        else
            DrawBillboardRec(camera, texture, (Rectangle){0, 0, (float) -texture.width/2, (float) texture.height}, position, billboardSize, lightColor);
    }
}

void RenderObject(DungeonState& state, int mapX, int mapY, int tileType) {
    //DrawSphere((Vector3){CUBE_SIZE * (float) mapX, CUBE_SIZE/2, CUBE_SIZE * (float) mapY}, 0.15f, RED);
    //DrawBillboard(state.camera, state.textures[tileType], (Vector3){CUBE_SIZE * (float) mapX, CUBE_SIZE/2, CUBE_SIZE * (float) mapY}, 0.5f, WHITE);
    Color lightColor;
    GetLightColorAt(state.lighting, mapX, mapY, lightColor);
    if(tileType == TILE_PILLAR) {
        Vector3 position = {CUBE_SIZE * (float) mapX, CUBE_SIZE/2, CUBE_SIZE * (float) mapY};
        Vector2 billboardSize = {1.0f, 3.0f};  // 1x2 world units
        Texture2D& texture = state.textures[tileType];
        DrawBillboardRec(state.camera, texture, (Rectangle){0, 0, (float) texture.width, (float) texture.height}, position, billboardSize, lightColor);
    }
    if(tileType == TILE_WALL_TORCH_NORTH) {
        Vector3 position = {CUBE_SIZE * (float) mapX, CUBE_SIZE/2, CUBE_SIZE * (float) mapY};
        position.z -= CUBE_SIZE/2;
        position.z += 0.10f;
        Vector2 billboardSize = {1.0f, 1.0f};
        Texture2D& texture = state.textures[tileType];
        DrawFacing(state.camera, {0, -1}, texture, position, billboardSize, lightColor);
    }
    if(tileType == TILE_WALL_TORCH_EAST) {
        Vector3 position = {CUBE_SIZE * (float) mapX, CUBE_SIZE/2, CUBE_SIZE * (float) mapY};
        position.x += CUBE_SIZE/2;
        position.x -= 0.10f;
        Vector2 billboardSize = {1.0f, 1.0f};
        Texture2D& texture = state.textures[tileType];
        DrawFacing(state.camera, {1, 0}, texture, position, billboardSize, lightColor);
    }
    if(tileType == TILE_WALL_TORCH_SOUTH) {
        Vector3 position = {CUBE_SIZE * (float) mapX, CUBE_SIZE/2, CUBE_SIZE * (float) mapY};
        position.z += CUBE_SIZE/2;
        position.z -= 0.10f;
        Vector2 billboardSize = {1.0f, 1.0f};
        Texture2D& texture = state.textures[tileType];
        DrawFacing(state.camera, {0, 1}, texture, position, billboardSize, lightColor);
    }
    if(tileType == TILE_WALL_TORCH_WEST) {
        Vector3 position = {CUBE_SIZE * (float) mapX, CUBE_SIZE/2, CUBE_SIZE * (float) mapY};
        position.x -= CUBE_SIZE/2;
        position.x += 0.10f;
        Vector2 billboardSize = {1.0f, 1.0f};
        Texture2D& texture = state.textures[tileType];
        DrawFacing(state.camera, {-1, 0}, texture, position, billboardSize, lightColor);
    }

}

static void RenderWallAlignedObjects(DungeonState& state, int mapX, int mapY) {
    for(WallAlignedObject& obj : state.wallObjects) {
        if(obj.mapPosition.x == mapX && obj.mapPosition.y == mapY) {
            Color lightColor = WHITE;
            GetLightColorAt(state.lighting, mapX, mapY, lightColor);
            CellSide side = GetFacingSide(state.camera, obj.normal);
            if(side == CellSide::NORTH || side == CellSide::SOUTH) {
                FrameInfo frame = GetFrameInfo(obj.frontPlayer);
                DrawBillboardRec(state.camera, *frame.texture, frame.srcRect, obj.position, obj.billboardSize, lightColor);
            } else {
                FrameInfo frame = GetFrameInfo(obj.sidePlayer);
                if(side == CellSide::WEST) {
                    DrawBillboardRec(state.camera, *frame.texture, frame.srcRect, obj.position, obj.billboardSize,
                                     lightColor);
                }
                else {
                    frame.srcRect.width = -frame.srcRect.width;
                    DrawBillboardRec(state.camera, *frame.texture, frame.srcRect, obj.position, obj.billboardSize,
                                     lightColor);
                }
            }
        }
    }
}

static void RenderThinWalls(DungeonState& state, int mapX, int mapY) {
    for(ThinWall& tw : state.thinWalls) {
        if(tw.mapPosition.x == mapX && tw.mapPosition.y == mapY) {
            Color lightColor = WHITE;
            GetLightColorAt(state.lighting, mapX, mapY, lightColor);
            //Vector3 position = {CUBE_SIZE * (float) mapX, CUBE_SIZE/2, CUBE_SIZE * (float) mapY};
            FrameInfo frame = GetFrameInfo(tw.player);
            switch(tw.orientation) {
                case AxisOrientation::X:
                    DrawThinWallLR(state, tw.position, 0.02f, CUBE_SIZE, CUBE_SIZE, *frame.texture, frame.srcRect);
                    break;
                case AxisOrientation::Y:
                    DrawThinWallTB(state, tw.position, CUBE_SIZE, 0.02f, CUBE_SIZE, *frame.texture, frame.srcRect);
                    break;
                case AxisOrientation::Z:
                    DrawThinWallFB(state, tw.position, CUBE_SIZE, CUBE_SIZE, 0.02f, *frame.texture, frame.srcRect);
                    break;
            }
        }
    }
}

void RenderMap(DungeonState& state) {
    std::vector<Vector2i> freePositions;
    int radius = 16;
    int px = int (state.player.position.x / CUBE_SIZE);
    int py = int (state.player.position.y / CUBE_SIZE);

    // Iterate over the bounding box of the circle
    for (int offsetX = -radius; offsetX <= radius; ++offsetX) {
        for (int offsetY = -radius; offsetY <= radius; ++offsetY) {
            int checkX = px + offsetX;
            int checkY = py + offsetY;

            // Check if the position is within the playfield bounds
            if (checkX < 0 || checkX >= state.map->width || checkY < 0 || checkY >= state.map->height) continue;

            // Check if the position is within the circle's radius
            if (offsetX * offsetX + offsetY * offsetY <= radius * radius) {
                // Add the position if it is not solid
                if(GetDungeonTile(*state.map, NAV_LAYER, px, py) != 2) {
                    freePositions.emplace_back(checkX, checkY);
                }
            }
        }
    }

    // Sort the free positions by their distance from the center
    std::sort(freePositions.begin(), freePositions.end(), [px, py](const Vector2i &a, const Vector2i &b) {
        int distA = (a.x - px) * (a.x - px) + (a.y - py) * (a.y - py);
        int distB = (b.x - px) * (b.x - px) + (b.y - py) * (b.y - py);
        return distA > distB;
    });

    //TraceLog(LOG_INFO, "Free positions: %d", freePositions.size());
    // Draw the free positions
    for (const auto &p : freePositions) {
        //TraceLog(LOG_INFO, "Free position: %d, %d", p.x, p.y);
        int currentNav = GetDungeonTile(*state.map, NAV_LAYER, p.x, p.y);
        if(currentNav == 2) continue;

        // Determine neighboring cells
        int leftCell = GetDungeonTile(*state.map, NAV_LAYER, p.x - 1, p.y);
        int rightCell = GetDungeonTile(*state.map, NAV_LAYER, p.x + 1, p.y);
        int frontCell = GetDungeonTile(*state.map, NAV_LAYER, p.x, p.y - 1);
        int backCell = GetDungeonTile(*state.map, NAV_LAYER, p.x, p.y + 1);
        //int aboveCell = (y > 0) ? map[(y - 1) * mapWidth + x] : 1;
        //int belowCell = (y < mapHeight - 1) ? map[(y + 1) * mapWidth + x] : 1;

        // Determine wall conditions
        CubeFaces faces = {};
        faces.top = TILE_CEILING;
        faces.bottom = TILE_FLOOR;
        //faces.top = TILE_NONE;
        //faces.bottom = TILE_NONE;

        faces.left = leftCell > 0 ? GetDungeonTile(*state.map, WALLS_LAYER, p.x-1, p.y) : TILE_NONE;
        faces.right = rightCell > 0 ? GetDungeonTile(*state.map, WALLS_LAYER, p.x+1, p.y) : TILE_NONE;
        // swap because of inverted z axis used as y axis
        faces.front = backCell > 0 ? GetDungeonTile(*state.map, WALLS_LAYER, p.x, p.y+1) : TILE_NONE;
        faces.back = frontCell > 0 ? GetDungeonTile(*state.map, WALLS_LAYER, p.x, p.y-1) : TILE_NONE;


        DrawWallFaces(state, {CUBE_SIZE * (float) p.x, CUBE_SIZE/2, CUBE_SIZE * (float) p.y}, CUBE_SIZE, CUBE_SIZE, CUBE_SIZE, faces);
        rlDisableDepthTest();
        RenderThinWalls(state, p.x, p.y);
        rlEnableDepthTest();
        RenderWallAlignedObjects(state, p.x, p.y);
    }
}

void UpdateCamera(DungeonState& state) {
    state.camera.position = {state.player.position.x, 1.65f - state.player.headOffset, state.player.position.y};

    Vector3 forward = {0.0f, 0.0f, 0.0f};
    if (state.playerFacing == 0) forward = {0.0f, 0.0f, -1.0f}; // North
    else if (state.playerFacing == 1) forward = {1.0f, 0.0f, 0.0f}; // East
    else if (state.playerFacing == 2) forward = {0.0f, 0.0f, 1.0f}; // South
    else if (state.playerFacing == 3) forward = {-1.0f, 0.0f, 0.0f}; // West

    state.camera.target = (Vector3){state.camera.position.x + forward.x,
                                    state.camera.position.y + forward.y,
                                    state.camera.position.z + forward.z};
}

static void SetFrontAndSideAnim(WallAlignedObject& wallObj, const std::string& front, const std::string& side) {
    InitSpriteAnimationPlayer(wallObj.frontPlayer);
    PlaySpriteAnimation(wallObj.frontPlayer, GetSpriteAnimation(front), true);
    InitSpriteAnimationPlayer(wallObj.sidePlayer);
    PlaySpriteAnimation(wallObj.sidePlayer, GetSpriteAnimation(side), true);
}

void CreateObjects(DungeonState& state) {
    float heightOffset = 0.40f;
    for(int i = 0; i < state.map->width; i++) {
        for(int j = 0; j < state.map->height; j++) {
            int animType = GetDungeonTile(*state.map, OBJECTS_LAYER, i, j);
            if(animType == TILE_WALL_TORCH_NORTH) {
                WallAlignedObject wallObj;
                wallObj.mapPosition = {i, j};
                wallObj.position = {CUBE_SIZE * (float) i, (CUBE_SIZE/2) + heightOffset, CUBE_SIZE * (float) j};
                wallObj.position.z -= CUBE_SIZE/2;
                wallObj.position.z += 0.10f;
                wallObj.billboardSize = {1.0f, 1.0f};
                wallObj.normal = {0, -1};
                SetFrontAndSideAnim(wallObj, "WallTorchFrontBurn", "WallTorchSideBurn");
                state.wallObjects.push_back(wallObj);
            }
            if(animType == TILE_WALL_TORCH_EAST) {
                WallAlignedObject wallObj;
                wallObj.mapPosition = {i, j};
                wallObj.position = {CUBE_SIZE * (float) i, (CUBE_SIZE/2) + heightOffset, CUBE_SIZE * (float) j};
                wallObj.position.x += CUBE_SIZE/2;
                wallObj.position.x -= 0.10f;
                wallObj.billboardSize = {1.0f, 1.0f};
                wallObj.normal = {1, 0};
                SetFrontAndSideAnim(wallObj, "WallTorchFrontBurn", "WallTorchSideBurn");
                state.wallObjects.push_back(wallObj);
            }
            if(animType == TILE_WALL_TORCH_SOUTH) {
                WallAlignedObject wallObj;
                wallObj.mapPosition = {i, j};
                wallObj.position = {CUBE_SIZE * (float) i, (CUBE_SIZE/2) + heightOffset, CUBE_SIZE * (float) j};
                wallObj.position.z += CUBE_SIZE/2;
                wallObj.position.z -= 0.10f;
                wallObj.billboardSize = {1.0f, 1.0f};
                wallObj.normal = {0, 1};
                SetFrontAndSideAnim(wallObj, "WallTorchFrontBurn", "WallTorchSideBurn");
                state.wallObjects.push_back(wallObj);
            }
            if(animType == TILE_WALL_TORCH_WEST) {
                WallAlignedObject wallObj;
                wallObj.mapPosition = {i, j};
                wallObj.position = {CUBE_SIZE * (float) i, (CUBE_SIZE/2) + heightOffset, CUBE_SIZE * (float) j};
                wallObj.position.x -= CUBE_SIZE/2;
                wallObj.position.x += 0.10f;
                wallObj.billboardSize = {1.0f, 1.0f};
                wallObj.normal = {-1, 0};
                SetFrontAndSideAnim(wallObj, "WallTorchFrontBurn", "WallTorchSideBurn");
                state.wallObjects.push_back(wallObj);
            }
        }
    }
}

void CreateThinWalls(DungeonState& state) {
    for(int i = 0; i < state.map->width; i++) {
        for(int j = 0; j < state.map->height; j++) {
            int animType = GetDungeonTile(*state.map, OBJECTS_LAYER, i, j);
            if(animType == TILE_DOOR_Z) {
                ThinWall thinWall;
                thinWall.mapPosition = {i, j};
                thinWall.position = {CUBE_SIZE * (float) i, CUBE_SIZE/2, CUBE_SIZE * (float) j};
                thinWall.orientation = AxisOrientation::Z;
                InitSpriteAnimationPlayer(thinWall.player);
                PlaySpriteAnimation(thinWall.player, GetSpriteAnimation("DoorOpen"), true);
                state.thinWalls.push_back(thinWall);
            }
            if(animType == TILE_DOOR_X) {
                ThinWall thinWall;
                thinWall.mapPosition = {i, j};
                thinWall.position = {CUBE_SIZE * (float) i, CUBE_SIZE/2, CUBE_SIZE * (float) j};
                thinWall.orientation = AxisOrientation::X;
                InitSpriteAnimationPlayer(thinWall.player);
                PlaySpriteAnimation(thinWall.player, GetSpriteAnimation("DoorOpen"), true);
                state.thinWalls.push_back(thinWall);
            }
            if(animType == TILE_RED_CARPET) {
                ThinWall thinWall;
                thinWall.mapPosition = {i, j};
                thinWall.position = {CUBE_SIZE * (float) i, 0.01f, CUBE_SIZE * (float) j};
                thinWall.orientation = AxisOrientation::Y;
                InitSpriteAnimationPlayer(thinWall.player);
                PlaySpriteAnimation(thinWall.player, GetSpriteAnimation("RedCarpetIdle"), true);
                state.thinWalls.push_back(thinWall);
            }
        }
    }
}

void InitDungeonRenderer(DungeonState &state, DungeonMap* map) {
    state.map = map;
    state.camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    state.camera.fovy = 65.0f;
    state.camera.projection = CAMERA_PERSPECTIVE;
    UpdateCamera(state);
    InitLighting(state.lighting, map->width, map->height);
    BuildLightMap(state.lighting, map->layers[LIGHTS_LAYER], map->layers[NAV_LAYER]);
    CreateObjects(state);
    CreateThinWalls(state);
}

void DestroyDungeonRenderer(DungeonState &state) {
    for(auto& texture : state.textures) {
        UnloadTexture(texture.second);
    }
    state.textures.clear();
}

void DungeonRendererUpdate(DungeonState &state, float dt) {
    if(!state.isTurning) {
        UpdateMove(state, dt);
        UpdateCamera(state);
    } else {
        UpdateTurn(state, dt);
    }
    // update anim players
    for(auto& anim : state.wallObjects) {
        UpdateSpriteAnimation(anim.frontPlayer, dt);
        UpdateSpriteAnimation(anim.sidePlayer, dt);
    }
    // Use erase-remove idiom to remove animations which are done
    /*
    state.animPlayers.erase(
            std::remove_if(state.animPlayers.begin(), state.animPlayers.end(),
                           [](const SpriteAnimationPlayer& anim) {
                               return anim;
                           }),
            state.animPlayers.end()
    );
    */
}

void RenderDungeon(DungeonState &state) {
    Vector3 oldCameraPos = state.camera.position;
    //state.camera.position = GetMovedBackPosition(state, 1.0f);

    BeginMode3D(state.camera);
    //rlDisableBackfaceCulling();
    rlEnableDepthTest();
    //rlDisableDepthTest();
    rlPushMatrix();

    RenderMap(state);
    rlPopMatrix();
    state.camera.position = oldCameraPos;
    EndMode3D();
}

void LoadDungeonTexture(DungeonState &state, int type, const std::string &filename) {
    state.textures[type] = LoadTexture(filename.c_str());
    SetTextureFilter(state.textures[type], TEXTURE_FILTER_POINT);  // Nearest neighbor filtering
    SetTextureWrap(state.textures[type], TEXTURE_WRAP_REPEAT);  // Allow tiling
}



