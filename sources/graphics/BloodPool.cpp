//
// Created by bison on 21-01-25.
//

#include "BloodPool.h"
#include "raylib.h"
#include "ai/PathFinding.h"

const Color DARK_BLOOD_RED = Color{139, 0, 0, 255};   // Dark blood red (#8B0000)
const Color MEDIUM_BLOOD_RED = Color{178, 34, 34, 255};  // Medium blood red (#B22222)
const Color LIGHT_BLOOD_RED = Color{204, 51, 51, 255};  // Light blood red (#FF6347)

static RenderTexture2D bloodTexture;

// Initialize the blood texture
void InitBloodRendering() {
    bloodTexture = LoadRenderTexture(gameScreenWidth, gameScreenHeight); // Assuming the screen size
}

// Clean up when done
void DestroyBloodRendering() {
    UnloadRenderTexture(bloodTexture);
}

// Render all blood pools to the precomposition texture
void PreRenderBloodPools(Level &level) {
    // Start rendering to the blood texture
    BeginTextureMode(bloodTexture);
    ClearBackground(BLANK); // Clear to transparent

    BeginBlendMode(BLEND_ADDITIVE);
    for (auto &animation : level.animations) {
        if (animation.type == AnimationType::BloodPool) {
            int posX = (int)animation.state.bloodPool.position.x + (int)animation.state.bloodPool.offset1.x;
            int posY = (int)animation.state.bloodPool.position.y + (int)animation.state.bloodPool.offset1.y;
            Vector2i gridPos = PixelToGridPositionI(posX, posY);
            Color light = GetVertexLight(level.lighting, gridPos.x, gridPos.y);

            Vector2 screenPos = GetWorldToScreen2D({(float)posX, (float)posY}, level.camera.camera);
            screenPos = ceilv(screenPos); // Round to nearest pixel
            Color tint1 = ColorTint(Fade(DARK_BLOOD_RED, 0.85f), light);
            DrawEllipse((int) screenPos.x, (int) screenPos.y, animation.state.bloodPool.radius1, 0.65f * animation.state.bloodPool.radius1, tint1);

            posX = (int)animation.state.bloodPool.position.x + (int)animation.state.bloodPool.offset2.x;
            posY = (int)animation.state.bloodPool.position.y + (int)animation.state.bloodPool.offset2.y;
            screenPos = GetWorldToScreen2D({(float)posX, (float)posY}, level.camera.camera);
            screenPos = ceilv(screenPos); // Round to nearest pixel
            Color tint2 = ColorTint(Fade(DARK_BLOOD_RED, 0.75f), light);
            DrawEllipse((int) screenPos.x, (int) screenPos.y, animation.state.bloodPool.radius2, 0.70f * animation.state.bloodPool.radius2, tint2);

            posX = (int)animation.state.bloodPool.position.x + (int)animation.state.bloodPool.offset3.x;
            posY = (int)animation.state.bloodPool.position.y + (int)animation.state.bloodPool.offset3.y;
            screenPos = GetWorldToScreen2D({(float)posX, (float)posY}, level.camera.camera);
            screenPos = ceilv(screenPos); // Round to nearest pixel

            Color tint3 = ColorTint(Fade(DARK_BLOOD_RED, 0.65f), light);
            DrawCircle((int) screenPos.x, (int) screenPos.y, animation.state.bloodPool.radius3, tint3);
        }
    }
    EndBlendMode();

    EndTextureMode(); // Finish rendering to blood texture
}

// Draw the pre-rendered blood texture in the main render pass
void DrawBloodPools() {
    DrawTextureRec(
            bloodTexture.texture,
            (Rectangle){0, 0, (float)bloodTexture.texture.width, -(float)bloodTexture.texture.height}, // Flip Y-axis
            (Vector2){0, 0},
            WHITE
    );
}
