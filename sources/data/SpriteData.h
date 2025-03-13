//
// Created by bison on 13-03-25.
//

#ifndef SANDBOX_SPRITEDATA_H
#define SANDBOX_SPRITEDATA_H

#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include "raylib.h"

enum class SpriteAnimationType {
    Idle,
    WalkUp,
    WalkDown,
    WalkLeft,
    WalkRight,
    AttackUp,
    AttackDown,
    AttackLeft,
    AttackRight,
};

struct CharacterSprite {
    std::map<SpriteAnimationType, int> bodyAnimations;
    std::map<SpriteAnimationType, int> weaponAnimations;
    int bodyPlayer;
    int weaponPlayer;
    bool displayWeapon;
};

struct SpriteSheetFrameSizeData {
    int frameWidth;
    int frameHeight;
};

struct SpriteSheetData {
    std::vector<Texture2D> texture;
    std::vector<SpriteSheetFrameSizeData> frameSizeData;
    std::vector<std::vector<Rectangle>> frameRects;
    std::vector<bool> loaded;
};

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

#endif //SANDBOX_SPRITEDATA_H
