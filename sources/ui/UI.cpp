//
// Created by bison on 19-01-25.
//

#include "UI.h"
#include "raymath.h"

void DrawStatusText(const char* text, Color color, int y, int size) {
    DrawText(text, 240 - (MeasureText(text, size) / 2), y, size, color);
}

void DisplayCharacterStatsFloating(Character &character, int x, int y, bool isPlayer) {
    int statusEffectsHeight = (int) character.statusEffects.size() * 12;

    // Calculate the initial rectangle
    auto backgroundRect = (Rectangle) {(float)x, (float)y, 82, 64 + (float) statusEffectsHeight + 4};

    // Adjust the rectangle position to fit within the screen boundaries
    if (backgroundRect.x + backgroundRect.width > 480) {
        backgroundRect.x = 480 - backgroundRect.width - 2; // Push left
    }
    if (backgroundRect.x < 0) {
        backgroundRect.x = 0; // Push right
    }
    if (backgroundRect.y + backgroundRect.height > 270) {
        backgroundRect.y = 270 - backgroundRect.height - 2; // Push up
    }
    if (backgroundRect.y < 0) {
        backgroundRect.y = 0; // Push down
    }

    // Recalculate text offsets based on the adjusted position
    int offsetX = (int)backgroundRect.x + 4;
    int offsetY = (int)backgroundRect.y + 4;

    // Draw the adjusted rectangle and its content
    DrawRectangleRounded(backgroundRect, 0.1f, 16, DARKGRAY);
    DrawRectangleRoundedLinesEx(backgroundRect, 0.1f, 16, 1.0f, LIGHTGRAY);
    DrawText(TextFormat("%s", character.name.c_str()), offsetX, offsetY, 10, YELLOW);
    DrawText(TextFormat("Health: %d/%d", character.health, character.maxHealth), offsetX, offsetY + 12, 10, WHITE);
    DrawText(TextFormat("Attack: %d", character.attack), offsetX, offsetY + 24, 10, WHITE);
    DrawText(TextFormat("Defense: %d", character.defense), offsetX, offsetY + 36, 10, WHITE);
    DrawText(TextFormat("Speed: %d", character.speed), offsetX, offsetY + 48, 10, WHITE);

    if (statusEffectsHeight > 0) {
        DrawLine(backgroundRect.x, backgroundRect.y + 64, backgroundRect.x + 82, backgroundRect.y + 64, LIGHTGRAY);
        offsetY += 64;
        // Show status effects
        for (size_t i = 0; i < character.statusEffects.size(); ++i) {
            DrawText(TextFormat("%s", GetStatusEffectName(character.statusEffects[i].type).c_str()), offsetX, offsetY + i * 12, 10, YELLOW);
        }
    }
}

void DrawSpeechBubble(float x, float y, const char *text, float alpha) {
    // Measure the width of the text
    int textWidth = MeasureText(text, 10);
    int bubbleWidth = textWidth + 20; // Add padding
    int bubbleHeight = 18;

    // Calculate the bubble position
    float bubbleX = x - bubbleWidth/2; // Offset to avoid the stalk overlapping the bubble
    float bubbleY = y - bubbleHeight - 2; // Position above the target point

    // Ensure the bubble stays inside the screen bounds (480x270)
    if (bubbleX + bubbleWidth > 480) bubbleX = 480 - bubbleWidth; // Prevent overflow on the right
    if (bubbleX < 0) bubbleX = 0; // Prevent overflow on the left
    if (bubbleY < 0) bubbleY = 0; // Prevent overflow at the top
    if (bubbleY + bubbleHeight > 270) bubbleY = 270 - bubbleHeight; // Prevent overflow at the bottom

    // Draw the bubble (rounded rectangle)
    Color bubbleColor = {255, 255, 255, 200}; // Light white color with transparency
    DrawRectangleRounded((Rectangle){bubbleX, bubbleY, (float) bubbleWidth, (float) bubbleHeight}, 0.2f, 20, Fade(bubbleColor, alpha));

    // Draw the text inside the bubble
    DrawText(text, bubbleX + 10, bubbleY + 5, 10, Fade(BLACK, alpha));
}

Color GetDamageColor(int dmg) {
    // Clamp damage between 1 and 40
    dmg = Clamp(dmg, 1, 40);

    // Define key gradient colors
    Color white = WHITE;
    Color yellow = YELLOW;
    Color red = RED;

    // Determine gradient range
    if (dmg <= 20) {
        // Interpolate from white to yellow (1 to 20)
        float t = (float)(dmg - 1) / 19.0f; // Normalized to 0-1
        return Color{
                (unsigned char)Lerp(white.r, yellow.r, t),
                (unsigned char)Lerp(white.g, yellow.g, t),
                (unsigned char)Lerp(white.b, yellow.b, t),
                255
        };
    } else {
        // Interpolate from yellow to red (21 to 40)
        float t = (float)(dmg - 21) / 19.0f; // Normalized to 0-1
        return Color{
                (unsigned char)Lerp(yellow.r, red.r, t),
                (unsigned char)Lerp(yellow.g, red.g, t),
                (unsigned char)Lerp(yellow.b, red.b, t),
                255
        };
    }
}

