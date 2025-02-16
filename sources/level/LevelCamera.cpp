//
// Created by bison on 16-02-25.
//

#include "LevelCamera.h"
#include "raymath.h"

void StartCameraPanToTarget(LevelCamera& cam, Character* target, float speed) {
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

void StartCameraPanToTargetTime(LevelCamera& cam, Character* target, float duration) {
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

void UpdateCamera(LevelCamera& cam, float dt) {
    if (cam.cameraPanning) {
        cam.cameraPanElapsed += dt;

        if (cam.cameraPanElapsed >= cam.cameraPanDuration) {
            cam.cameraPanning = false;
            cam.camera.target = cam.cameraPanTarget;
            TraceLog(LOG_INFO, "Camera arrived at target!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
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
