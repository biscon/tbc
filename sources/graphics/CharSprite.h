//
// Created by bison on 26-12-25.
//

#ifndef SANDBOX_CHARSPRITE_H
#define SANDBOX_CHARSPRITE_H

#include <string>
#include "data/SpriteData.h"
#include "util/MathUtil.h"

void InitCharShader(SpriteData& spriteData);
void InitCharSprite(SpriteData& spriteData, CharSprite &sprite, const std::string& spriteTemplate);
void UpdateCharSprite(SpriteData& spriteData, CharSprite& sprite, float deltaTime);
void DrawCharSpriteColors(SpriteData& spriteData, CharSprite &sprite, Color c1, Color c2, Color c3, Color c4);
void PlayCharSpriteAnim(SpriteData& spriteData, CharSprite &sprite, CharAnimationType type, bool loop);

Vector2 GetCharSpritePos(SpriteData& spriteData, CharSprite &sprite);
Vector2i GetCharSpritePosI(SpriteData& spriteData, CharSprite& sprite);
Vector2i GetCharGridPosI(SpriteData& spriteData, CharSprite &sprite);

void SetCharSpritePos(SpriteData& spriteData, CharSprite& sprite, Vector2 pos);
void SetCharSpritePosI(SpriteData& spriteData, CharSprite& sprite, Vector2i pos);
void SetCharGridPosI(SpriteData& spriteData, CharSprite& sprite, Vector2i pos);

void SetCharSpriteScale(SpriteData& spriteData, CharSprite &sprite, float scale);

void SetSkinColorPreset(CharSprite& sprite, size_t presetIndex);
void SetHairColorPreset(CharSprite& sprite, size_t presetIndex);
void SetOutfitColorPreset(CharSprite& sprite, size_t presetIndex);
void SetHairStyle(SpriteData& spriteData, CharSprite& sprite, size_t presetIndex);
void RandomizeCharAppearance(SpriteData& spriteData, CharSprite& sprite);

void SetCharWeaponType(SpriteData& spriteData, CharSprite& sprite, const std::string& type);

#endif //SANDBOX_CHARSPRITE_H
