//
// Created by bison on 16-02-25.
//

#ifndef SANDBOX_LEVELCAMERA_H
#define SANDBOX_LEVELCAMERA_H

#include "raylib.h"
#include "data/GameData.h"
#include "util/MathUtil.h"

void InitLevelCamera(LevelCamera& cam);
void StartCameraPanToTargetPos(LevelCamera& cam, Vector2 target, float speed);
void StartCameraPanToTilePos(LevelCamera& cam, Vector2i target, float speed);
void StartCameraPanToTargetChar(SpriteData& spriteData, CharacterData& charData, LevelCamera& cam, int target, float speed);
void StartCameraPanToTargetCharTime(SpriteData& spriteData, CharacterData& charData, LevelCamera& cam, int target, float duration);
void UpdateCamera(LevelCamera& cam, float dt);

#endif //SANDBOX_LEVELCAMERA_H
