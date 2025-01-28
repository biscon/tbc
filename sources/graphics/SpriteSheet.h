//
// Created by bison on 28-01-25.
//

#ifndef SANDBOX_SPRITESHEET_H
#define SANDBOX_SPRITESHEET_H

#include <vector>
#include "raylib.h"

struct SpriteSheet {
    Texture2D texture;
    int frameWidth;
    int frameHeight;
    std::vector<Rectangle> frameRects;
};

// Load a sprite sheet from a file and split it into frames
void LoadSpriteSheet(SpriteSheet& spriteSheet, const char* filename, int frameWidth, int frameHeight);
void UnloadSpriteSheet(SpriteSheet& spriteSheet);

#endif //SANDBOX_SPRITESHEET_H
