//
// Created by bison on 09-02-25.
//

#ifndef SANDBOX_DUNGEONSTATE_H
#define SANDBOX_DUNGEONSTATE_H

#include <unordered_map>
#include "dungeon/DungeonMap.h"
#include "dungeon/Lighting.h"
#include "util/MathUtil.h"
#include "graphics/SpriteAnimation.h"

struct Player {
    Vector2 position;
    float headOffset;
};

struct WallAlignedObject {
    Vector2i mapPosition;
    Vector3 position;
    Vector2 normal;
    Vector2 billboardSize = {1.0f, 1.0f};
    SpriteAnimationPlayer frontPlayer{};
    SpriteAnimationPlayer sidePlayer{};
};

enum class AxisOrientation {
    X, Y, Z
};

struct ThinWall {
    Vector2i mapPosition;
    Vector3 position;
    AxisOrientation orientation;
    SpriteAnimationPlayer player{};
};

struct DungeonState {
    // Player structure
    Player player = {{9.0f, 3.0f}, 0};
    std::unordered_map<int, Texture2D> textures;
    Camera3D camera = {0};
    // 0 = North, 1 = East, 2 = South, 3 = West
    // Player's current facing direction (0 = North, 1 = East, 2 = South, 3 = West)
    int playerFacing = 2;  // Initial facing direction (South)

    // Camera turn animation variables
    bool isTurning = false;      // Flag to indicate if turning is in progress
    float turnSpeed = 400.0f;      // Degrees per frame, adjust for smoothness
    float turnAngle = 0.0f;      // Accumulated rotation angle
    float targetAngle = 90.0f;   // The target rotation angle (90 degrees)
    int turnDirection = 1;       // 1 for right, -1 for left

    const float moveDuration = 0.35f;  // Duration of the smooth movement
    bool isMoving = false;       // Flag to indicate if moving is in progress
    float moveTimer = 0.0f;      // Time accumulator for movement
    Vector2 moveStart = {0};    // The target position to move to
    Vector2 moveTarget = {0};    // The target position to move to
    DungeonMap* map = nullptr;
    Lighting lighting;
    std::vector<WallAlignedObject> wallObjects;
    std::vector<ThinWall> thinWalls;
};


#endif //SANDBOX_DUNGEONSTATE_H
