//
// Created by bison on 26-12-25.
//

#include "CharSprite.h"
#include "SpriteAnimation.h"
#include "ai/PathFinding.h"
#include "util/GraphicUtil.h"

static const SkinPreset gSkinPresets[] = {
        { "Caucasian",
                { ColorToVec3("c68642"), ColorToVec3("f1c27d") }
        },
        { "Hispanic",
                { ColorToVec3("a86b3c"), ColorToVec3("e0a36f") }
        },
        { "Asian",
                { ColorToVec3("b77a4a"), ColorToVec3("f0c28c") }
        },
        { "Black",
                { ColorToVec3("5c3a1e"), ColorToVec3("8d5524") }
        }
};

static constexpr size_t SKIN_PRESET_COUNT =
        sizeof(gSkinPresets) / sizeof(gSkinPresets[0]);

static const HairPreset gHairPresets[] = {
        { "Black",
                { ColorToVec3("1b1b1b"), ColorToVec3("3a3a3a") }
        },
        { "DarkBrown",
                { ColorToVec3("2e1b12"), ColorToVec3("5a3a2b") }
        },
        { "Brown",
                { ColorToVec3("4a2c1d"), ColorToVec3("8b5a3c") }
        },
        { "LightBrown",
                { ColorToVec3("6b3f1f"), ColorToVec3("c68642") }
        },
        { "Blonde",
                { ColorToVec3("d1b24c"), ColorToVec3("f2d335") }
        },
        { "Platinum",
                { ColorToVec3("e5e0b8"), ColorToVec3("fbf7da") }
        },
        { "Red",
                { ColorToVec3("7a2e18"), ColorToVec3("c1442e") }
        },
        { "Grey",
                { ColorToVec3("8a8a8a"), ColorToVec3("d0d0d0") }
        }
};

static constexpr size_t HAIR_PRESET_COUNT =
        sizeof(gHairPresets) / sizeof(gHairPresets[0]);


static const OutfitPreset gOutfitPresets[] = {

        { "WornDenim_Jacket",
                {
                        ColorToVec3("3a3f5a"), // pants dark (washed denim)
                        ColorToVec3("5a6085"), // pants light
                        ColorToVec3("4a3b2f"), // jacket dark (brown canvas)
                        ColorToVec3("7a6755")  // jacket light
                }
        },

        { "WorkerKhaki",
                {
                        ColorToVec3("4a463a"),
                        ColorToVec3("6e6a55"),
                        ColorToVec3("3e4a3f"),
                        ColorToVec3("5f6f60")
                }
        },

        { "UrbanGrey",
                {
                        ColorToVec3("3f3f3f"),
                        ColorToVec3("626262"),
                        ColorToVec3("4a4a4a"),
                        ColorToVec3("707070")
                }
        },

        { "OliveFatigue",
                {
                        ColorToVec3("3f4a2f"),
                        ColorToVec3("5f6a45"),
                        ColorToVec3("4a5035"),
                        ColorToVec3("6a7450")
                }
        },

        { "DustyTraveler",
                {
                        ColorToVec3("5a4532"),
                        ColorToVec3("7a6045"),
                        ColorToVec3("4a3f34"),
                        ColorToVec3("6a5a48")
                }
        },

        { "FadedBlue_Workshirt",
                {
                        ColorToVec3("4a4f66"),
                        ColorToVec3("6a6f8a"),
                        ColorToVec3("3f4a5a"),
                        ColorToVec3("5f6a7a")
                }
        },

        { "RustAndCanvas",
                {
                        ColorToVec3("5a3a2f"),
                        ColorToVec3("7a5445"),
                        ColorToVec3("4a4a3a"),
                        ColorToVec3("6a6a55")
                }
        },

        { "NomadSand",
                {
                        ColorToVec3("6a5a3a"),
                        ColorToVec3("8a7650"),
                        ColorToVec3("5a4a32"),
                        ColorToVec3("7a6645")
                }
        },

        { "CharcoalVest",
                {
                        ColorToVec3("333333"),
                        ColorToVec3("555555"),
                        ColorToVec3("4a3a3a"),
                        ColorToVec3("6a5555")
                }
        },

        { "GreenHoodie",
                {
                        ColorToVec3("4a4f3a"),
                        ColorToVec3("6a6f55"),
                        ColorToVec3("3f5a3f"),
                        ColorToVec3("5f7a5f")
                }
        },

        { "BrownLeather",
                {
                        ColorToVec3("3f2a1e"),
                        ColorToVec3("6a4a32"),
                        ColorToVec3("4a2f1e"),
                        ColorToVec3("7a5438")
                }
        },

        { "ColdWeather",
                {
                        ColorToVec3("3a3f4a"),
                        ColorToVec3("5a6070"),
                        ColorToVec3("4a4f5a"),
                        ColorToVec3("6a7085")
                }
        },

        { "WashedBlack",
                {
                        ColorToVec3("2f2f2f"),
                        ColorToVec3("4a4a4a"),
                        ColorToVec3("3a3a3a"),
                        ColorToVec3("5a5a5a")
                }
        },

        { "RuralWorker",
                {
                        ColorToVec3("4a3f2f"),
                        ColorToVec3("6a5a45"),
                        ColorToVec3("3f4a3f"),
                        ColorToVec3("5f6a5f")
                }
        },

        { "OldUniform",
                {
                        ColorToVec3("3f4a4a"),
                        ColorToVec3("5f6a6a"),
                        ColorToVec3("4a4f4a"),
                        ColorToVec3("6a705f")
                }
        },

        { "Patchwork",
                {
                        ColorToVec3("5a3f3f"),
                        ColorToVec3("7a5a5a"),
                        ColorToVec3("3f4a4a"),
                        ColorToVec3("5f6a6a")
                }
        }
};

static constexpr size_t OUTFIT_PRESET_COUNT =
        sizeof(gOutfitPresets) / sizeof(gOutfitPresets[0]);

static void InitAnims(SpriteData& spriteData, const std::string& spriteTemplate, CharAnimationLayer& layer, const std::string& layerName, CharAnimationType animType, const std::string& animName) {
    layer.animations[(size_t) animType] = GetSpriteAnimation(spriteData, spriteTemplate + "_" + animName + "_" + layerName);
}

static void InitLayer(SpriteData& spriteData, const std::string& spriteTemplate, CharAnimationLayer& layer, const std::string& layerName) {
    InitAnims(spriteData, spriteTemplate, layer, layerName, CharAnimationType::Idle, "Idle");
    InitAnims(spriteData, spriteTemplate, layer, layerName, CharAnimationType::MeleeIdle, "MeleeIdle");
    InitAnims(spriteData, spriteTemplate, layer, layerName, CharAnimationType::MeleeHit, "MeleeHit");
    InitAnims(spriteData, spriteTemplate, layer, layerName, CharAnimationType::MeleeAttack, "MeleeAttack");
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

void DrawCharSpriteColors(SpriteData& spriteData, CharSprite &sprite, Color c1, Color c2, Color c3, Color c4) {
    auto& shader = spriteData.charShader.shader;
    auto& params = spriteData.charShader;
    BeginShaderMode(shader);

    // configure colors
    SetShaderValue(shader, params.locSkinDst1, &sprite.skinColor1, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, params.locSkinDst2, &sprite.skinColor2, SHADER_UNIFORM_VEC3);

    SetShaderValue(shader, params.locHairDst1, &sprite.hairColor1, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, params.locHairDst2, &sprite.hairColor2, SHADER_UNIFORM_VEC3);

    SetShaderValue(shader, params.locOutfitDst1, &sprite.outfitColor1, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, params.locOutfitDst2, &sprite.outfitColor2, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, params.locOutfitDst3, &sprite.outfitColor3, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, params.locOutfitDst4, &sprite.outfitColor4, SHADER_UNIFORM_VEC3);

    for(auto& layer : sprite.layers) {
        if(layer.player != -1) {
            SpriteAnimationPlayerRenderData& renderData = spriteData.player.renderData[layer.player];
            renderData.flipX = sprite.orientation == CharOrientation::Left;
            DrawSpriteAnimationColors(spriteData, layer.player, renderData.position.x, renderData.position.y, c1, c2, c3, c4);
        }
    }
    EndShaderMode();
}

void PlayCharSpriteAnim(SpriteData& spriteData, CharSprite &sprite, CharAnimationType type, bool loop) {
    sprite.currentAnim = type;
    for(auto& layer : sprite.layers) {
        if(layer.player != -1) {
            int animId = layer.animations[(size_t) type];
            if(animId != INVALID_ANIM) {
                PlaySpriteAnimation(spriteData, layer.player, animId, loop);
                TraceLog(LOG_INFO, "Starting animation type %i on player %i", type, layer.player);
            } else {
                TraceLog(LOG_INFO, "Cant start anim!!!");
            }
        }
    }
}

Vector2 GetCharSpritePos(SpriteData& spriteData, CharSprite &sprite) {
    return spriteData.player.renderData[sprite.layers[0].player].position;
}

Vector2i GetCharSpritePosI(SpriteData& spriteData, CharSprite &sprite) {
    SpriteAnimationPlayerRenderData& renderData = spriteData.player.renderData[sprite.layers[0].player];
    return Vector2i{(int) renderData.position.x, (int) renderData.position.y};
}

Vector2i GetCharGridPosI(SpriteData& spriteData, CharSprite &sprite) {
    SpriteAnimationPlayerRenderData& renderData = spriteData.player.renderData[sprite.layers[0].player];
    return PixelToGridPositionI((int) renderData.position.x, (int) renderData.position.y);
}

void SetCharSpritePos(SpriteData& spriteData, CharSprite &sprite, Vector2 pos) {
    for(auto& layer : sprite.layers) {
        if(layer.player != -1) {
            SpriteAnimationPlayerRenderData& renderData = spriteData.player.renderData[layer.player];
            renderData.position = pos;
        }
    }
}

void SetCharSpritePosI(SpriteData& spriteData, CharSprite &sprite, Vector2i pos) {
    SetCharSpritePos(spriteData, sprite, Vector2{(float)pos.x, (float)pos.y});
}

void SetCharGridPosI(SpriteData& spriteData, CharSprite& sprite, Vector2i pos) {
    Vector2 spritePos = GridToPixelPosition(pos.x, pos.y);
    SetCharSpritePos(spriteData, sprite, spritePos);
}

void SetCharSpriteScale(SpriteData& spriteData, CharSprite &sprite, float scale) {
    for(auto& layer : sprite.layers) {
        if(layer.player != -1) {
            SpriteAnimationPlayerRenderData& renderData = spriteData.player.renderData[layer.player];
            renderData.scale.x = scale;
            renderData.scale.y = scale;
        }
    }
}

void InitCharShader(SpriteData &spriteData) {
    spriteData.charShader.shader = LoadShader(nullptr, "../shaders/char_shader.fs.glsl");
    auto& shader = spriteData.charShader.shader;
    auto& params = spriteData.charShader;
    params.locSkinSrc1 = GetShaderLocation(shader, "uSkinSrc1");
    params.locSkinSrc2 = GetShaderLocation(shader, "uSkinSrc2");
    params.locHairSrc1 = GetShaderLocation(shader, "uHairSrc1");
    params.locHairSrc2 = GetShaderLocation(shader, "uHairSrc2");
    params.locOutfitSrc1 = GetShaderLocation(shader, "uOutfitSrc1");
    params.locOutfitSrc2 = GetShaderLocation(shader, "uOutfitSrc2");
    params.locOutfitSrc3 = GetShaderLocation(shader, "uOutfitSrc3");
    params.locOutfitSrc4 = GetShaderLocation(shader, "uOutfitSrc4");

    params.locSkinDst1 = GetShaderLocation(shader, "uSkinDst1");
    params.locSkinDst2 = GetShaderLocation(shader, "uSkinDst2");
    params.locHairDst1 = GetShaderLocation(shader, "uHairDst1");
    params.locHairDst2 = GetShaderLocation(shader, "uHairDst2");
    params.locOutfitDst1 = GetShaderLocation(shader, "uOutfitDst1");
    params.locOutfitDst2 = GetShaderLocation(shader, "uOutfitDst2");
    params.locOutfitDst3 = GetShaderLocation(shader, "uOutfitDst3");
    params.locOutfitDst4 = GetShaderLocation(shader, "uOutfitDst4");

    params.locTolerance = GetShaderLocation(shader, "uTolerance");

    float tolerance = 0.01f;
    SetShaderValue(shader, params.locTolerance, &tolerance, SHADER_UNIFORM_FLOAT);

    Vector3 skinSrc1 = ColorToVec3("d9a066");
    Vector3 skinSrc2 = ColorToVec3("eec39a");
    Vector3 hairSrc1 = ColorToVec3("f2d335");
    Vector3 hairSrc2 = ColorToVec3("fbf236");

    Vector3 outfitSrc1 = ColorToVec3("6abe30");
    Vector3 outfitSrc2 = ColorToVec3("99e550");
    Vector3 outfitSrc3 = ColorToVec3("5b6ee1");
    Vector3 outfitSrc4 = ColorToVec3("639bff");

    SetShaderValue(shader, params.locSkinSrc1, &skinSrc1, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, params.locSkinSrc2, &skinSrc2, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, params.locHairSrc1, &hairSrc1, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, params.locHairSrc2, &hairSrc2, SHADER_UNIFORM_VEC3);

    SetShaderValue(shader, params.locOutfitSrc1, &outfitSrc1, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, params.locOutfitSrc2, &outfitSrc2, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, params.locOutfitSrc3, &outfitSrc3, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, params.locOutfitSrc4, &outfitSrc4, SHADER_UNIFORM_VEC3);
}

void SetSkinColorPreset(CharSprite& sprite, size_t presetIndex) {
    presetIndex %= SKIN_PRESET_COUNT;
    sprite.skinColor1 = gSkinPresets[presetIndex].colors.c1;
    sprite.skinColor2 = gSkinPresets[presetIndex].colors.c2;
}

void SetHairColorPreset(CharSprite& sprite, size_t presetIndex) {
    presetIndex %= HAIR_PRESET_COUNT;
    sprite.hairColor1 = gHairPresets[presetIndex].colors.c1;
    sprite.hairColor2 = gHairPresets[presetIndex].colors.c2;
}

void SetOutfitColorPreset(CharSprite& sprite, size_t presetIndex) {
    presetIndex %= OUTFIT_PRESET_COUNT;

    const auto& c = gOutfitPresets[presetIndex].colors;
    sprite.outfitColor1 = c.lowerDark;
    sprite.outfitColor2 = c.lowerLight;
    sprite.outfitColor3 = c.upperDark;
    sprite.outfitColor4 = c.upperLight;
}

void SetHairStyle(SpriteData& spriteData, CharSprite& sprite, size_t presetIndex)
{
    if (presetIndex < 1 || presetIndex > 6) return;

    auto& hair = sprite.layers[(size_t)CharAnimationLayerType::Hair];

    TraceLog(LOG_INFO, "Setting hairstyle to preset %i", presetIndex);

    // Only rebind animations
    InitAnims(spriteData, sprite.spriteTemplate, hair, "Hair" + std::to_string(presetIndex),
              CharAnimationType::Idle, "Idle");
    InitAnims(spriteData, sprite.spriteTemplate, hair, "Hair" + std::to_string(presetIndex),
              CharAnimationType::MeleeIdle, "MeleeIdle");
    InitAnims(spriteData, sprite.spriteTemplate, hair, "Hair" + std::to_string(presetIndex),
              CharAnimationType::MeleeHit, "MeleeHit");
    InitAnims(spriteData, sprite.spriteTemplate, hair, "Hair" + std::to_string(presetIndex),
              CharAnimationType::MeleeAttack, "MeleeAttack");
    InitAnims(spriteData, sprite.spriteTemplate, hair, "Hair" + std::to_string(presetIndex),
              CharAnimationType::PistolIdle, "PistolIdle");
    InitAnims(spriteData, sprite.spriteTemplate, hair, "Hair" + std::to_string(presetIndex),
              CharAnimationType::PistolHit, "PistolHit");
    InitAnims(spriteData, sprite.spriteTemplate, hair, "Hair" + std::to_string(presetIndex),
              CharAnimationType::PistolAttack, "PistolAttack");

    // Restart current animation safely
    int anim = hair.animations[(size_t) sprite.currentAnim];
    if (anim != INVALID_ANIM) {
        PlaySpriteAnimation(spriteData, hair.player, anim, true);
        // copy progress from body layer
        SpriteAnimationPlayerAnimData& bodyAnimData = spriteData.player.animData[sprite.layers[0].player];
        spriteData.player.animData[hair.player] = bodyAnimData;
    }
}

void SetOutfit(SpriteData& spriteData, CharSprite& sprite, size_t presetIndex)
{
    if (presetIndex < 1 || presetIndex > 2) return;

    auto& outfit = sprite.layers[(size_t)CharAnimationLayerType::Outfit];

    TraceLog(LOG_INFO, "Setting outfit to preset %i", presetIndex);

    // Only rebind animations
    InitAnims(spriteData, sprite.spriteTemplate, outfit, "Outfit" + std::to_string(presetIndex),
              CharAnimationType::Idle, "Idle");
    InitAnims(spriteData, sprite.spriteTemplate, outfit, "Outfit" + std::to_string(presetIndex),
              CharAnimationType::MeleeIdle, "MeleeIdle");
    InitAnims(spriteData, sprite.spriteTemplate, outfit, "Outfit" + std::to_string(presetIndex),
              CharAnimationType::MeleeHit, "MeleeHit");
    InitAnims(spriteData, sprite.spriteTemplate, outfit, "Outfit" + std::to_string(presetIndex),
              CharAnimationType::MeleeAttack, "MeleeAttack");
    InitAnims(spriteData, sprite.spriteTemplate, outfit, "Outfit" + std::to_string(presetIndex),
              CharAnimationType::PistolIdle, "PistolIdle");
    InitAnims(spriteData, sprite.spriteTemplate, outfit, "Outfit" + std::to_string(presetIndex),
              CharAnimationType::PistolHit, "PistolHit");
    InitAnims(spriteData, sprite.spriteTemplate, outfit, "Outfit" + std::to_string(presetIndex),
              CharAnimationType::PistolAttack, "PistolAttack");

    // Restart current animation safely
    int anim = outfit.animations[(size_t) sprite.currentAnim];
    if (anim != INVALID_ANIM) {
        PlaySpriteAnimation(spriteData, outfit.player, anim, true);
        // copy progress from body layer
        SpriteAnimationPlayerAnimData& bodyAnimData = spriteData.player.animData[sprite.layers[0].player];
        spriteData.player.animData[outfit.player] = bodyAnimData;
    }
}

void SetCharWeaponType(SpriteData& spriteData, CharSprite& sprite, const std::string& type)
{
    auto& weapon = sprite.layers[(size_t)CharAnimationLayerType::Weapon];

    TraceLog(LOG_INFO, "Setting weapon type to %s", type.c_str());

    // Only rebind animations
    InitAnims(spriteData, sprite.spriteTemplate, weapon, type,
              CharAnimationType::Idle, "Idle");
    InitAnims(spriteData, sprite.spriteTemplate, weapon, type,
              CharAnimationType::MeleeIdle, "MeleeIdle");
    InitAnims(spriteData, sprite.spriteTemplate, weapon, type,
              CharAnimationType::MeleeHit, "MeleeHit");
    InitAnims(spriteData, sprite.spriteTemplate, weapon, type,
              CharAnimationType::MeleeAttack, "MeleeAttack");
    InitAnims(spriteData, sprite.spriteTemplate, weapon, type,
              CharAnimationType::PistolIdle, "PistolIdle");
    InitAnims(spriteData, sprite.spriteTemplate, weapon, type,
              CharAnimationType::PistolHit, "PistolHit");
    InitAnims(spriteData, sprite.spriteTemplate, weapon, type,
              CharAnimationType::PistolAttack, "PistolAttack");

    // Restart current animation safely
    int anim = weapon.animations[(size_t) sprite.currentAnim];
    if (anim != INVALID_ANIM) {
        PlaySpriteAnimation(spriteData, weapon.player, anim, true);
        // copy progress from body layer
        SpriteAnimationPlayerAnimData& bodyAnimData = spriteData.player.animData[sprite.layers[0].player];
        spriteData.player.animData[weapon.player] = bodyAnimData;
    }
}

void RandomizeCharAppearance(SpriteData& spriteData, CharSprite& sprite) {
    size_t skinIdx = (size_t)GetRandomValue(0, SKIN_PRESET_COUNT - 1);
    size_t hairIdx = (size_t)GetRandomValue(0, HAIR_PRESET_COUNT - 1);
    size_t outfitColorIdx = (size_t)GetRandomValue(0, OUTFIT_PRESET_COUNT - 1);
    size_t outfitIdx = (size_t)GetRandomValue(1, 2);
    size_t hairStyleIdx = (size_t) GetRandomValue(1, 6);

    SetSkinColorPreset(sprite, skinIdx);
    SetHairColorPreset(sprite, hairIdx);
    SetOutfitColorPreset(sprite, outfitColorIdx);
    SetOutfit(spriteData, sprite, outfitIdx);
    SetHairStyle(spriteData, sprite, hairStyleIdx);
}


