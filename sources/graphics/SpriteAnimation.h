//
// Created by bison on 15-01-25.
//

#ifndef SANDBOX_SPRITEANIMATION_H
#define SANDBOX_SPRITEANIMATION_H

#include <vector>
#include <map>
#include "raylib.h"
#include "SpriteSheet.h"
#include <string>
#include <unordered_map>

struct SpriteAnimation {
    std::string name;
    int spriteSheetIndex;
    std::vector<int> frames; // indexes into the sprite sheet
    std::vector<float> frameDelays; // time to display each frame
    Vector2 origin; // origin of the sprite in the frame
};

struct SpriteAnimationPlayer {
    SpriteAnimation* animation;
    int currentFrame;
    float frameTime;
    Vector2 position;
    Vector2 scale;
    float rotation;
    Color tint;
    bool loop; // if false, the animation stops at the last frame
    bool playing; // if false, the animation is paused
};

struct FrameInfo {
    Texture2D* texture;
    Rectangle srcRect;
};

void InitSpriteAnimationPlayer(SpriteAnimationPlayer& player);
void UpdateSpriteAnimation(SpriteAnimationPlayer& player, float dt);
void DrawSpriteAnimation(SpriteAnimationPlayer& player);
void DrawSpriteAnimation(SpriteAnimationPlayer& player, float x, float y);
FrameInfo GetFrameInfo(SpriteAnimationPlayer& player);
void PlaySpriteAnimation(SpriteAnimationPlayer& player, SpriteAnimation* animation, bool loop = true);
void PlaySpriteAnimationRestart(SpriteAnimationPlayer &player, SpriteAnimation *animation, bool loop);
void SetFrame(SpriteAnimationPlayer& player, int frame);

struct SpriteAnimationManager {
    std::vector<SpriteSheet> spriteSheets;
    std::unordered_map<std::string, SpriteAnimation> animations;
};

void InitSpriteAnimationManager(const std::string& filename);
void DestroySpriteAnimationManager();
//void CreateSpriteAnimation(const std::string& name, int spriteSheetIndex, std::vector<int> frames, std::vector<float> frameDelays, Vector2 origin);
SpriteAnimation* GetSpriteAnimation(const std::string& name);
SpriteSheet* GetSpriteSheet(int index);

#endif //SANDBOX_SPRITEANIMATION_H
