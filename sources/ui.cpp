//
// Created by bison on 19-01-25.
//

#include "ui.h"

void DrawStatusText(const char* text, Color color, int y, int size) {
    DrawText(text, 240 - (MeasureText(text, size) / 2), y, size, color);
}

void DisplayCharacterStatsFloating(Character &character, int x, int y, bool isPlayer) {
    int statusEffectsHeight = (int) character.statusEffects.size() * 12;

    // Calculate the initial rectangle
    auto backgroundRect = (Rectangle) {(float)x, (float)y, 82, 52 + (float) statusEffectsHeight + 4};

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

    DrawText(TextFormat("Health: %d/%d", character.health, character.maxHealth), offsetX, offsetY, 10, WHITE);
    DrawText(TextFormat("Attack: %d", character.attack), offsetX, offsetY + 12, 10, WHITE);
    DrawText(TextFormat("Defense: %d", character.defense), offsetX, offsetY + 24, 10, WHITE);
    DrawText(TextFormat("Speed: %d", character.speed), offsetX, offsetY + 36, 10, WHITE);

    if (statusEffectsHeight > 0) {
        DrawLine(backgroundRect.x, backgroundRect.y + 52, backgroundRect.x + 82, backgroundRect.y + 52, LIGHTGRAY);
        offsetY += 52;
        // Show status effects
        for (size_t i = 0; i < character.statusEffects.size(); ++i) {
            DrawText(TextFormat("%s", GetStatusEffectName(character.statusEffects[i].type).c_str()), offsetX, offsetY + i * 12, 10, YELLOW);
        }
    }
}
