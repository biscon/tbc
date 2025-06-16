//
// Created by bison on 16-02-25.
//

#ifndef SANDBOX_LEVELCAMERA_H
#define SANDBOX_LEVELCAMERA_H

#include "raylib.h"
#include "data/GameData.h"
#include "util/MathUtil.h"

struct LevelCamera {
    Camera2D camera;
    Vector2 cameraVelocity;
    Vector2 cameraPanTarget;
    Vector2 cameraStartPos;
    bool cameraPanning;
    float cameraPanDuration;
    float cameraPanElapsed;
    int worldWidth;
    int worldHeight;
    bool cameraLockX;
    bool cameraLockY;
};

void InitLevelCamera(LevelCamera& cam);
void StartCameraPanToTargetPos(LevelCamera& cam, Vector2 target, float speed);
void StartCameraPanToTilePos(LevelCamera& cam, Vector2i target, float speed);
void StartCameraPanToTargetChar(SpriteData& spriteData, CharacterData& charData, LevelCamera& cam, int target, float speed);
void StartCameraPanToTargetCharTime(SpriteData& spriteData, CharacterData& charData, LevelCamera& cam, int target, float duration);
void UpdateCamera(LevelCamera& cam, float dt);

#endif //SANDBOX_LEVELCAMERA_H
