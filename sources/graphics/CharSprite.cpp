//
// Created by bison on 26-12-25.
//

#include "CharSprite.h"
#include "SpriteAnimation.h"

static void InitAnims(SpriteData& spriteData, const std::string& spriteTemplate, CharAnimationLayer& layer, const std::string& layerName, CharAnimationType animType, const std::string& animName) {
    layer.animations[(size_t) animType] = GetSpriteAnimation(spriteData, spriteTemplate + "_" + animName + "_" + layerName);
}

static void InitLayer(SpriteData& spriteData, const std::string& spriteTemplate, CharAnimationLayer& layer, const std::string& layerName) {
    InitAnims(spriteData, spriteTemplate, layer, layerName, CharAnimationType::Idle, "Idle");
    InitAnims(spriteData, spriteTemplate, layer, layerName, CharAnimationType::MeleeIdle, "MeleeIdle");
    InitAnims(spriteData, spriteTemplate, layer, layerName, CharAnimationType::MeleeHit, "MeleeHit");
    InitAnims(spriteData, spriteTemplate, layer, layerName, CharAnimationType::MeleeAttack, "MeleeSwing");
    InitAnims(spriteData, spriteTemplate, layer, layerName, CharAnimationType::PistolIdle, "PistolIdle");
    InitAnims(spriteData, spriteTemplate, layer, layerName, CharAnimationType::PistolHit, "PistolHit");
    InitAnims(spriteData, spriteTemplate, layer, layerName, CharAnimationType::PistolAttack, "PistolAttack");
    layer.player = CreateSpriteAnimationPlayer(spriteData);
}

void InitCharSprite(SpriteData& spriteData, CharSprite &sprite, const std::string& spriteTemplate) {
    sprite.spriteTemplate = spriteTemplate;

    auto& naked = sprite.layers[(size_t) CharAnimationLayerType::Naked];
    InitLayer(spriteData, spriteTemplate, naked, "Naked");

    auto& outfit = sprite.layers[(size_t) CharAnimationLayerType::Outfit];
    InitLayer(spriteData, spriteTemplate, outfit, "Outfit1");

    auto& hair = sprite.layers[(size_t) CharAnimationLayerType::Hair];
    InitLayer(spriteData, spriteTemplate, hair, "Hair1");

    auto& weapon = sprite.layers[(size_t) CharAnimationLayerType::Weapon];
    InitLayer(spriteData, spriteTemplate, weapon, "Sword");
}

void UpdateCharSprite(SpriteData &spriteData, CharSprite &sprite, float deltaTime) {
    for(auto& layer : sprite.layers) {
        if(layer.player != -1) {
            UpdateSpriteAnimation(spriteData, layer.player, deltaTime);
        }
    }
}

void DrawCharSpriteColors(SpriteData& spriteData, CharSprite &sprite, float x, float y, Color c1, Color c2, Color c3, Color c4) {
    for(auto& layer : sprite.layers) {
        if(layer.player != -1) {
            DrawSpriteAnimationColors(spriteData, layer.player, x, y, c1, c2, c3, c4);
        }
    }
}
