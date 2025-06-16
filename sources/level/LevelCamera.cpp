//
// Created by bison on 16-02-25.
//

#include "LevelCamera.h"
#include "raymath.h"
#include "graphics/CharacterSprite.h"

void StartCameraPanToTargetPos(LevelCamera& cam, Vector2 target, float speed) {
    float halfWidth = gameScreenWidthF/2.0f;
    float halfHeight = gameScreenHeightF/2.0f;
    // if distance is less than 20 abort
    if (Vector2Distance(Vector2Add(cam.camera.target, {halfWidth, halfHeight}), target) < 32) {
        TraceLog(LOG_INFO, "Camera distance less than threshold, aborting camera pan");
        return;
    }
    // ceil target
    target.x = ceilf(target.x);
    target.y = ceilf(target.y);
    cam.cameraVelocity = Vector2{0, 0};
    cam.cameraPanning = true;
    cam.cameraPanTarget = target;
    cam.cameraPanTarget.x -= halfWidth;
    cam.cameraPanTarget.y -= halfHeight;
    cam.cameraStartPos = cam.camera.target;

    // Calculate duration based on speed
    float distance = Vector2Distance(cam.cameraStartPos, cam.cameraPanTarget);
    cam.cameraPanDuration = (speed > 0.0f) ? distance / speed : 0.0f; // Avoid division by zero
    cam.cameraPanElapsed = 0.0f;
}

void StartCameraPanToTilePos(LevelCamera& cam, Vector2i target, float speed) {
    Vector2 targetPos = {(float) target.x * 16, (float) target.y * 16};
    StartCameraPanToTargetPos(cam, targetPos, speed);
}

void StartCameraPanToTargetChar(SpriteData& spriteData, CharacterData& charData, LevelCamera& cam, int target, float speed) {
    float halfWidth = gameScreenWidthF/2.0f;
    float halfHeight = gameScreenHeightF/2.0f;
    CharacterSprite& sprite = charData.sprite[target];
    // if distance is less than 20 abort
    if (Vector2Distance(Vector2Add(cam.camera.target, {halfWidth, halfHeight}), GetCharacterSpritePos(spriteData, sprite)) < 32) {
        TraceLog(LOG_INFO, "Camera distance less than threshold, aborting camera pan");
        return;
    }
    cam.cameraVelocity = Vector2{0, 0};
    cam.cameraPanning = true;
    cam.cameraPanTarget = GetCharacterSpritePos(spriteData, sprite);
    cam.cameraPanTarget.x -= halfWidth;
    cam.cameraPanTarget.y -= halfHeight;
    cam.cameraStartPos = cam.camera.target;

    // Calculate duration based on speed
    float distance = Vector2Distance(cam.cameraStartPos, cam.cameraPanTarget);
    cam.cameraPanDuration = (speed > 0.0f) ? distance / speed : 0.0f; // Avoid division by zero
    cam.cameraPanElapsed = 0.0f;
}

void StartCameraPanToTargetCharTime(SpriteData& spriteData, CharacterData& charData, LevelCamera& cam, int target, float duration) {
    cam.cameraVelocity = Vector2{0, 0};
    cam.cameraPanning = true;
    cam.cameraPanTarget = GetCharacterSpritePos(spriteData, charData.sprite[target]);
    cam.cameraPanTarget.x -= gameScreenWidthF/2.0f;
    cam.cameraPanTarget.y -= gameScreenHeightF/2.0f;
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

void UpdateCameraUnbounded(LevelCamera& cam, float dt) {
    if (cam.cameraPanning) {
        cam.cameraPanElapsed += dt;

        if (cam.cameraPanElapsed >= cam.cameraPanDuration) {
            cam.cameraPanning = false;
            cam.camera.target = cam.cameraPanTarget;
        } else {
            float t = cam.cameraPanElapsed / cam.cameraPanDuration;
            t = Smootherstep(t);  // Use a better interpolation function
            //t = EaseOutExpo(t);
            cam.camera.target = Vector2Lerp(cam.cameraStartPos, cam.cameraPanTarget, t);

            // Ceil camera target to prevent jittering
            cam.camera.target.x = ceilf(cam.camera.target.x);
            cam.camera.target.y = ceilf(cam.camera.target.y);
        }
    }
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
    float maxX = (float)cam.worldWidth - gameScreenWidthF;
    float maxY = (float)cam.worldHeight - gameScreenHeightF;
    float minX = 0.0f;
    float minY = 0.0f;

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
}
