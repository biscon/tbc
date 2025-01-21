//
// Created by bison on 19-01-25.
//

#ifndef SANDBOX_UI_H
#define SANDBOX_UI_H

#include "raylib.h"
#include "Character.h"

void DrawStatusText(const char* text, Color color, int y, int size);
void DisplayCharacterStatsFloating(Character &character, int x, int y, bool isPlayer);
void DrawSpeechBubble(float x, float y, const char *text, float alpha);

#endif //SANDBOX_UI_H
