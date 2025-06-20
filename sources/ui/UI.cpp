//
// Created by bison on 19-01-25.
//

#include "UI.h"
#include "raymath.h"
#include "data/GameData.h"

void DrawStatusText(const char* text, Color color, int y, int size) {
    DrawText(text, gameScreenHalfWidth - (MeasureText(text, size) / 2), y, size, color);
}

void DrawStatusTextBg(const char* text, Color color, int y, int size) {
    int textWidth = MeasureText(text, size);
    int textHeight = size; // Approximate height as the font size

    // Padding around the text
    int padding = 8;

    // Calculate the background rectangle
    int rectX = gameScreenHalfWidth - (textWidth / 2) - padding;
    int rectY = y - padding / 2;
    int rectWidth = textWidth + padding * 2;
    int rectHeight = textHeight + padding;

    // Draw semi-transparent rounded black background
    DrawRectangleRounded((Rectangle){ (float)rectX, (float)rectY, (float)rectWidth, (float)rectHeight }, 0.4f, 8, (Color){ 0, 0, 0, 128 });

    // Draw the text
    DrawText(text, gameScreenHalfWidth - (textWidth / 2), y, size, color);
}

void DisplayCharacterStatsFloating(CharacterData& charData, int character, int x, int y, bool isPlayer) {
    int statusEffectsHeight = (int)charData.statusEffects[character].size() * 12;

    // Calculate the initial rectangle
    auto backgroundRect = (Rectangle){(float)x, (float)y, 82, 76 + (float)statusEffectsHeight + 4}; // Adjusted height for level

    // Adjust the rectangle position to fit within the screen boundaries
    if (backgroundRect.x + backgroundRect.width > gameScreenWidth) {
        backgroundRect.x = gameScreenWidth - backgroundRect.width - 2; // Push left
    }
    if (backgroundRect.x < 0) {
        backgroundRect.x = 0; // Push right
    }
    if (backgroundRect.y + backgroundRect.height > gameScreenHeight) {
        backgroundRect.y = gameScreenHeight - backgroundRect.height - 2; // Push up
    }
    if (backgroundRect.y < 0) {
        backgroundRect.y = 0; // Push down
    }

    // Recalculate text offsets based on the adjusted position
    int offsetX = (int)backgroundRect.x + 4;
    int offsetY = (int)backgroundRect.y + 4;

    CharacterStats& stats = charData.stats[character];
    // Draw the adjusted rectangle and its content
    DrawRectangleRounded(backgroundRect, 0.1f, 16, DARKGRAY);
    DrawRectangleRoundedLinesEx(backgroundRect, 0.1f, 16, 1.0f, LIGHTGRAY);
    DrawText(TextFormat("%s", charData.name[character].c_str()), offsetX, offsetY, 10, YELLOW);
    DrawText(TextFormat("Level: %d", stats.level), offsetX, offsetY + 12, 10, WHITE); // Added level
    DrawText(TextFormat("Health: %d/%d", stats.health, stats.maxHealth), offsetX, offsetY + 24, 10, WHITE);
    DrawText(TextFormat("Attack: %d", stats.attack), offsetX, offsetY + 36, 10, WHITE);
    DrawText(TextFormat("Defense: %d", stats.defense), offsetX, offsetY + 48, 10, WHITE);
    DrawText(TextFormat("Speed: %d", stats.speed), offsetX, offsetY + 60, 10, WHITE); // Adjusted offsets

    if (statusEffectsHeight > 0) {
        DrawLine(backgroundRect.x, backgroundRect.y + 76, backgroundRect.x + 82, backgroundRect.y + 76, LIGHTGRAY); // Adjusted separator position
        offsetY += 76;
        // Show status effects
        for (size_t i = 0; i < charData.statusEffects[character].size(); ++i) {
            DrawText(TextFormat("%s", GetStatusEffectName(charData.statusEffects[character][i].type).c_str()), offsetX, offsetY + i * 12, 10, YELLOW);
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
    /*
    if (bubbleX + bubbleWidth > 480) bubbleX = 480 - bubbleWidth; // Prevent overflow on the right
    if (bubbleX < 0) bubbleX = 0; // Prevent overflow on the left
    if (bubbleY < 0) bubbleY = 0; // Prevent overflow at the top
    if (bubbleY + bubbleHeight > 270) bubbleY = 270 - bubbleHeight; // Prevent overflow at the bottom
     */

    // Draw the bubble (rounded rectangle)
    Color bubbleColor = {255, 255, 255, 200}; // Light white color with transparency
    DrawRectangleRounded((Rectangle){bubbleX, bubbleY, (float) bubbleWidth, (float) bubbleHeight}, 0.2f, 20, Fade(bubbleColor, alpha));

    // Draw the text inside the bubble
    DrawText(text, bubbleX + 10, bubbleY + 5, 10, Fade(BLACK, alpha));
}

Color GetDamageColor(int dmg, int attackerAttack) {
    // Define key gradient colors
    Color white = WHITE;   // Low baseAttack
    Color yellow = YELLOW; // Medium baseAttack
    Color red = RED;       // High baseAttack

    // Define dynamic thresholds based on attacker's attack stat
    int lowThreshold = attackerAttack / 2;     // Low baseAttack threshold (e.g., half of attack)
    int highThreshold = attackerAttack * 3 / 2; // High baseAttack threshold (e.g., 1.5x attack)

    // Clamp baseAttack within the calculated range
    dmg = Clamp(dmg, lowThreshold, highThreshold);

    // Calculate interpolation factor based on thresholds
    if (dmg <= attackerAttack) {
        // Interpolate from white to yellow (low to medium baseAttack)
        float t = (float)(dmg - lowThreshold) / (attackerAttack - lowThreshold);
        return Color{
                (unsigned char)Lerp(white.r, yellow.r, t),
                (unsigned char)Lerp(white.g, yellow.g, t),
                (unsigned char)Lerp(white.b, yellow.b, t),
                255
        };
    } else {
        // Interpolate from yellow to red (medium to high baseAttack)
        float t = (float)(dmg - attackerAttack) / (highThreshold - attackerAttack);
        return Color{
                (unsigned char)Lerp(yellow.r, red.r, t),
                (unsigned char)Lerp(yellow.g, red.g, t),
                (unsigned char)Lerp(yellow.b, red.b, t),
                255
        };
    }
}
