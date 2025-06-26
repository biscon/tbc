//
// Created by bison on 19-01-25.
//

#ifndef SANDBOX_UI_H
#define SANDBOX_UI_H

#include "raylib.h"
#include "character/Character.h"

void DrawStatusText(const char* text, Color color, int y, int size);
void DrawStatusTextBg(const char* text, Color color, int y, int size, Font font);
//void DisplayCharacterStatsFloating(CharacterData& charData, int character, int x, int y, bool isPlayer);
void DisplayCharacterStatsFloating(CharacterData& charData, int character, int x, int y, bool isPlayer, Font font);
void DrawSpeechBubble(float x, float y, const char *text, float alpha);
Color GetDamageColor(int dmg, int attackerAttack);

#endif //SANDBOX_UI_H
