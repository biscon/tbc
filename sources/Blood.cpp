//
// Created by bison on 21-01-25.
//

#include "Blood.h"
#include "raylib.h"

const Color DARK_BLOOD_RED = Color{139, 0, 0, 255};   // Dark blood red (#8B0000)
const Color MEDIUM_BLOOD_RED = Color{178, 34, 34, 255};  // Medium blood red (#B22222)
const Color LIGHT_BLOOD_RED = Color{204, 51, 51, 255};  // Light blood red (#FF6347)

static RenderTexture2D bloodTexture;

// Initialize the blood texture
void InitializeBloodRendering() {
    bloodTexture = LoadRenderTexture(480, 270); // Assuming the screen size
}

// Clean up when done
void UnloadBloodRendering() {
    UnloadRenderTexture(bloodTexture);
}

// Render all blood pools to the precomposition texture
void PreRenderBloodPools(CombatState &combat) {
    // Start rendering to the blood texture
    BeginTextureMode(bloodTexture);
    ClearBackground(BLANK); // Clear to transparent

    BeginBlendMode(BLEND_ADDITIVE);
    for (auto &animation : combat.animations) {
        if (animation.type == AnimationType::BloodPool) {
            int posX = (int)animation.state.bloodPool.position.x + (int)animation.state.bloodPool.offset1.x;
            int posY = (int)animation.state.bloodPool.position.y + (int)animation.state.bloodPool.offset1.y;
            DrawEllipse(posX, posY, animation.state.bloodPool.radius1, 0.65f * animation.state.bloodPool.radius1, Fade(DARK_BLOOD_RED, 0.85f));

            posX = (int)animation.state.bloodPool.position.x + (int)animation.state.bloodPool.offset2.x;
            posY = (int)animation.state.bloodPool.position.y + (int)animation.state.bloodPool.offset2.y;
            DrawEllipse(posX, posY, animation.state.bloodPool.radius2, 0.70f * animation.state.bloodPool.radius2, Fade(DARK_BLOOD_RED, 0.75f));

            posX = (int)animation.state.bloodPool.position.x + (int)animation.state.bloodPool.offset3.x;
            posY = (int)animation.state.bloodPool.position.y + (int)animation.state.bloodPool.offset3.y;
            DrawCircle(posX, posY, animation.state.bloodPool.radius3, Fade(DARK_BLOOD_RED, 0.65f));
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
