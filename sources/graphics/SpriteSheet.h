//
// Created by bison on 28-01-25.
//

#ifndef SANDBOX_SPRITESHEET_H
#define SANDBOX_SPRITESHEET_H

#include <vector>
#include "raylib.h"
#include "util/MathUtil.h"
#include "data/SpriteData.h"


// Load a sprite sheet from a file and split it into frames
int LoadSpriteSheet(SpriteSheetData& sheetData, const char* filename, int frameWidth, int frameHeight, bool autoRects = true);
void UnloadSpriteSheet(SpriteSheetData& sheetData, int spriteSheet);
int AddSpriteSheetFrame(SpriteSheetData& sheetData, int spriteSheet, int x, int y, int w, int h);

#endif //SANDBOX_SPRITESHEET_H
