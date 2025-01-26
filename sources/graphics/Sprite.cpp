//
// Created by bison on 15-01-25.
//

#include <cstring>
#include <utility>
#include "Sprite.h"

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

void CreateSpriteAnimation(SpriteAnimationManager& manager, const char* name, SpriteAnimationType type, SpriteSheet *spriteSheet,
                           std::vector<int> frames, std::vector<float> frameDelays, Vector2 origin) {
    SpriteAnimation animation;
    strncpy(animation.name, name, 64);
    animation.type = type;
    animation.spriteSheet = spriteSheet;
    animation.frames = std::move(frames);
    animation.frameDelays = std::move(frameDelays);
    animation.origin = origin;
    manager.animations.push_back(animation);
}

SpriteAnimation *GetSpriteAnimation(SpriteAnimationManager &manager, const char *name, SpriteAnimationType type) {
    for(auto &anim : manager.animations) {
        if(strcmp(anim.name, name) == 0 && anim.type == type) {
            return &anim;
        }
    }
    return nullptr;
}

void UpdateSpriteAnimation(SpriteAnimationPlayer& player, float dt) {
    if (!player.playing || !player.animation || player.animation->frames.empty()) {
        return;
    }

    // Update the frame time
    player.frameTime += dt;

    // Get the current frame's delay
    float currentDelay = player.animation->frameDelays[player.currentFrame];

    // Check if it's time to move to the next frame
    if (player.frameTime >= currentDelay) {
        player.frameTime -= currentDelay; // Subtract the delay to handle leftover time

        if (player.currentFrame + 1 < player.animation->frames.size()) {
            // Move to the next frame
            player.currentFrame++;
        } else if (player.loop) {
            // Loop back to the first frame if looping is enabled
            player.currentFrame = 0;
        } else {
            // Stop the animation at the last frame
            player.playing = false;
        }
    }
}

void DrawSpriteAnimation(SpriteAnimationPlayer& player) {
    if (!player.animation || !player.animation->spriteSheet || player.animation->frames.empty()) {
        return;
    }

    // Get the current frame index
    int frameIndex = player.animation->frames[player.currentFrame];

    // Get the source rectangle from the sprite sheet
    const SpriteSheet* spriteSheet = player.animation->spriteSheet;
    if (frameIndex < 0 || frameIndex >= spriteSheet->frameRects.size()) {
        return; // Invalid frame index, skip drawing
    }
    Rectangle sourceRect = spriteSheet->frameRects[frameIndex];

    // Calculate destination rectangle without manually applying origin
    Rectangle destRect = {
            player.position.x,                                // World x position
            player.position.y,                                // World y position
            sourceRect.width * player.scale.x, // Scaled width
            sourceRect.height * player.scale.y // Scaled height
    };

    // Define the rotation origin relative to the sprite space
    Vector2 rotationOrigin = {
            player.animation->origin.x * player.scale.x, // Scaled x origin
            player.animation->origin.y * player.scale.y  // Scaled y origin
    };

    // Draw the texture with rotation, scale, and tint
    DrawTexturePro(
            spriteSheet->texture, // The texture
            sourceRect,           // The source rectangle
            destRect,             // The destination rectangle
            rotationOrigin,       // Rotation origin relative to destRect
            player.rotation,      // The rotation in degrees
            player.tint           // The color tint
    );
}

void DrawSpriteAnimation(SpriteAnimationPlayer& player, float x, float y) {
    if (!player.animation || !player.animation->spriteSheet || player.animation->frames.empty()) {
        return;
    }

    // Get the current frame index
    int frameIndex = player.animation->frames[player.currentFrame];

    // Get the source rectangle from the sprite sheet
    const SpriteSheet* spriteSheet = player.animation->spriteSheet;
    if (frameIndex < 0 || frameIndex >= spriteSheet->frameRects.size()) {
        return; // Invalid frame index, skip drawing
    }
    Rectangle sourceRect = spriteSheet->frameRects[frameIndex];

    // Calculate destination rectangle without manually applying origin
    Rectangle destRect = {
            x,                                // World x position
            y,                                // World y position
            sourceRect.width * player.scale.x, // Scaled width
            sourceRect.height * player.scale.y // Scaled height
    };

    // Define the rotation origin relative to the sprite space
    Vector2 rotationOrigin = {
            player.animation->origin.x * player.scale.x, // Scaled x origin
            player.animation->origin.y * player.scale.y  // Scaled y origin
    };

    // Draw the texture with rotation, scale, and tint
    DrawTexturePro(
            spriteSheet->texture, // The texture
            sourceRect,           // The source rectangle
            destRect,             // The destination rectangle
            rotationOrigin,       // Rotation origin relative to destRect
            player.rotation,      // The rotation in degrees
            player.tint           // The color tint
    );
}

void InitSpriteAnimationPlayer(SpriteAnimationPlayer &player) {
    player.animation = nullptr;
    player.currentFrame = 0;
    player.frameTime = 0;
    player.loop = true;
    player.playing = false;
    player.position = {0, 0};
    player.rotation = 0;
    player.scale = {1, 1};
    player.tint = WHITE;
}


void InitCharacterSprite(CharacterSprite &sprite, SpriteAnimationManager &animationManager, const char *walkUp,
                         const char *walkDown, const char *walkLeft, const char *walkRight) {
    InitSpriteAnimationPlayer(sprite.player);
    sprite.animations[SpriteAnimationType::WalkUp] = GetSpriteAnimation(animationManager, walkUp, SpriteAnimationType::WalkUp);
    sprite.animations[SpriteAnimationType::WalkDown] = GetSpriteAnimation(animationManager, walkDown, SpriteAnimationType::WalkDown);
    sprite.animations[SpriteAnimationType::WalkLeft] = GetSpriteAnimation(animationManager, walkLeft, SpriteAnimationType::WalkLeft);
    sprite.animations[SpriteAnimationType::WalkRight] = GetSpriteAnimation(animationManager, walkRight, SpriteAnimationType::WalkRight);
}

void InitCharacterSprite(CharacterSprite &sprite, SpriteAnimationManager &animationManager, const std::string& animType) {
    InitSpriteAnimationPlayer(sprite.player);
    sprite.animations[SpriteAnimationType::WalkUp] = GetSpriteAnimation(animationManager, (animType + "WalkUp").c_str(), SpriteAnimationType::WalkUp);
    sprite.animations[SpriteAnimationType::WalkDown] = GetSpriteAnimation(animationManager, (animType + "WalkDown").c_str(), SpriteAnimationType::WalkDown);
    sprite.animations[SpriteAnimationType::WalkLeft] = GetSpriteAnimation(animationManager, (animType + "WalkLeft").c_str(), SpriteAnimationType::WalkLeft);
    sprite.animations[SpriteAnimationType::WalkRight] = GetSpriteAnimation(animationManager, (animType + "WalkRight").c_str(), SpriteAnimationType::WalkRight);
}

void PlaySpriteAnimation(SpriteAnimationPlayer &player, SpriteAnimation *animation, bool loop) {
    // do nothing if the animation is already playing
    if (player.animation == animation) {
        player.playing = true;
        return;
    }
    player.animation = animation;
    player.currentFrame = 0;
    player.frameTime = 0;
    player.loop = loop;
    player.playing = true;
}

SpriteAnimation *GetCharacterAnimation(CharacterSprite &sprite, SpriteAnimationType type) {
    return sprite.animations[type];
}

void SetFrame(SpriteAnimationPlayer &player, int frame) {
    if (frame >= 0 && frame < player.animation->frames.size()) {
        player.currentFrame = frame;
        player.frameTime = 0;
    }
}

void SetSpriteAnimPaused(CharacterSprite& sprite, SpriteAnimationType type) {
    PlaySpriteAnimation(sprite.player, GetCharacterAnimation(sprite, type), true);
    SetFrame(sprite.player, 0);
    sprite.player.playing = false;
}



