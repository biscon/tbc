//
// Created by bison on 20-08-25.
//

#include "GraphicUtil.h"
#include <cmath>

static inline unsigned char FloatToByte(float f) {
    if (f <= 0.0f) return 0;
    if (f >= 1.0f) return 255;
    return (unsigned char)(f * 255.0f + 0.5f);
}

static inline float SrgbToLinear(float c) {
    // Input c is [0..1] sRGB, output is linear
    return powf(c, 2.2f);
}

Texture2D LoadTextureLinearized(const char* fileName, bool premultiplyAlpha) {
    // Load image into CPU memory
    Image img = LoadImage(fileName);
    if (img.data == nullptr) {
        TraceLog(LOG_ERROR, "Failed to load image: %s", fileName);
        return Texture2D{}; // return empty
    }

    Color* pixels = LoadImageColors(img);

    // Modify pixels in place
    for (int i = 0; i < img.width * img.height; i++) {
        Color& c = pixels[i];

        float r = (float)c.r / 255.0f;
        float g = (float)c.g / 255.0f;
        float b = (float)c.b / 255.0f;
        float a = (float)c.a / 255.0f;

        // sRGB -> linear
        r = powf(r, 2.2f);
        g = powf(g, 2.2f);
        b = powf(b, 2.2f);

        if (premultiplyAlpha) {
            r *= a;
            g *= a;
            b *= a;
        }

        c.r = (unsigned char)(fminf(r, 1.0f) * 255.0f + 0.5f);
        c.g = (unsigned char)(fminf(g, 1.0f) * 255.0f + 0.5f);
        c.b = (unsigned char)(fminf(b, 1.0f) * 255.0f + 0.5f);
        c.a = (unsigned char)(a * 255.0f + 0.5f);
    }

    // Create a new Image that owns the modified pixels
    Image newImg = {
            .data = pixels,
            .width = img.width,
            .height = img.height,
            .mipmaps = 1,
            .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    // Upload to GPU
    Texture2D tex = LoadTextureFromImage(newImg);

    // Free CPU pixel buffer once
    UnloadImage(newImg);

    // Free the original unmodified image
    UnloadImage(img);

    return tex;
}

Texture2D LoadTexturePreMultiplied(const char* fileName) {
    // Load image into CPU memory
    Image img = LoadImage(fileName);
    if (img.data == nullptr) {
        TraceLog(LOG_ERROR, "Failed to load image: %s", fileName);
        return Texture2D{}; // return empty
    }
    ImageAlphaPremultiply(&img);
    // Upload to GPU
    Texture2D tex = LoadTextureFromImage(img);
    // Free the original unmodified image
    UnloadImage(img);
    return tex;
}
