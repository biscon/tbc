//
// Created by bison on 13-03-25.
//

#ifndef SANDBOX_SPRITEDATA_H
#define SANDBOX_SPRITEDATA_H

#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <array>
#include "raylib.h"
#include "WeaponData.h"

constexpr int INVALID_ANIM = -1;

enum class CharAnimationType {
    Idle,
    MeleeIdle,
    MeleeHit,
    MeleeAttack,
    PistolIdle,
    PistolHit,
    PistolAttack,
    COUNT
};

enum class CharOrientation {
    Left, Right
};

enum class CharAnimationLayerType {
    Naked,
    Outfit,
    Hair,
    Weapon,
    COUNT
};


struct CharAnimationLayer {
    std::array<int, (size_t)CharAnimationType::COUNT> animations =
            []{
                std::array<int, (size_t)CharAnimationType::COUNT> a{};
                a.fill(INVALID_ANIM);
                return a;
            }();
    int player = -1;
    bool hide = false;
};

struct ColorPair {
    Vector3 c1; // darker / shadow
    Vector3 c2; // lighter / highlight
};

struct SkinPreset {
    const char* name;
    ColorPair colors;
};

struct HairPreset {
    const char* name;
    ColorPair colors;
};

struct OutfitColors {
    Vector3 lowerDark;
    Vector3 lowerLight;
    Vector3 upperDark;
    Vector3 upperLight;
};

struct OutfitPreset {
    const char* name;
    OutfitColors colors;
};

struct CharSprite {
    std::string spriteTemplate;
    CharOrientation orientation = CharOrientation::Right;
    std::array<CharAnimationLayer,(size_t) CharAnimationLayerType::COUNT> layers;
    Vector3 skinColor1;
    Vector3 skinColor2;
    Vector3 hairColor1;
    Vector3 hairColor2;
    Vector3 outfitColor1;
    Vector3 outfitColor2;
    Vector3 outfitColor3;
    Vector3 outfitColor4;
    CharAnimationType currentAnim;
    WeaponAnimType weaponType;
};


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
    std::string spriteTemplate;
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
    bool flipX;
};

struct SpriteAnimationPlayerAnimData {
    int currentFrame;
    float frameTime;
    bool loop; // if false, the animation stops at the last frame
    bool playing; // if false, the animation is paused
    bool reverse;
};

struct SpriteAnimationPlayerData {
    std::vector<int> animationIdx;
    std::vector<SpriteAnimationPlayerRenderData> renderData;
    std::vector<SpriteAnimationPlayerAnimData> animData;
};

struct CharShaderData {
    Shader shader;
    // source colors, never change
    int locSkinSrc1;
    int locSkinSrc2;
    int locHairSrc1;
    int locHairSrc2;
    int locOutfitSrc1;
    int locOutfitSrc2;
    int locOutfitSrc3;
    int locOutfitSrc4;

    // dst colors
    int locSkinDst1;
    int locSkinDst2;
    int locHairDst1;
    int locHairDst2;
    int locOutfitDst1;
    int locOutfitDst2;
    int locOutfitDst3;
    int locOutfitDst4;

    int locTolerance;
};

struct SpriteData {
    SpriteSheetData sheet;
    SpriteAnimationData anim;
    SpriteAnimationPlayerData player;
    CharShaderData charShader;
};

#endif //SANDBOX_SPRITEDATA_H
