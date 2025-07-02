//
// Created by bison on 19-01-25.
//

#include "UI.h"
#include "raymath.h"
#include "data/GameData.h"
#include "graphics/SpriteSheet.h"

static const Color bgColor = Color{15, 15, 15, 200};

void DrawStatusText(const char* text, Color color, int y, int size) {
    DrawText(text, gameScreenHalfWidth - (MeasureText(text, size) / 2), y, size, color);
}

void DrawStatusTextBg(const char* text, Color color, int y, int size, Font font) {
    Vector2 textDims = MeasureTextEx(font, text, size, 1.0f);

    int textWidth = (int) textDims.x;
    int textHeight = (int) textDims.y; // Approximate height as the font size

    // Padding around the text
    int padding = 8;

    // Calculate the background rectangle
    int rectX = gameScreenHalfWidth - (textWidth / 2) - padding;
    int rectY = y - padding / 2;
    int rectWidth = textWidth + padding * 2;
    int rectHeight = textHeight + padding;

    // Draw semi-transparent rounded black background
    DrawRectangleRounded((Rectangle){ (float)rectX, (float)rectY, (float)rectWidth, (float)rectHeight }, 0.4f, 8, (Color){ 15, 15, 15, 200 });

    Vector2 textPos = {
            roundf(gameScreenHalfWidth - (textWidth / 2)),
            floorf(y)
    };
    // Draw the text
    DrawTextEx(font, text, textPos, size, 1, color);
}

void RenderCharacterStats(CharacterData& charData, int character, int x, int y, int width, Font font, float fontSize) {
    const float spacing = 1.0f;
    const float padding = 0.0f;
    const float lineHeight = fontSize + 4;

    CharacterStats& stats = charData.stats[character];
    const std::string& name = charData.name[character];

    // Prepare label-value pairs
    std::vector<std::pair<std::string, std::string>> lines = {
            { "Strength", TextFormat("%d", stats.STR) },
            { "Reflexes", TextFormat("%d", stats.REF) },
            { "Endurance", TextFormat("%d", stats.END) },
            { "Intelligence", TextFormat("%d", stats.INT) },
            { "Perception", TextFormat("%d", stats.PER) },
            { "Charisma", TextFormat("%d", stats.CHA) },
            { "Luck", TextFormat("%d", stats.LUK) },
            { "Level", TextFormat("%d", stats.LVL) },
            { "Health", TextFormat("%d/%d", stats.HP, CalculateCharHealth(stats)) },
            { "Action Points", TextFormat("%d/%d", stats.AP, CalculateCharMaxAP(stats)) },
    };


    float height = (lines.size() * lineHeight) + 3 * padding;

    Rectangle bg = {
            (float)x, (float)y,
            (float) width,
            height
    };


    //DrawRectangleRoundedLinesEx(bg, 0.1f, 16, 1.0f, DARKGRAY);

    // Stats
    float lineY = bg.y;
    int count = 0;
    for (const auto& [label, value] : lines) {
        Vector2 labelPos = { roundf(bg.x + padding), roundf(lineY) };
        Vector2 valuePos = {
                roundf(bg.x + bg.width - padding - MeasureTextEx(font, value.c_str(), fontSize, spacing).x),
                roundf(lineY)
        };
        DrawTextEx(font, label.c_str(), labelPos, fontSize, spacing, LIGHTGRAY);
        DrawTextEx(font, value.c_str(), valuePos, fontSize, spacing, GRAY);
        lineY += count == 6 ? lineHeight + 5 : lineHeight;
        if(count == 6) {
            DrawLine(bg.x, lineY - 5, bg.x + bg.width, lineY - 5, DARKGRAY);
        }
        count++;
    }
}





void DisplayCharacterStatsFloating(CharacterData& charData, int character, int x, int y, bool isPlayer, Font font) {
    const float fontSize = 5.0f;
    const float spacing = 1.0f;
    const float padding = 4.0f;
    const float lineHeight = fontSize + 2;

    CharacterStats& stats = charData.stats[character];
    const std::string& name = charData.name[character];

    // Prepare label-value pairs
    std::vector<std::pair<std::string, std::string>> lines = {
            { "STR", TextFormat("%d", stats.STR) },
            { "REF", TextFormat("%d", stats.REF) },
            { "END", TextFormat("%d", stats.END) },
            { "INT", TextFormat("%d", stats.INT) },
            { "PER", TextFormat("%d", stats.PER) },
            { "CHA", TextFormat("%d", stats.CHA) },
            { "LUK", TextFormat("%d", stats.LUK) },
            { "Level", TextFormat("%d", stats.LVL) },
            { "HP", TextFormat("%d/%d", stats.HP, CalculateCharHealth(stats)) },
    };


    // Include status effect height
    int statusCount = (int)charData.statusEffects[character].size();
    float statusHeight = statusCount * lineHeight;
    bool hasStatus = statusCount > 0;

    // Measure max label and value widths
    float labelWidth = 0;
    float valueWidth = 0;
    for (const auto& [label, value] : lines) {
        labelWidth = std::max(labelWidth, MeasureTextEx(font, label.c_str(), fontSize, spacing).x);
        valueWidth = std::max(valueWidth, MeasureTextEx(font, value.c_str(), fontSize, spacing).x);
    }

    float titleWidth = MeasureTextEx(font, name.c_str(), fontSize, spacing).x;
    float contentWidth = labelWidth + 8 + valueWidth;
    float width = std::max(titleWidth, contentWidth) + 2 * padding;

    float height = (lines.size() * lineHeight) + (hasStatus ? (statusHeight + lineHeight) : 0) + 3 * padding;

    Rectangle bg = {
            (float)x, (float)y,
            width,
            height
    };

    // Clamp into screen bounds
    if (bg.x + bg.width > gameScreenWidth)
        bg.x = gameScreenWidth - bg.width - 2;
    if (bg.x < 0)
        bg.x = 0;
    if (bg.y + bg.height > gameScreenHeight)
        bg.y = gameScreenHeight - bg.height - 2;
    if (bg.y < 0)
        bg.y = 0;

    static const Color bgColor = Color{15, 15, 15, 200};

    DrawRectangleRounded(bg, 0.1f, 16, bgColor);
    DrawRectangleRoundedLinesEx(bg, 0.1f, 16, 1.0f, DARKGRAY);

    // Title
    Vector2 namePos = {
            roundf(bg.x + padding),
            roundf(bg.y + padding)
    };
    DrawTextEx(font, name.c_str(), namePos, fontSize, spacing, YELLOW);

    // Stats
    float lineY = namePos.y + lineHeight + 1;
    for (const auto& [label, value] : lines) {
        Vector2 labelPos = { roundf(bg.x + padding), roundf(lineY) };
        Vector2 valuePos = {
                roundf(bg.x + bg.width - padding - MeasureTextEx(font, value.c_str(), fontSize, spacing).x),
                roundf(lineY)
        };
        DrawTextEx(font, label.c_str(), labelPos, fontSize, spacing, LIGHTGRAY);
        DrawTextEx(font, value.c_str(), valuePos, fontSize, spacing, LIGHTGRAY);
        lineY += lineHeight;
    }

    // Status effects
    if (hasStatus) {
        DrawLine(
                (int)(bg.x + padding),
                (int)(lineY + 1),
                (int)(bg.x + bg.width - padding),
                (int)(lineY + 1),
                LIGHTGRAY
        );

        lineY += lineHeight;
        for (int i = 0; i < statusCount; ++i) {
            const std::string& effectName = GetStatusEffectName(charData.statusEffects[character][i].type);
            Vector2 pos = { roundf(bg.x + padding), roundf(lineY) };
            DrawTextEx(font, effectName.c_str(), pos, fontSize, spacing, YELLOW);
            lineY += lineHeight;
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

void RenderButtons(const std::unordered_map<std::string, Button> &buttons, const Font& font, float fontSize) {
    for(const auto& entry : buttons) {
        auto& button = entry.second;
        DrawRectangleRounded(button.region.rect, 0.30f, 4, bgColor);
        DrawRectangleRoundedLinesEx(button.region.rect, 0.30f, 4, 1.0f, button.hovered ? WHITE : DARKGRAY);
        Vector2 textDim = MeasureTextEx(font, button.label.c_str(), 5, 1);
        Vector2 pos = {
                roundf(button.region.rect.x + (button.region.rect.width / 2.0f) - (textDim.x / 2.0f)),
                floorf(button.region.rect.y + 3.0f)
        };
        DrawTextEx(font, button.label.c_str(), pos, fontSize, 1, button.hovered ? WHITE : GRAY);
    }
}

bool HandleInputButtons(std::unordered_map<std::string, Button> &buttons) {
    Vector2 mouse = GetMousePosition();
    bool anyHovered = false;
    for (auto& entry : buttons) {
        auto& button = entry.second;
        button.hovered = CheckCollisionPointRec(mouse, button.region.rect);
        if(button.hovered) {
            anyHovered = true;
        }
        button.region.Update(mouse);
    }
    return anyHovered;
}

void DrawRectangleCorners(Rectangle& rect, Color color, int cornerSize) {
    int x = (int) rect.x;
    int y = (int) rect.y;
    int w = (int) rect.width;
    int h = (int) rect.height;

    int right = x + w;
    int bottom = y + h;

    // Top-left
    DrawLine(x+1, y, x + cornerSize, y, color);       // horizontal
    DrawLine(x+1, y+1, x+1, y + cornerSize, color);       // vertical


    // Top-right
    DrawLine(right - cornerSize, y, right-1, y, color);      // horizontal
    DrawLine(right, y+1, right, y + cornerSize, color);      // vertical


    // Bottom-left
    DrawLine(x+1, bottom -1 , x + cornerSize, bottom - 1, color);    // horizontal
    DrawLine(x+1, bottom - cornerSize, x + 1, bottom-1, color);    // vertical


    // Bottom-right
    DrawLine(right - cornerSize, bottom - 1, right-1, bottom - 1, color);  // horizontal
    DrawLine(right, bottom - cornerSize, right, bottom-1, color);  // vertical
}

void InitUI(GameData &data) {
    int sheet = LoadSpriteSheet(data.spriteData.sheet, ASSETS_PATH"icons_16x16.png", 16, 16);
    data.ui.iconSpriteSheet = sheet;
}

void DestroyUI(GameData &data) {
    UnloadSpriteSheet(data.spriteData.sheet, data.ui.iconSpriteSheet);
}

void DrawToolTip(Font& font, float fontSize, float spacing, std::string tooltip) {
    Vector2 mouse = GetMousePosition();
    Vector2 size = MeasureTextEx(font, tooltip.c_str(), fontSize, spacing);
    size.x = ceilf(size.x); size.y = ceilf(size.y);

    Rectangle tipRect = {floorf(mouse.x + 8), ceilf(mouse.y + 8), size.x + 4, size.y + 4};
    DrawRectangleRec(tipRect, Color{15, 15, 15, 255});
    DrawRectangleLinesEx(tipRect, 1, DARKGRAY);
    DrawTextEx(font, tooltip.c_str(), {tipRect.x + 2, tipRect.y + 2}, fontSize, spacing, LIGHTGRAY);
}
