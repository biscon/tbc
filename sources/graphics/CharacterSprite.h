//
// Created by bison on 26-01-25.
//

#ifndef SANDBOX_CHARACTERSPRITE_H
#define SANDBOX_CHARACTERSPRITE_H

#include "SpriteAnimation.h"
#include "util/MathUtil.h"

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
    std::map<SpriteAnimationType, SpriteAnimation*> bodyAnimations;
    std::map<SpriteAnimationType, SpriteAnimation*> weaponAnimations;
    SpriteAnimationPlayer bodyPlayer;
    SpriteAnimationPlayer weaponPlayer;
    bool displayWeapon;
};

void InitCharacterSprite(CharacterSprite &sprite, const std::string& bodyType, bool hasAttacks);
void SetCharacterSpriteWeaponAnimation(CharacterSprite &sprite, const std::string &weaponType);
SpriteAnimation* GetCharacterAnimationBody(CharacterSprite& sprite, SpriteAnimationType type);
SpriteAnimation* GetCharacterAnimationWeapon(CharacterSprite& sprite, SpriteAnimationType type);
Vector2 GetCharacterSpritePos(CharacterSprite& sprite);
Vector2i GetCharacterSpritePosI(CharacterSprite& sprite);
Vector2i GetCharacterGridPosI(CharacterSprite &sprite);
float GetCharacterSpritePosX(CharacterSprite& sprite);
float GetCharacterSpritePosY(CharacterSprite& sprite);
void SetCharacterSpritePos(CharacterSprite& sprite, Vector2 pos);
void SetCharacterSpritePosI(CharacterSprite& sprite, Vector2i pos);
void SetCharacterSpritePosX(CharacterSprite& sprite, float x);
void SetCharacterSpritePosY(CharacterSprite& sprite, float y);
void SetCharacterSpriteRotation(CharacterSprite& sprite, float rotation);
float GetCharacterSpriteRotation(CharacterSprite& sprite);
void PlayCharacterSpriteAnim(CharacterSprite& sprite, SpriteAnimationType type, bool loop);
void PlayCharacterSpriteAnimRestart(CharacterSprite& sprite, SpriteAnimationType type, bool loop);
void PauseCharacterSpriteAnim(CharacterSprite& sprite);
void StartPausedCharacterSpriteAnim(CharacterSprite& sprite, SpriteAnimationType type, bool loop);
void DrawCharacterSprite(CharacterSprite& sprite);
void DrawCharacterSprite(CharacterSprite& sprite, float x, float y);
void SetCharacterSpriteTint(CharacterSprite& sprite, Color tint);
Color GetCharacterSpriteTint(CharacterSprite& sprite);
void SetCharacterSpriteFrame(CharacterSprite& sprite, int frame);
void UpdateCharacterSprite(CharacterSprite& sprite, float deltaTime);



#endif //SANDBOX_CHARACTERSPRITE_H
