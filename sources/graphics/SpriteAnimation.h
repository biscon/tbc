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

struct FrameInfo {
    Texture2D texture;
    Rectangle srcRect;
};

struct SpriteAnimationData {
    std::vector<std::string> name;
    std::vector<int> spriteSheetIdx;
    std::vector<std::vector<int>> frames;
    std::vector<std::vector<float>> frameDelays;
    std::vector<Vector2> origin;

    std::unordered_map<std::string, int> nameIndexMap;
};

struct SpriteAnimationPlayerRenderData {
    Vector2 position;
    Vector2 scale;
    float rotation;
    Color tint;
};

struct SpriteAnimationPlayerAnimData {
    int currentFrame;
    float frameTime;
    bool loop; // if false, the animation stops at the last frame
    bool playing; // if false, the animation is paused
};

struct SpriteAnimationPlayerData {
    std::vector<int> animationIdx;
    std::vector<SpriteAnimationPlayerRenderData> renderData;
    std::vector<SpriteAnimationPlayerAnimData> animData;
};

struct SpriteData {
    SpriteSheetData sheet;
    SpriteAnimationData anim;
    SpriteAnimationPlayerData player;
};

int CreateSpriteAnimationPlayer(SpriteData& sprite);
void UpdateSpriteAnimation(SpriteData& sprite, int player, float dt);
void DrawSpriteAnimation(SpriteData& sprite, int player);
void DrawSpriteAnimation(SpriteData& sprite, int player, float x, float y);
FrameInfo GetFrameInfo(SpriteData& sprite, int player);
void PlaySpriteAnimation(SpriteData& sprite, int player, int animation, bool loop = true);
void PlaySpriteAnimationRestart(SpriteData& sprite, int player, int animation, bool loop);
void SetFrame(SpriteData& sprite, int player, int frame);


void InitSpriteAnimationData(SpriteData& spriteData, const std::string &filename);
void DestroySpriteAnimationData(SpriteData& spriteData);
int GetSpriteAnimation(SpriteData& spriteData, const std::string& name);

#endif //SANDBOX_SPRITEANIMATION_H
