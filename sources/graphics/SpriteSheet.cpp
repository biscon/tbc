//
// Created by bison on 28-01-25.
//

#include "SpriteSheet.h"

// Load a sprite sheet from a file and split it into frames
void LoadSpriteSheet(SpriteSheet& spriteSheet, const char* filename, int frameWidth, int frameHeight) {
    // Load the texture
    spriteSheet.texture = LoadTexture(filename);
    spriteSheet.frameWidth = frameWidth;
    spriteSheet.frameHeight = frameHeight;

    // Calculate the number of frames in the sprite sheet
    int columns = spriteSheet.texture.width / frameWidth;
    int rows = spriteSheet.texture.height / frameHeight;

    // Populate the frameRects vector
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < columns; ++x) {
            Rectangle frame = {
                    static_cast<float>(x * frameWidth), // x position
                    static_cast<float>(y * frameHeight), // y position
                    static_cast<float>(frameWidth), // frame width
                    static_cast<float>(frameHeight)  // frame height
            };
            spriteSheet.frameRects.push_back(frame);
        }
    }
}

void UnloadSpriteSheet(SpriteSheet &spriteSheet) {
    UnloadTexture(spriteSheet.texture);
}
