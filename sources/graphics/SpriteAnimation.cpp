//
// Created by bison on 15-01-25.
//

#include <cstring>
#include <utility>
#include "SpriteAnimation.h"
#include "SpriteSheet.h"
#include <fstream>
#include "../util/json.hpp"
using json = nlohmann::json;

static SpriteAnimationManager manager;

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
    if (!player.animation || player.animation->frames.empty()) {
        return;
    }

    // Get the current frame index
    int frameIndex = player.animation->frames[player.currentFrame];

    // Get the source rectangle from the sprite sheet
    const SpriteSheet* spriteSheet = GetSpriteSheet(player.animation->spriteSheetIndex);
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
    if (!player.animation || player.animation->frames.empty()) {
        return;
    }

    // Get the current frame index
    int frameIndex = player.animation->frames[player.currentFrame];

    // Get the source rectangle from the sprite sheet
    const SpriteSheet* spriteSheet = GetSpriteSheet(player.animation->spriteSheetIndex);
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

void PlaySpriteAnimationRestart(SpriteAnimationPlayer &player, SpriteAnimation *animation, bool loop) {
    player.animation = animation;
    player.currentFrame = 0;
    player.frameTime = 0;
    player.loop = loop;
    player.playing = true;
}

void SetFrame(SpriteAnimationPlayer &player, int frame) {
    if (frame >= 0 && frame < player.animation->frames.size()) {
        player.currentFrame = frame;
        player.frameTime = 0;
    }
}

static void CreateSpriteAnimation(const std::string& name, int spriteSheetIndex,
                                  std::vector<int> frames, std::vector<float> frameDelays, Vector2 origin) {
    SpriteAnimation animation;
    animation.name = name;
    animation.spriteSheetIndex = spriteSheetIndex;
    animation.frames = std::move(frames);
    animation.frameDelays = std::move(frameDelays);
    animation.origin = origin;
    manager.animations[name] = animation;
}

SpriteAnimation *GetSpriteAnimation(const std::string& name) {
    if(manager.animations.find(name) != manager.animations.end()) {
        return &manager.animations[name];
    } else {
        TraceLog(LOG_ERROR, "GetSpriteAnimation: Animation not found: %s", name.c_str());
    }
    return nullptr;
}

void InitSpriteAnimationManager(const std::string &filename) {
    std::ifstream file(filename);
    json j;
    file >> j;
    for(auto& e : j) {
        std::string group = e["group"].get<std::string>();
        std::string sheetFilename = ASSETS_PATH"" + e["sheet"].get<std::string>();
        TraceLog(LOG_INFO, "SpriteAnimationManager: Loading sprite sheet: %s", sheetFilename.c_str());
        SpriteSheet sheet;
        LoadSpriteSheet(sheet, sheetFilename.c_str(), e["frameWidth"].get<int>(), e["frameHeight"].get<int>());
        for(auto& jAnim : e["animations"]) {
            std::string name = group + jAnim["name"].get<std::string>();
            //SpriteAnimationType type = static_cast<SpriteAnimationType>(jAnim["type"].get<int>());
            std::vector<int> frames;
            std::vector<float> frameDelays;
            for(auto& jFrame : jAnim["frames"]) {
                frames.push_back(jFrame.get<int>());
            }
            for(auto& jDelay : jAnim["delays"]) {
                frameDelays.push_back(jDelay.get<float>());
            }
            Vector2 origin = {jAnim["origin"][0].get<float>(), jAnim["origin"][1].get<float>()};
            TraceLog(LOG_INFO, "SpriteAnimationManager: Creating animation: %s", name.c_str());
            int sheetIndex = (int) manager.spriteSheets.size();
            CreateSpriteAnimation(name, sheetIndex, frames, frameDelays, origin);
        }
        manager.spriteSheets.push_back(sheet);
    }
}

void DestroySpriteAnimationManager() {
    for(auto& sheet : manager.spriteSheets) {
        UnloadSpriteSheet(sheet);
    }
    manager.spriteSheets.clear();
    manager.animations.clear();
}

SpriteSheet *GetSpriteSheet(int index) {
    if(index >= 0 && index < manager.spriteSheets.size()) {
        return &manager.spriteSheets[index];
    }
    return nullptr;
}






