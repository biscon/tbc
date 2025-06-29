//
// Created by bison on 26-01-25.
//

#include <cmath>
#include "CharacterSprite.h"
#include "ai/PathFinding.h"

static void LoadAnimType(SpriteData& spriteData, std::map<SpriteAnimationType, int>& animations, const std::string& animType, bool hasAttacks) {
    animations[SpriteAnimationType::WalkUp] = GetSpriteAnimation(spriteData, animType + "WalkUp");
    animations[SpriteAnimationType::WalkDown] = GetSpriteAnimation(spriteData, animType + "WalkDown");
    animations[SpriteAnimationType::WalkLeft] = GetSpriteAnimation(spriteData, animType + "WalkLeft");
    animations[SpriteAnimationType::WalkRight] = GetSpriteAnimation(spriteData, animType + "WalkRight");
    if(hasAttacks) {
        animations[SpriteAnimationType::AttackUp] = GetSpriteAnimation(spriteData, animType + "AttackUp");
        animations[SpriteAnimationType::AttackDown] = GetSpriteAnimation(spriteData, animType + "AttackDown");
        animations[SpriteAnimationType::AttackLeft] = GetSpriteAnimation(spriteData, animType + "AttackLeft");
        animations[SpriteAnimationType::AttackRight] = GetSpriteAnimation(spriteData, animType + "AttackRight");
    } else {
        animations[SpriteAnimationType::AttackUp] = animations[SpriteAnimationType::WalkUp];
        animations[SpriteAnimationType::AttackDown] = animations[SpriteAnimationType::WalkDown];
        animations[SpriteAnimationType::AttackLeft] = animations[SpriteAnimationType::WalkLeft];
        animations[SpriteAnimationType::AttackRight] = animations[SpriteAnimationType::WalkRight];
    }
}

void InitCharacterSprite(SpriteData& spriteData, CharacterSprite &sprite, const std::string& spriteTemplate, bool hasAttacks) {
    sprite.spriteTemplate = spriteTemplate;
    sprite.displayWeapon = false;
    LoadAnimType(spriteData, sprite.bodyAnimations, spriteTemplate, hasAttacks);
    /*
    if(sprite.displayWeapon) {
        LoadAnimType(sprite.weaponAnimations, weaponType, hasAttacks);
    }
     */
    sprite.bodyPlayer = CreateSpriteAnimationPlayer(spriteData);
    sprite.weaponPlayer = CreateSpriteAnimationPlayer(spriteData);
}

void SetCharacterSpriteWeaponAnimation(SpriteData& spriteData, CharacterSprite &sprite, const std::string &weaponType) {
    sprite.displayWeapon = true;
    LoadAnimType(spriteData, sprite.weaponAnimations, weaponType, true);
    sprite.weaponPlayer = CreateSpriteAnimationPlayer(spriteData);
}

int GetCharacterAnimationBody(CharacterSprite &sprite, SpriteAnimationType type) {
    return sprite.bodyAnimations[type];
}

int GetCharacterAnimationWeapon(CharacterSprite &sprite, SpriteAnimationType type) {
    return sprite.weaponAnimations[type];
}

Vector2 GetCharacterSpritePos(SpriteData& spriteData, CharacterSprite &sprite) {
    return spriteData.player.renderData[sprite.bodyPlayer].position;
}

Vector2i GetCharacterSpritePosI(SpriteData& spriteData, CharacterSprite &sprite) {
    SpriteAnimationPlayerRenderData& renderData = spriteData.player.renderData[sprite.bodyPlayer];
    return Vector2i{(int) renderData.position.x, (int) renderData.position.y};
}

Vector2i GetCharacterGridPosI(SpriteData& spriteData, CharacterSprite &sprite) {
    SpriteAnimationPlayerRenderData& renderData = spriteData.player.renderData[sprite.bodyPlayer];
    return PixelToGridPositionI((int) renderData.position.x, (int) renderData.position.y);
}

void SetCharacterGridPosI(SpriteData& spriteData, CharacterSprite& sprite, Vector2i pos) {
    Vector2 spritePos = GridToPixelPosition(pos.x, pos.y);
    SetCharacterSpritePos(spriteData, sprite, spritePos);
}

float GetCharacterSpritePosX(SpriteData& spriteData, CharacterSprite &sprite) {
    SpriteAnimationPlayerRenderData& renderData = spriteData.player.renderData[sprite.bodyPlayer];
    return renderData.position.x;
}

float GetCharacterSpritePosY(SpriteData& spriteData, CharacterSprite &sprite) {
    SpriteAnimationPlayerRenderData& renderData = spriteData.player.renderData[sprite.bodyPlayer];
    return renderData.position.y;
}

void SetCharacterSpritePos(SpriteData& spriteData, CharacterSprite &sprite, Vector2 pos) {
    SpriteAnimationPlayerRenderData& bodyRenderData = spriteData.player.renderData[sprite.bodyPlayer];
    SpriteAnimationPlayerRenderData& weaponRenderData = spriteData.player.renderData[sprite.weaponPlayer];
    bodyRenderData.position = pos;
    weaponRenderData.position = pos;
}

void SetCharacterSpritePosI(SpriteData& spriteData, CharacterSprite &sprite, Vector2i pos) {
    SpriteAnimationPlayerRenderData& bodyRenderData = spriteData.player.renderData[sprite.bodyPlayer];
    SpriteAnimationPlayerRenderData& weaponRenderData = spriteData.player.renderData[sprite.weaponPlayer];
    bodyRenderData.position = Vector2{(float)pos.x, (float)pos.y};
    weaponRenderData.position = Vector2{(float)pos.x, (float)pos.y};
}

void SetCharacterSpritePosX(SpriteData& spriteData, CharacterSprite &sprite, float x) {
    SpriteAnimationPlayerRenderData& bodyRenderData = spriteData.player.renderData[sprite.bodyPlayer];
    SpriteAnimationPlayerRenderData& weaponRenderData = spriteData.player.renderData[sprite.weaponPlayer];
    bodyRenderData.position.x = x;
    weaponRenderData.position.x = x;
}

void SetCharacterSpritePosY(SpriteData& spriteData, CharacterSprite &sprite, float y) {
    SpriteAnimationPlayerRenderData& bodyRenderData = spriteData.player.renderData[sprite.bodyPlayer];
    SpriteAnimationPlayerRenderData& weaponRenderData = spriteData.player.renderData[sprite.weaponPlayer];
    bodyRenderData.position.y = y;
    weaponRenderData.position.y = y;
}

void SetCharacterSpriteRotation(SpriteData& spriteData, CharacterSprite &sprite, float rotation) {
    SpriteAnimationPlayerRenderData& bodyRenderData = spriteData.player.renderData[sprite.bodyPlayer];
    SpriteAnimationPlayerRenderData& weaponRenderData = spriteData.player.renderData[sprite.weaponPlayer];
    bodyRenderData.rotation = rotation;
    weaponRenderData.rotation = rotation;
}

float GetCharacterSpriteRotation(SpriteData& spriteData, CharacterSprite &sprite) {
    SpriteAnimationPlayerRenderData& bodyRenderData = spriteData.player.renderData[sprite.bodyPlayer];
    return bodyRenderData.rotation;
}

void PlayCharacterSpriteAnim(SpriteData& spriteData, CharacterSprite &sprite, SpriteAnimationType type, bool loop) {
    PlaySpriteAnimation(spriteData, sprite.bodyPlayer, GetCharacterAnimationBody(sprite, type), loop);
    if(sprite.displayWeapon)
        PlaySpriteAnimation(spriteData, sprite.weaponPlayer, GetCharacterAnimationWeapon(sprite, type), loop);
}

void PauseCharacterSpriteAnim(SpriteData& spriteData, CharacterSprite &sprite) {
    SpriteAnimationPlayerAnimData& bodyAnimData = spriteData.player.animData[sprite.bodyPlayer];
    SpriteAnimationPlayerAnimData& weaponAnimData = spriteData.player.animData[sprite.weaponPlayer];
    bodyAnimData.playing = false;
    weaponAnimData.playing = false;
}

void StartPausedCharacterSpriteAnim(SpriteData& spriteData, CharacterSprite &sprite, SpriteAnimationType type, bool loop) {
    PlaySpriteAnimation(spriteData, sprite.bodyPlayer, GetCharacterAnimationBody(sprite, type), loop);
    SetFrame(spriteData, sprite.bodyPlayer, 0);

    SpriteAnimationPlayerAnimData& bodyAnimData = spriteData.player.animData[sprite.bodyPlayer];
    SpriteAnimationPlayerAnimData& weaponAnimData = spriteData.player.animData[sprite.weaponPlayer];

    bodyAnimData.playing = false;

    if(sprite.displayWeapon) {
        PlaySpriteAnimation(spriteData, sprite.weaponPlayer, GetCharacterAnimationWeapon(sprite, type), loop);
        SetFrame(spriteData, sprite.weaponPlayer, 0);
        weaponAnimData.playing = false;
    }
}

void DrawCharacterSprite(SpriteData& spriteData, CharacterSprite &sprite) {
    DrawSpriteAnimation(spriteData, sprite.bodyPlayer);
    if(sprite.displayWeapon)
        DrawSpriteAnimation(spriteData, sprite.weaponPlayer);
}

void DrawCharacterSprite(SpriteData& spriteData, CharacterSprite &sprite, float x, float y) {
    DrawSpriteAnimation(spriteData, sprite.bodyPlayer, x, y);
    if(sprite.displayWeapon)
        DrawSpriteAnimation(spriteData, sprite.weaponPlayer, x, y);
}

void DrawCharacterSpriteScaled(SpriteData& spriteData, CharacterSprite &sprite, float x, float y, float scale) {
    DrawSpriteAnimationScaled(spriteData, sprite.bodyPlayer, x, y, scale);
    if(sprite.displayWeapon)
        DrawSpriteAnimationScaled(spriteData, sprite.weaponPlayer, x, y, scale);
}

void DrawCharacterSpriteColors(SpriteData& spriteData, CharacterSprite &sprite, float x, float y, Color c1, Color c2, Color c3, Color c4) {
    DrawSpriteAnimationColors(spriteData, sprite.bodyPlayer, x, y, c1, c2, c3, c4);
    if(sprite.displayWeapon)
        DrawSpriteAnimationColors(spriteData, sprite.weaponPlayer, x, y, c1, c2, c3, c4);
}


void SetCharacterSpriteTint(SpriteData& spriteData, CharacterSprite &sprite, Color tint) {
    SpriteAnimationPlayerRenderData& bodyRenderData = spriteData.player.renderData[sprite.bodyPlayer];
    SpriteAnimationPlayerRenderData& weaponRenderData = spriteData.player.renderData[sprite.weaponPlayer];
    bodyRenderData.tint = tint;
    weaponRenderData.tint = tint;
}

Color GetCharacterSpriteTint(SpriteData& spriteData, CharacterSprite &sprite) {
    SpriteAnimationPlayerRenderData& bodyRenderData = spriteData.player.renderData[sprite.bodyPlayer];
    return bodyRenderData.tint;
}

void SetCharacterSpriteFrame(SpriteData& spriteData, CharacterSprite &sprite, int frame) {
    SetFrame(spriteData, sprite.bodyPlayer, frame);
    if(sprite.displayWeapon)
        SetFrame(spriteData, sprite.weaponPlayer, frame);
}

void UpdateCharacterSprite(SpriteData& spriteData, CharacterSprite &sprite, float deltaTime) {
    UpdateSpriteAnimation(spriteData, sprite.bodyPlayer, deltaTime);
    UpdateSpriteAnimation(spriteData, sprite.weaponPlayer, deltaTime);
}

void PlayCharacterSpriteAnimRestart(SpriteData& spriteData, CharacterSprite &sprite, SpriteAnimationType type, bool loop) {
    PlaySpriteAnimationRestart(spriteData, sprite.bodyPlayer, GetCharacterAnimationBody(sprite, type), loop);
    if(sprite.displayWeapon)
        PlaySpriteAnimationRestart(spriteData, sprite.weaponPlayer, GetCharacterAnimationWeapon(sprite, type), loop);
}
