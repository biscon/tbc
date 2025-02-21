//
// Created by bison on 16-02-25.
//

#ifndef SANDBOX_LEVELCAMERA_H
#define SANDBOX_LEVELCAMERA_H

#include "raylib.h"
#include "character/Character.h"

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
};

void InitLevelCamera(LevelCamera& cam);
void StartCameraPanToTargetPos(LevelCamera& cam, Vector2 target, float speed);
void StartCameraPanToTilePos(LevelCamera& cam, Vector2i target, float speed);
void StartCameraPanToTargetChar(LevelCamera& cam, Character* target, float speed);
void StartCameraPanToTargetCharTime(LevelCamera& cam, Character* target, float duration);
void UpdateCamera(LevelCamera& cam, float dt);

#endif //SANDBOX_LEVELCAMERA_H
