//
// Created by bison on 15-01-25.
//

#include "SpriteAnimation.h"
#include "SpriteSheet.h"
#include <fstream>
#include "../util/json.hpp"
#include "Rendering.h"

using json = nlohmann::json;

void UpdateSpriteAnimation(SpriteData& sprite, int player, float dt) {
    SpriteAnimationData& spriteData = sprite.anim;
    SpriteAnimationPlayerData& playerData = sprite.player;

    if (!playerData.animData[player].playing || playerData.animationIdx[player] == -1) {
        return;
    }

    int animIdx = playerData.animationIdx[player];
    if (spriteData.frames[animIdx].empty()) {
        return;
    }

    SpriteAnimationPlayerAnimData& animData = playerData.animData[player];
    animData.frameTime += dt;

    float currentDelay = spriteData.frameDelays[animIdx][animData.currentFrame];

    if (animData.frameTime >= currentDelay) {
        animData.frameTime -= currentDelay;

        if (!animData.reverse) {
            // Playing forward
            if (animData.currentFrame + 1 < spriteData.frames[animIdx].size()) {
                animData.currentFrame++;
            } else if (animData.loop) {
                animData.currentFrame = 0;
            } else {
                animData.playing = false;
            }
        } else {
            // Playing in reverse
            if (animData.currentFrame > 0) {
                animData.currentFrame--;
            } else if (animData.loop) {
                animData.currentFrame = (int)spriteData.frames[animIdx].size() - 1;
            } else {
                animData.playing = false;
            }
        }
    }
}

void DrawSpriteAnimation(SpriteData& sprite, int player, float x, float y) {
    SpriteSheetData& sheetData = sprite.sheet;
    SpriteAnimationData& spriteData = sprite.anim;
    SpriteAnimationPlayerData& playerData = sprite.player;

    int animIdx = playerData.animationIdx[player];

    if(animIdx == -1 || spriteData.frames[animIdx].empty()) {
        TraceLog(LOG_INFO, "Roaching out from drawing, empty frames or missing animation");
        return;
    }

    int spriteSheetIdx = spriteData.spriteSheetIdx[animIdx];


    // Get the current frame index
    int frameIndex = spriteData.frames[animIdx][playerData.animData[player].currentFrame];

    if (frameIndex < 0 || frameIndex >= sheetData.frameRects[spriteSheetIdx].size()) {
        TraceLog(LOG_INFO, "Roaching out from drawing, cannot find framerect");
        std::abort();
        return; // Invalid frame index, skip drawing
    }
    Rectangle sourceRect = sheetData.frameRects[spriteSheetIdx][frameIndex];
    SpriteAnimationPlayerRenderData& renderData = playerData.renderData[player];

    // Calculate destination rectangle without manually applying origin
    Rectangle destRect = {
            x,                                // World x position
            y,                                // World y position
            sourceRect.width * renderData.scale.x, // Scaled width
            sourceRect.height * renderData.scale.y // Scaled height
    };


    // Define the rotation origin relative to the sprite space
    Vector2 rotationOrigin = {
            spriteData.origin[animIdx].x * renderData.scale.x, // Scaled x origin
            spriteData.origin[animIdx].y * renderData.scale.y  // Scaled y origin
    };

    // Draw the texture with rotation, scale, and tint
    DrawTexturePro(
            sheetData.texture[spriteSheetIdx], // The texture
            sourceRect,           // The source rectangle
            destRect,             // The destination rectangle
            rotationOrigin,       // Rotation origin relative to destRect
            renderData.rotation,      // The rotation in degrees
            renderData.tint           // The color tint
    );
}

void DrawSpriteAnimationScaled(SpriteData& sprite, int player, float x, float y, float scale) {
    SpriteSheetData& sheetData = sprite.sheet;
    SpriteAnimationData& spriteData = sprite.anim;
    SpriteAnimationPlayerData& playerData = sprite.player;

    int animIdx = playerData.animationIdx[player];

    if(animIdx == -1 || spriteData.frames[animIdx].empty()) {
        TraceLog(LOG_INFO, "Roaching out from drawing, empty frames or missing animation");
        return;
    }

    int spriteSheetIdx = spriteData.spriteSheetIdx[animIdx];


    // Get the current frame index
    int frameIndex = spriteData.frames[animIdx][playerData.animData[player].currentFrame];

    if (frameIndex < 0 || frameIndex >= sheetData.frameRects[spriteSheetIdx].size()) {
        TraceLog(LOG_INFO, "Roaching out from drawing, cannot find framerect");
        std::abort();
        return; // Invalid frame index, skip drawing
    }
    Rectangle sourceRect = sheetData.frameRects[spriteSheetIdx][frameIndex];
    SpriteAnimationPlayerRenderData& renderData = playerData.renderData[player];

    // Calculate destination rectangle without manually applying origin
    Rectangle destRect = {
            x,                                // World x position
            y,                                // World y position
            sourceRect.width * scale, // Scaled width
            sourceRect.height * scale // Scaled height
    };


    // Define the rotation origin relative to the sprite space
    Vector2 rotationOrigin = {
            spriteData.origin[animIdx].x * scale, // Scaled x origin
            spriteData.origin[animIdx].y * scale  // Scaled y origin
    };

    // Draw the texture with rotation, scale, and tint
    DrawTexturePro(
            sheetData.texture[spriteSheetIdx], // The texture
            sourceRect,           // The source rectangle
            destRect,             // The destination rectangle
            rotationOrigin,       // Rotation origin relative to destRect
            renderData.rotation,      // The rotation in degrees
            renderData.tint           // The color tint
    );
}

void DrawSpriteAnimation(SpriteData& sprite, int player) {
    SpriteAnimationPlayerRenderData& renderData = sprite.player.renderData[player];
    DrawSpriteAnimation(sprite, player, renderData.position.x, renderData.position.y);
}

int CreateSpriteAnimationPlayer(SpriteData& sprite) {
    SpriteAnimationPlayerData &playerData = sprite.player;
    SpriteAnimationPlayerAnimData animData{};
    animData.playing = false;
    animData.currentFrame = 0;
    animData.frameTime = 0;
    animData.loop = true;
    animData.reverse = false;
    playerData.animData.push_back(animData);

    SpriteAnimationPlayerRenderData renderData{};
    renderData.position = {0, 0};
    renderData.rotation = 0;
    renderData.scale = {1, 1};
    renderData.tint = WHITE;
    playerData.renderData.push_back(renderData);

    playerData.animationIdx.push_back(-1);
    return (int) playerData.animationIdx.size()-1;
}


void PlaySpriteAnimation(SpriteData& sprite, int player, int animation, bool loop) {
    SpriteAnimationPlayerData &playerData = sprite.player;
    SpriteAnimationPlayerAnimData& animData = playerData.animData[player];
    // do nothing if the animation is already playing
    if (playerData.animationIdx[player] == animation) {
        animData.playing = true;
        return;
    }
    playerData.animationIdx[player] = animation;

    animData.currentFrame = 0;
    animData.frameTime = 0;
    animData.loop = loop;
    animData.playing = true;
}

void PlaySpriteAnimationRestart(SpriteData& sprite, int player, int animation, bool loop) {
    SpriteAnimationPlayerAnimData& animData = sprite.player.animData[player];
    sprite.player.animationIdx[player] = animation;
    animData.currentFrame = 0;
    animData.frameTime = 0;
    animData.loop = loop;
    animData.playing = true;
}

void PauseSpriteAnimation(SpriteData& sprite, int player) {
    SpriteAnimationPlayerAnimData& animData = sprite.player.animData[player];
    animData.playing = false;
}

void ResumeSpriteAnimation(SpriteData& sprite, int player) {
    SpriteAnimationPlayerAnimData& animData = sprite.player.animData[player];
    animData.playing = true;
}

void SetReverseSpriteAnimation(SpriteData& sprite, int player, bool reverse) {
    SpriteAnimationPlayerAnimData& animData = sprite.player.animData[player];
    animData.reverse = reverse;
}

void SetFrame(SpriteData& sprite, int player, int frame) {
    SpriteAnimationPlayerAnimData& animData = sprite.player.animData[player];

    if (frame >= 0 && frame < sprite.anim.frames[sprite.player.animationIdx[player]].size()) {
        animData.currentFrame = frame;
        animData.frameTime = 0;
    }
}

FrameInfo GetFrameInfo(SpriteData& sprite, int player) {
    int animIdx = sprite.player.animationIdx[player];

    FrameInfo info{};
    if (sprite.player.animationIdx[player] == -1 || sprite.anim.frames[animIdx].empty()) {
        TraceLog(LOG_ERROR, "GetFrameInfo: No animation or frames");
        return info;
    }

    // Get the current frame index
    int frameIndex = sprite.anim.frames[animIdx][sprite.player.animData[player].currentFrame];
    int spriteSheetIdx = sprite.anim.spriteSheetIdx[animIdx];

    if (frameIndex < 0 || frameIndex >= sprite.sheet.frameRects[spriteSheetIdx].size()) {
        TraceLog(LOG_ERROR, "GetFrameInfo: Invalid frame index: %d", frameIndex);
        return info;
    }
    info.srcRect = sprite.sheet.frameRects[spriteSheetIdx][frameIndex];
    info.texture = sprite.sheet.texture[spriteSheetIdx];

    return info;
}

static void CreateSpriteAnimation(SpriteAnimationData& animData, const std::string& name, int spriteSheetIdx,
                                  std::vector<int> frames, std::vector<float> frameDelays, Vector2 origin) {
    animData.name.push_back(name);
    animData.spriteSheetIdx.push_back(spriteSheetIdx);
    animData.frames.push_back(frames);
    animData.frameDelays.push_back(frameDelays);
    animData.origin.push_back(origin);
    animData.nameIndexMap[name] = (int) animData.name.size()-1;
}

int GetSpriteAnimation(SpriteData& spriteData, const std::string& name) {
    SpriteAnimationData& animData = spriteData.anim;
    if(animData.nameIndexMap.find(name) != animData.nameIndexMap.end()) {
        return animData.nameIndexMap[name];
    } else {
        TraceLog(LOG_ERROR, "GetSpriteAnimation: Animation not found: %s", name.c_str());
        std::abort();
    }
    return -1;
}

void InitSpriteAnimationData(SpriteData& spriteData, const std::string &filename) {
    SpriteSheetData& sheetData = spriteData.sheet;
    SpriteAnimationData& animData = spriteData.anim;
    std::ifstream file(filename);
    json j;
    file >> j;
    for(auto& e : j) {
        std::string group = e["group"].get<std::string>();
        std::string sheetFilename = ASSETS_PATH"" + e["sheet"].get<std::string>();
        TraceLog(LOG_INFO, "SpriteAnimationManager: Loading sprite sheet: %s", sheetFilename.c_str());

        int sheet = LoadSpriteSheet(sheetData, sheetFilename.c_str(), e["frameWidth"].get<int>(), e["frameHeight"].get<int>());
        for(auto& jAnim : e["animations"]) {
            std::string name = group + jAnim["name"].get<std::string>();
            //SpriteAnimationType type = static_cast<SpriteAnimationType>(jAnim["type"].get<int>());
            std::vector<int> frames;
            std::vector<float> frameDelays;
            for(auto& jFrame : jAnim["frames"]) {
                frames.push_back(jFrame.get<int>());
            }
            for(auto& jDelay : jAnim["delays"]) {
                frameDelays.push_back(jDelay.get<float>());
            }
            Vector2 origin = {jAnim["origin"][0].get<float>(), jAnim["origin"][1].get<float>()};
            TraceLog(LOG_INFO, "SpriteAnimationManager: Creating animation: %s", name.c_str());
            CreateSpriteAnimation(animData, name, sheet, frames, frameDelays, origin);
        }
    }
}

void DestroySpriteAnimationData(SpriteData& spriteData) {
    SpriteSheetData& sheetData = spriteData.sheet;
    for(int i = 0; i < sheetData.texture.size(); ++i) {
        UnloadSpriteSheet(sheetData, i);
    }
    sheetData.texture.clear();
    sheetData.frameRects.clear();
    sheetData.frameSizeData.clear();
}

void DrawSpriteAnimationColors(SpriteData& sprite, int player, float x, float y, Color c1, Color c2, Color c3, Color c4) {
    SpriteSheetData& sheetData = sprite.sheet;
    SpriteAnimationData& spriteData = sprite.anim;
    SpriteAnimationPlayerData& playerData = sprite.player;

    int animIdx = playerData.animationIdx[player];
    if(animIdx == -1) {
        TraceLog(LOG_WARNING, "Cannot draw animIdx -1");
        return;
    }
    int spriteSheetIdx = spriteData.spriteSheetIdx[animIdx];

    if(spriteData.frames[animIdx].empty()) {
        TraceLog(LOG_WARNING, "Roaching out from drawing, empty frames or missing animation");
        return;
    }


    // Get the current frame index
    int frameIndex = spriteData.frames[animIdx][playerData.animData[player].currentFrame];

    if (frameIndex < 0 || frameIndex >= sheetData.frameRects[spriteSheetIdx].size()) {
        TraceLog(LOG_WARNING, "Roaching out from drawing, cannot find framerect");
        std::abort();
        return; // Invalid frame index, skip drawing
    }
    Rectangle sourceRect = sheetData.frameRects[spriteSheetIdx][frameIndex];
    SpriteAnimationPlayerRenderData& renderData = playerData.renderData[player];

    // Define the rotation origin relative to the sprite space
    Vector2 scaledOrigin = {
            spriteData.origin[animIdx].x * renderData.scale.x, // Scaled x origin
            spriteData.origin[animIdx].y * renderData.scale.y  // Scaled y origin
    };

    Rectangle destRect = {
            x - scaledOrigin.x,
            y - scaledOrigin.y,
            sourceRect.width * renderData.scale.x, // Scaled width
            sourceRect.height * renderData.scale.y // Scaled height
    };



    // Draw the texture with rotation, scale, and tint
    /*
    DrawTexturePro(
            sheetData.texture[spriteSheetIdx], // The texture
            sourceRect,           // The source rectangle
            destRect,             // The destination rectangle
            rotationOrigin,       // Rotation origin relative to destRect
            renderData.rotation,      // The rotation in degrees
            renderData.tint           // The color tint
    );
    */

    DrawTexturedQuadWithVertexColorsRotated(sheetData.texture[spriteSheetIdx], sourceRect, destRect, c1, c2, c3, c4, scaledOrigin, renderData.rotation);
}


