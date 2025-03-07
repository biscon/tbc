//
// Created by bison on 28-01-25.
//

#include "SpriteSheet.h"

// Load a sprite sheet from a file and split it into frames
int LoadSpriteSheet(SpriteSheetData& sheetData, const char* filename, int frameWidth, int frameHeight) {

    // Load the texture
    Texture2D texture = LoadTexture(filename);
    sheetData.texture.emplace_back(texture);
    sheetData.frameSizeData.push_back({ frameWidth, frameHeight});

    // Calculate the number of frames in the sprite sheet
    int columns = texture.width / frameWidth;
    int rows = texture.height / frameHeight;

    sheetData.frameRects.emplace_back();
    // Populate the frameRects vector
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < columns; ++x) {
            Rectangle frame = {
                    static_cast<float>(x * frameWidth), // x position
                    static_cast<float>(y * frameHeight), // y position
                    static_cast<float>(frameWidth), // frame width
                    static_cast<float>(frameHeight)  // frame height
            };
            sheetData.frameRects.back().push_back(frame);
        }
    }
    sheetData.loaded.push_back(true);
    return (int) sheetData.texture.size()-1;
}

void UnloadSpriteSheet(SpriteSheetData& sheetData, int spriteSheet) {
    UnloadTexture(sheetData.texture[spriteSheet]);
    sheetData.loaded[spriteSheet] = false;
}
