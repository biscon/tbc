//
// Created by bison on 15-01-25.
//

#ifndef SANDBOX_SPRITEANIMATION_H
#define SANDBOX_SPRITEANIMATION_H

#include <vector>
#include <map>
#include "raylib.h"
#include "data/SpriteData.h"
#include <string>
#include <unordered_map>

int CreateSpriteAnimationPlayer(SpriteData& sprite);
void UpdateSpriteAnimation(SpriteData& sprite, int player, float dt);
void DrawSpriteAnimation(SpriteData& sprite, int player);
void DrawSpriteAnimation(SpriteData& sprite, int player, float x, float y);
void DrawSpriteAnimationColors(SpriteData& sprite, int player, float x, float y, Color c1, Color c2, Color c3, Color c4);
FrameInfo GetFrameInfo(SpriteData& sprite, int player);
void PlaySpriteAnimation(SpriteData& sprite, int player, int animation, bool loop = true);
void PlaySpriteAnimationRestart(SpriteData& sprite, int player, int animation, bool loop);
void SetFrame(SpriteData& sprite, int player, int frame);


void InitSpriteAnimationData(SpriteData& spriteData, const std::string &filename);
void DestroySpriteAnimationData(SpriteData& spriteData);
int GetSpriteAnimation(SpriteData& spriteData, const std::string& name);

#endif //SANDBOX_SPRITEANIMATION_H
