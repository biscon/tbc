//
// Created by bison on 15-01-25.
//

#ifndef SANDBOX_SPRITE_H
#define SANDBOX_SPRITE_H

#include <vector>
#include <map>
#include "raylib.h"
#include <string>

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

struct SpriteSheet {
    Texture2D texture;
    int frameWidth;
    int frameHeight;
    std::vector<Rectangle> frameRects;
};

// Load a sprite sheet from a file and split it into frames
void LoadSpriteSheet(SpriteSheet& spriteSheet, const char* filename, int frameWidth, int frameHeight);
void UnloadSpriteSheet(SpriteSheet& spriteSheet);

struct SpriteAnimation {
    char name[64];
    SpriteAnimationType type;
    SpriteSheet* spriteSheet;
    std::vector<int> frames; // indexes into the sprite sheet
    std::vector<float> frameDelays; // time to display each frame
    Vector2 origin; // origin of the sprite in the frame
};

struct SpriteAnimationManager {
    std::vector<SpriteAnimation> animations;
};

void CreateSpriteAnimation(SpriteAnimationManager& manager, const char* name, SpriteAnimationType type, SpriteSheet* spriteSheet, std::vector<int> frames, std::vector<float> frameDelays, Vector2 origin);
SpriteAnimation* GetSpriteAnimation(SpriteAnimationManager& manager, const char* name, SpriteAnimationType type);
void UpdateSpriteAnimations(SpriteAnimationManager &manager, float dt);

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

void InitSpriteAnimationPlayer(SpriteAnimationPlayer& player);
void UpdateSpriteAnimation(SpriteAnimationPlayer& player, float dt);
void DrawSpriteAnimation(SpriteAnimationPlayer& player);
void DrawSpriteAnimation(SpriteAnimationPlayer& player, float x, float y);
void PlaySpriteAnimation(SpriteAnimationPlayer& player, SpriteAnimation* animation, bool loop = true);
void SetFrame(SpriteAnimationPlayer& player, int frame);

struct CharacterSprite {
    std::map<SpriteAnimationType, SpriteAnimation*> animations;
    SpriteAnimationPlayer player;
};

void InitCharacterSprite(CharacterSprite& sprite, SpriteAnimationManager& animationManager, const char* walkUp, const char* walkDown, const char* walkLeft, const char* walkRight);
void InitCharacterSprite(CharacterSprite &sprite, SpriteAnimationManager &animationManager, const std::string& animType);
SpriteAnimation* GetCharacterAnimation(CharacterSprite& sprite, SpriteAnimationType type);
void SetSpriteAnimPaused(CharacterSprite& sprite, SpriteAnimationType type);

#endif //SANDBOX_SPRITE_H
