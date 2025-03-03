//
// Created by bison on 28-01-25.
//

#ifndef SANDBOX_SPRITESHEET_H
#define SANDBOX_SPRITESHEET_H

#include <vector>
#include "raylib.h"
#include "util/MathUtil.h"

struct SpriteSheetFrameSizeData {
    int frameWidth;
    int frameHeight;
};

struct SpriteSheetData {
    std::vector<Texture2D> texture;
    std::vector<SpriteSheetFrameSizeData> frameSizeData;
    std::vector<std::vector<Rectangle>> frameRects;
};

// Load a sprite sheet from a file and split it into frames
int LoadSpriteSheet(SpriteSheetData& sheetData, const char* filename, int frameWidth, int frameHeight);
void UnloadSpriteSheet(SpriteSheetData& sheetData, int spriteSheet);

#endif //SANDBOX_SPRITESHEET_H
