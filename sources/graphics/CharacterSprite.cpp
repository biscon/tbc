//
// Created by bison on 26-01-25.
//

#include "CharacterSprite.h"

static void LoadAnimType(std::map<SpriteAnimationType, SpriteAnimation*>& animations, const std::string& animType, bool hasAttacks) {
    animations[SpriteAnimationType::WalkUp] = GetSpriteAnimation(animType + "WalkUp");
    animations[SpriteAnimationType::WalkDown] = GetSpriteAnimation(animType + "WalkDown");
    animations[SpriteAnimationType::WalkLeft] = GetSpriteAnimation(animType + "WalkLeft");
    animations[SpriteAnimationType::WalkRight] = GetSpriteAnimation(animType + "WalkRight");
    if(hasAttacks) {
        animations[SpriteAnimationType::AttackUp] = GetSpriteAnimation(animType + "AttackUp");
        animations[SpriteAnimationType::AttackDown] = GetSpriteAnimation(animType + "AttackDown");
        animations[SpriteAnimationType::AttackLeft] = GetSpriteAnimation(animType + "AttackLeft");
        animations[SpriteAnimationType::AttackRight] = GetSpriteAnimation(animType + "AttackRight");
    } else {
        animations[SpriteAnimationType::AttackUp] = animations[SpriteAnimationType::WalkUp];
        animations[SpriteAnimationType::AttackDown] = animations[SpriteAnimationType::WalkDown];
        animations[SpriteAnimationType::AttackLeft] = animations[SpriteAnimationType::WalkLeft];
        animations[SpriteAnimationType::AttackRight] = animations[SpriteAnimationType::WalkRight];
    }
}

void InitCharacterSprite(CharacterSprite &sprite, const std::string& bodyType, bool hasAttacks) {
    sprite.displayWeapon = false;
    LoadAnimType(sprite.bodyAnimations, bodyType, hasAttacks);
    /*
    if(sprite.displayWeapon) {
        LoadAnimType(sprite.weaponAnimations, weaponType, hasAttacks);
    }
     */
    InitSpriteAnimationPlayer(sprite.bodyPlayer);
    InitSpriteAnimationPlayer(sprite.weaponPlayer);
}

void SetCharacterSpriteWeaponAnimation(CharacterSprite &sprite, const std::string &weaponType) {
    sprite.displayWeapon = true;
    LoadAnimType(sprite.weaponAnimations, weaponType, true);
    InitSpriteAnimationPlayer(sprite.weaponPlayer);
}

SpriteAnimation *GetCharacterAnimationBody(CharacterSprite &sprite, SpriteAnimationType type) {
    return sprite.bodyAnimations[type];
}

SpriteAnimation *GetCharacterAnimationWeapon(CharacterSprite &sprite, SpriteAnimationType type) {
    return sprite.weaponAnimations[type];
}

Vector2 GetCharacterSpritePos(CharacterSprite &sprite) {
    return sprite.bodyPlayer.position;
}

Vector2i GetCharacterSpritePosI(CharacterSprite &sprite) {
    return Vector2i{(int)sprite.bodyPlayer.position.x, (int)sprite.bodyPlayer.position.y};
}

float GetCharacterSpritePosX(CharacterSprite &sprite) {
    return sprite.bodyPlayer.position.x;
}

float GetCharacterSpritePosY(CharacterSprite &sprite) {
    return sprite.bodyPlayer.position.y;
}

void SetCharacterSpritePos(CharacterSprite &sprite, Vector2 pos) {
    sprite.bodyPlayer.position = pos;
    sprite.weaponPlayer.position = pos;
}

void SetCharacterSpritePosI(CharacterSprite &sprite, Vector2i pos) {
    sprite.bodyPlayer.position = Vector2{(float)pos.x, (float)pos.y};
    sprite.weaponPlayer.position = Vector2{(float)pos.x, (float)pos.y};
}

void SetCharacterSpritePosX(CharacterSprite &sprite, float x) {
    sprite.bodyPlayer.position.x = x;
    sprite.weaponPlayer.position.x = x;
}

void SetCharacterSpritePosY(CharacterSprite &sprite, float y) {
    sprite.bodyPlayer.position.y = y;
    sprite.weaponPlayer.position.y = y;
}

void SetCharacterSpriteRotation(CharacterSprite &sprite, float rotation) {
    sprite.bodyPlayer.rotation = rotation;
    sprite.weaponPlayer.rotation = rotation;
}

float GetCharacterSpriteRotation(CharacterSprite &sprite) {
    return sprite.bodyPlayer.rotation;
}

void PlayCharacterSpriteAnim(CharacterSprite &sprite, SpriteAnimationType type, bool loop) {
    PlaySpriteAnimation(sprite.bodyPlayer, GetCharacterAnimationBody(sprite, type), loop);
    if(sprite.displayWeapon)
        PlaySpriteAnimation(sprite.weaponPlayer, GetCharacterAnimationWeapon(sprite, type), loop);
}

void PauseCharacterSpriteAnim(CharacterSprite &sprite) {
    sprite.bodyPlayer.playing = false;
    sprite.weaponPlayer.playing = false;
}

void StartPausedCharacterSpriteAnim(CharacterSprite &sprite, SpriteAnimationType type, bool loop) {
    PlaySpriteAnimation(sprite.bodyPlayer, GetCharacterAnimationBody(sprite, type), loop);
    SetFrame(sprite.bodyPlayer, 0);
    sprite.bodyPlayer.playing = false;

    if(sprite.displayWeapon) {
        PlaySpriteAnimation(sprite.weaponPlayer, GetCharacterAnimationWeapon(sprite, type), loop);
        SetFrame(sprite.weaponPlayer, 0);
        sprite.weaponPlayer.playing = false;
    }
}

void DrawCharacterSprite(CharacterSprite &sprite) {
    DrawSpriteAnimation(sprite.bodyPlayer);
    if(sprite.displayWeapon)
        DrawSpriteAnimation(sprite.weaponPlayer);
}

void DrawCharacterSprite(CharacterSprite &sprite, float x, float y) {
    DrawSpriteAnimation(sprite.bodyPlayer, x, y);
    if(sprite.displayWeapon)
        DrawSpriteAnimation(sprite.weaponPlayer, x, y);
}

void SetCharacterSpriteTint(CharacterSprite &sprite, Color tint) {
    sprite.bodyPlayer.tint = tint;
    sprite.weaponPlayer.tint = tint;
}

Color GetCharacterSpriteTint(CharacterSprite &sprite) {
    return sprite.bodyPlayer.tint;
}

void SetCharacterSpriteFrame(CharacterSprite &sprite, int frame) {
    SetFrame(sprite.bodyPlayer, frame);
    if(sprite.displayWeapon)
        SetFrame(sprite.weaponPlayer, frame);
}

void UpdateCharacterSprite(CharacterSprite &sprite, float deltaTime) {
    UpdateSpriteAnimation(sprite.bodyPlayer, deltaTime);
    UpdateSpriteAnimation(sprite.weaponPlayer, deltaTime);
}

void PlayCharacterSpriteAnimRestart(CharacterSprite &sprite, SpriteAnimationType type, bool loop) {
    PlaySpriteAnimationRestart(sprite.bodyPlayer, GetCharacterAnimationBody(sprite, type), loop);
    if(sprite.displayWeapon)
        PlaySpriteAnimationRestart(sprite.weaponPlayer, GetCharacterAnimationWeapon(sprite, type), loop);
}
