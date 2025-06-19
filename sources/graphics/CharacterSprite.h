//
// Created by bison on 26-01-25.
//

#ifndef SANDBOX_CHARACTERSPRITE_H
#define SANDBOX_CHARACTERSPRITE_H

#include "SpriteAnimation.h"
#include "util/MathUtil.h"
#include "data/SpriteData.h"


void InitCharacterSprite(SpriteData& spriteData, CharacterSprite &sprite, const std::string& spriteTemplate, bool hasAttacks);
void SetCharacterSpriteWeaponAnimation(SpriteData& spriteData, CharacterSprite &sprite, const std::string &weaponType);
int GetCharacterAnimationBody(CharacterSprite& sprite, SpriteAnimationType type);
int GetCharacterAnimationWeapon(CharacterSprite& sprite, SpriteAnimationType type);
Vector2 GetCharacterSpritePos(SpriteData& spriteData, CharacterSprite &sprite);
Vector2i GetCharacterSpritePosI(SpriteData& spriteData, CharacterSprite& sprite);
Vector2i GetCharacterGridPosI(SpriteData& spriteData, CharacterSprite &sprite);
float GetCharacterSpritePosX(SpriteData& spriteData, CharacterSprite& sprite);
float GetCharacterSpritePosY(SpriteData& spriteData, CharacterSprite& sprite);
void SetCharacterSpritePos(SpriteData& spriteData, CharacterSprite& sprite, Vector2 pos);
void SetCharacterSpritePosI(SpriteData& spriteData, CharacterSprite& sprite, Vector2i pos);
void SetCharacterSpritePosX(SpriteData& spriteData, CharacterSprite& sprite, float x);
void SetCharacterSpritePosY(SpriteData& spriteData, CharacterSprite& sprite, float y);
void SetCharacterSpriteRotation(SpriteData& spriteData, CharacterSprite& sprite, float rotation);
float GetCharacterSpriteRotation(SpriteData& spriteData, CharacterSprite& sprite);
void PlayCharacterSpriteAnim(SpriteData& spriteData, CharacterSprite& sprite, SpriteAnimationType type, bool loop);
void PlayCharacterSpriteAnimRestart(SpriteData& spriteData, CharacterSprite& sprite, SpriteAnimationType type, bool loop);
void PauseCharacterSpriteAnim(SpriteData& spriteData, CharacterSprite& sprite);
void StartPausedCharacterSpriteAnim(SpriteData& spriteData, CharacterSprite& sprite, SpriteAnimationType type, bool loop);
void DrawCharacterSprite(SpriteData& spriteData, CharacterSprite& sprite);
void DrawCharacterSprite(SpriteData& spriteData, CharacterSprite& sprite, float x, float y);
void SetCharacterSpriteTint(SpriteData& spriteData, CharacterSprite& sprite, Color tint);
Color GetCharacterSpriteTint(SpriteData& spriteData, CharacterSprite& sprite);
void SetCharacterSpriteFrame(SpriteData& spriteData, CharacterSprite& sprite, int frame);
void UpdateCharacterSprite(SpriteData& spriteData, CharacterSprite& sprite, float deltaTime);

void DrawCharacterSpriteColors(SpriteData& spriteData, CharacterSprite &sprite, float x, float y, Color c1, Color c2, Color c3, Color c4);



#endif //SANDBOX_CHARACTERSPRITE_H
