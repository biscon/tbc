//
// Created by bison on 16-02-25.
//

#include "LevelCamera.h"
#include "raymath.h"
#include "graphics/CharacterSprite.h"

void StartCameraPanToTargetPos(LevelCamera& cam, Vector2 target, float speed) {
    // Compute visible screen center offset from overscroll
    float offsetX = (cam.overscrollRight - cam.overscrollLeft) / 2.0f;
    float offsetY = (cam.overscrollBottom - cam.overscrollTop) / 2.0f;

    // Adjust camera pan target to center target in *visible* screen space
    Vector2 camTarget = {
            target.x - (gameScreenWidthF / 2.0f) + offsetX,
            target.y - (gameScreenHeightF / 2.0f) + offsetY
    };

    // Abort if already close
    Vector2 currentCenter = Vector2Add(cam.camera.target, {gameScreenWidthF / 2.0f, gameScreenHeightF / 2.0f});
    if (Vector2Distance(currentCenter, target) < 32.0f) {
        TraceLog(LOG_INFO, "Camera distance less than threshold, aborting camera pan");
        return;
    }

    cam.cameraVelocity = Vector2{0, 0};
    cam.cameraPanning = true;
    cam.cameraPanTarget = camTarget;
    cam.cameraStartPos = cam.camera.target;

    float distance = Vector2Distance(cam.cameraStartPos, cam.cameraPanTarget);
    cam.cameraPanDuration = (speed > 0.0f) ? distance / speed : 0.0f;
    cam.cameraPanElapsed = 0.0f;
}


void StartCameraPanToTilePos(LevelCamera& cam, Vector2i target, float speed) {
    Vector2 targetPos = {(float)target.x * 16.0f, (float)target.y * 16.0f};
    StartCameraPanToTargetPos(cam, targetPos, speed);
}

void StartCameraPanToTargetChar(SpriteData& spriteData, CharacterData& charData, LevelCamera& cam, int target, float speed) {
    Vector2 charPos = GetCharacterSpritePos(spriteData, charData.sprite[target]);

    // Same offset logic
    float offsetX = (cam.overscrollRight - cam.overscrollLeft) / 2.0f;
    float offsetY = (cam.overscrollBottom - cam.overscrollTop) / 2.0f;

    Vector2 camTarget = {
            charPos.x - (gameScreenWidthF / 2.0f) + offsetX,
            charPos.y - (gameScreenHeightF / 2.0f) + offsetY
    };

    Vector2 currentCenter = Vector2Add(cam.camera.target, {gameScreenWidthF / 2.0f, gameScreenHeightF / 2.0f});
    if (Vector2Distance(currentCenter, charPos) < 32.0f) {
        TraceLog(LOG_INFO, "Camera distance less than threshold, aborting camera pan");
        return;
    }

    cam.cameraVelocity = Vector2{0, 0};
    cam.cameraPanning = true;
    cam.cameraPanTarget = camTarget;
    cam.cameraStartPos = cam.camera.target;

    float distance = Vector2Distance(cam.cameraStartPos, cam.cameraPanTarget);
    cam.cameraPanDuration = (speed > 0.0f) ? distance / speed : 0.0f;
    cam.cameraPanElapsed = 0.0f;
}


void StartCameraPanToTargetCharTime(SpriteData& spriteData, CharacterData& charData, LevelCamera& cam, int target, float duration) {
    Vector2 charPos = GetCharacterSpritePos(spriteData, charData.sprite[target]);

    float offsetX = (cam.overscrollRight - cam.overscrollLeft) / 2.0f;
    float offsetY = (cam.overscrollBottom - cam.overscrollTop) / 2.0f;

    Vector2 camTarget = {
            charPos.x - (gameScreenWidthF / 2.0f) + offsetX,
            charPos.y - (gameScreenHeightF / 2.0f) + offsetY
    };

    cam.cameraVelocity = Vector2{0, 0};
    cam.cameraPanning = true;
    cam.cameraPanTarget = camTarget;
    cam.cameraStartPos = cam.camera.target;
    cam.cameraPanDuration = duration;
    cam.cameraPanElapsed = 0.0f;
}

static float Smootherstep(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

static float EaseOutExpo(float t) {
    return 1 - powf(2, -10 * t);
}

void UpdateCamera(LevelCamera& cam, float dt) {
    // Handle camera panning animation
    if (cam.cameraPanning) {
        cam.cameraPanElapsed += dt;

        if (cam.cameraPanElapsed >= cam.cameraPanDuration) {
            cam.cameraPanning = false;
            cam.camera.target = cam.cameraPanTarget;
        } else {
            float t = cam.cameraPanElapsed / cam.cameraPanDuration;
            t = Smootherstep(t);
            cam.camera.target = Vector2Lerp(cam.cameraStartPos, cam.cameraPanTarget, t);

            // Ceil to avoid jitter
            cam.camera.target.x = ceilf(cam.camera.target.x);
            cam.camera.target.y = ceilf(cam.camera.target.y);
        }
    } else {
        cam.camera.target = Vector2Add(cam.camera.target, cam.cameraVelocity);
        // Ceil camera target to prevent jittering
        cam.camera.target.x = ceilf(cam.camera.target.x);
        cam.camera.target.y = ceilf(cam.camera.target.y);
    }

    // Calculate max scrollable area
    /*
    float maxX = (float)cam.worldWidth - gameScreenWidthF;
    float maxY = (float)cam.worldHeight - gameScreenHeightF;
    float minX = 0.0f;
    float minY = 0.0f;
    */

    // Calculate max scrollable area with overscroll
    float minX = (float) -cam.overscrollLeft;
    float minY = (float) -cam.overscrollTop;
    float maxX = (float) cam.worldWidth - gameScreenWidthF + (float) cam.overscrollRight;
    float maxY = (float) cam.worldHeight - gameScreenHeightF + (float) cam.overscrollBottom;

    // Handle X-axis
    if (cam.cameraLockX) {
        cam.camera.target.x = ((float) cam.worldWidth - gameScreenWidthF) / 2.0f;
    } else {
        cam.camera.target.x = Clamp(cam.camera.target.x, minX, maxX);
        cam.camera.target.x = ceilf(cam.camera.target.x); // Optional: keep integer alignment
    }

    // Handle Y-axis
    if (cam.cameraLockY) {
        cam.camera.target.y = ((float) cam.worldHeight - gameScreenHeightF) / 2.0f;
    } else {
        cam.camera.target.y = Clamp(cam.camera.target.y, minY, maxY);
        cam.camera.target.y = ceilf(cam.camera.target.y);
    }

    if (cam.camera.target.x <= minX && cam.cameraVelocity.x < 0.0f) {
        cam.cameraVelocity.x = 0.0f;
    }
    if (cam.camera.target.x >= maxX && cam.cameraVelocity.x > 0.0f) {
        cam.cameraVelocity.x = 0.0f;
    }
    if (cam.camera.target.y <= minY && cam.cameraVelocity.y < 0.0f) {
        cam.cameraVelocity.y = 0.0f;
    }
    if (cam.camera.target.y >= maxY && cam.cameraVelocity.y > 0.0f) {
        cam.cameraVelocity.y = 0.0f;
    }

}

void InitLevelCamera(LevelCamera &cam) {
    cam.camera = {0};
    cam.camera.target = {0, 0};
    cam.camera.offset = {0, 0};
    cam.camera.rotation = 0.0f;
    cam.camera.zoom = 1.0f;
    cam.cameraVelocity = {0, 0};
    cam.cameraPanTarget = {0, 0};
    cam.cameraStartPos = {0, 0};
    cam.cameraPanning = false;
    cam.cameraPanDuration = 0.0f;
    cam.cameraPanElapsed = 0.0f;
    cam.cameraLockX = false;
    cam.cameraLockY = false;
    cam.overscrollRight = 48;
    cam.overscrollBottom = 32;
}
