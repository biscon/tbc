//
// Created by bison on 19-01-25.
//

#ifndef SANDBOX_UI_H
#define SANDBOX_UI_H

#include "raylib.h"
#include "character/Character.h"

void InitUI(GameData& data);
void DestroyUI(GameData& data);
void DrawStatusText(const char* text, Color color, int y, int size);
void DrawStatusTextBg(const char* text, Color color, int y, int size, Font font);
void RenderCharacterStats(CharacterData& charData, int character, int x, int y, int width, Font font, float fontSize);
void DisplayCharacterStatsFloating(CharacterData& charData, int character, int x, int y, bool isPlayer, Font font);
void ClampToScreenBounds(Rectangle& bg);
void DrawSpeechBubble(float x, float y, const char *text, float alpha);
Color GetDamageColor(int dmg, int attackerAttack);
void RenderButtons(const std::unordered_map<std::string, Button> &buttons, const Font& font, float fontSize);
bool HandleInputButtons(std::unordered_map<std::string, Button> &buttons);
void DrawRectangleCorners(Rectangle& rect, Color color, int cornerSize);
void DrawToolTip(Font& font, float fontSize, float spacing, std::string tooltip);

#endif //SANDBOX_UI_H
