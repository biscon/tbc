//
// Created by bison on 16-02-25.
//

#include "LevelCamera.h"
#include "raymath.h"

void StartCameraPanToTargetPos(LevelCamera& cam, Vector2 target, float speed) {
    // if distance is less than 20 abort
    if (Vector2Distance(Vector2Add(cam.camera.target, {240, 135}), target) < 32) {
        TraceLog(LOG_INFO, "Camera distance less than threshold, aborting camera pan");
        return;
    }
    // ceil target
    target.x = ceilf(target.x);
    target.y = ceilf(target.y);
    cam.cameraVelocity = Vector2{0, 0};
    cam.cameraPanning = true;
    cam.cameraPanTarget = target;
    cam.cameraPanTarget.x -= 240;
    cam.cameraPanTarget.y -= 135;
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

void StartCameraPanToTargetChar(LevelCamera& cam, Character* target, float speed) {
    // if distance is less than 20 abort
    if (Vector2Distance(Vector2Add(cam.camera.target, {240, 135}), GetCharacterSpritePos(target->sprite)) < 32) {
        TraceLog(LOG_INFO, "Camera distance less than threshold, aborting camera pan");
        return;
    }
    cam.cameraVelocity = Vector2{0, 0};
    cam.cameraPanning = true;
    cam.cameraPanTarget = GetCharacterSpritePos(target->sprite);
    cam.cameraPanTarget.x -= 240;
    cam.cameraPanTarget.y -= 135;
    cam.cameraStartPos = cam.camera.target;

    // Calculate duration based on speed
    float distance = Vector2Distance(cam.cameraStartPos, cam.cameraPanTarget);
    cam.cameraPanDuration = (speed > 0.0f) ? distance / speed : 0.0f; // Avoid division by zero
    cam.cameraPanElapsed = 0.0f;
}

void StartCameraPanToTargetCharTime(LevelCamera& cam, Character* target, float duration) {
    cam.cameraVelocity = Vector2{0, 0};
    cam.cameraPanning = true;
    cam.cameraPanTarget = GetCharacterSpritePos(target->sprite);
    cam.cameraPanTarget.x -= 240;
    cam.cameraPanTarget.y -= 135;
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


    float border = 0;
    // Ensure the camera does not scroll more than border pixels outside the visible area
    float maxX = (float) cam.worldWidth - 480 + border;
    float maxY = (float) cam.worldHeight - 270 + border;
    float minX = -border;
    float minY = -border;

    if (cam.camera.target.x < minX) {
        cam.camera.target.x = minX;
        cam.cameraVelocity.x = 0.0f;
    }
    if (cam.camera.target.y < minY) {
        cam.camera.target.y = minY;
        cam.cameraVelocity.y = 0.0f;
    }
    if (cam.camera.target.x > maxX) {
        cam.camera.target.x = maxX;
        cam.cameraVelocity.x = 0.0f;
    }
    if (cam.camera.target.y > maxY) {
        cam.camera.target.y = maxY;
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

}
