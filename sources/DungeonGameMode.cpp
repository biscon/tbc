//
// Created by bison on 29-01-25.
//

#include "DungeonGameMode.h"

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "graphics/DungeonRenderer.h"
#include "dungeon/DungeonMap.h"
#include "ui/MiniMap.h"
#include <cmath>
#include <iostream>
#include <vector>

static DungeonState dungeonState;
static DungeonMap dungeonMap;
static MiniMap miniMap;

void DungeonInit() {
    TraceLog(LOG_INFO, "DungeonInit");
    LoadDungeonMap(dungeonMap, ASSETS_PATH"dungeon_map_01.json");
    InitDungeonRenderer(dungeonState, &dungeonMap);
    LoadDungeonTexture(dungeonState, TILE_WALL1, ASSETS_PATH"brick_wall_grey.png");
    LoadDungeonTexture(dungeonState, TILE_WALL2, ASSETS_PATH"plank_wall.png");
    LoadDungeonTexture(dungeonState, TILE_SHOP1, ASSETS_PATH"shop_front.png");
    LoadDungeonTexture(dungeonState, TILE_FLOOR, ASSETS_PATH"dark_planks_uneven.png");
    //LoadDungeonTexture(dungeonState, TILE_FLOOR, ASSETS_PATH"simple_floor_128x128.png");
    LoadDungeonTexture(dungeonState, TILE_CEILING, ASSETS_PATH"dark_planks_uneven.png");
    LoadDungeonTexture(dungeonState, TILE_DOOR_Z, ASSETS_PATH"simple_door.png");

    LoadDungeonTexture(dungeonState, TILE_PILLAR, ASSETS_PATH"pillar.png");

    CreateMiniMap(miniMap, dungeonMap, 80, 80, {0, 0});

}

void DungeonUpdate(float dt) {
    DungeonRendererUpdate(dungeonState, dt);
    //UpdateCamera(&camera, CAMERA_ORBITAL);
}

void DungeonRender() {
    ClearBackground(BLACK);

    RenderDungeon(dungeonState);
    DrawMiniMap(miniMap, dungeonState);
    //DrawText(TextFormat("Player x,y = %f,%f facing = %d", dungeonState.player.position.x, dungeonState.player.position.y, dungeonState.playerFacing), 10, 10, 10, WHITE);
    //DrawText(TextFormat("Camera x,y,z = %f,%f,%f", dungeonState.camera.position.x, dungeonState.camera.position.y, dungeonState.camera.position.z), 10, 20, 10, WHITE);
}

void DungeonHandleInput() {
    Vector2 newPosition = dungeonState.player.position;
    int playerMapX = (int)(dungeonState.player.position.x / CUBE_SIZE);
    int playerMapY = (int)(dungeonState.player.position.y / CUBE_SIZE);
    int newX = playerMapX, newY = playerMapY;
    if (IsKeyDown(KEY_W) && !dungeonState.isTurning && !dungeonState.isMoving) {

        if (dungeonState.playerFacing == 0) newY -= 1; // North
        else if (dungeonState.playerFacing == 1) newX += 1; // East
        else if (dungeonState.playerFacing == 2) newY += 1; // South
        else if (dungeonState.playerFacing == 3) newX -= 1; // West

        if (newX >= 0 && newX < dungeonMap.width && newY >= 0 && newY < dungeonMap.height &&
                GetDungeonTile(dungeonMap, NAV_LAYER, newX, newY) == 0) {
            newPosition.x = (float) newX * CUBE_SIZE;
            newPosition.y = (float) newY * CUBE_SIZE;
        } else {
            TraceLog(LOG_INFO, "Can't move to %d, %d, mapValue: %d", newX, newY, GetDungeonTile(dungeonMap, NAV_LAYER, newX, newY));
        }
    }
    if (IsKeyDown(KEY_S) && !dungeonState.isTurning && !dungeonState.isMoving) {
        newX = playerMapX, newY = playerMapY;
        if (dungeonState.playerFacing == 0) newY += 1; // South (backwards)
        else if (dungeonState.playerFacing == 1) newX -= 1; // West (backwards)
        else if (dungeonState.playerFacing == 2) newY -= 1; // North (backwards)
        else if (dungeonState.playerFacing == 3) newX += 1; // East (backwards)

        if (newX >= 0 && newX < dungeonMap.width && newY >= 0 && newY < dungeonMap.height && GetDungeonTile(dungeonMap, NAV_LAYER, newX, newY) == 0) {
            newPosition.x = (float) newX * CUBE_SIZE;
            newPosition.y = (float) newY * CUBE_SIZE;
        } else {
            TraceLog(LOG_INFO, "Can't move to %d, %d, mapValue: %d", newX, newY, GetDungeonTile(dungeonMap, NAV_LAYER, newX, newY));
        }
    }

    if (IsKeyDown(KEY_A) && !dungeonState.isTurning && !dungeonState.isMoving) {
        dungeonState.playerFacing = (dungeonState.playerFacing + 3) % 4; // Rotate left
        StartTurn(dungeonState, -1);
    }
    if (IsKeyDown(KEY_D) && !dungeonState.isTurning && !dungeonState.isMoving) {
        dungeonState.playerFacing = (dungeonState.playerFacing + 1) % 4; // Rotate right
        StartTurn(dungeonState, 1);
    }
    if(dungeonState.player.position != newPosition) {
        // start move
        StartMove(dungeonState, newPosition);
        TraceLog(LOG_INFO, "Player moving to %d, %d, mapValue: %d", newX, newY, GetDungeonTile(dungeonMap, NAV_LAYER, newX, newY));
    }
}


void DungeonPreRender() {

}

void DungeonDestroy() {
    DestroyDungeonRenderer(dungeonState);
    UnloadDungeonMap(dungeonMap);
    DestroyMiniMap(miniMap);
}

void DungeonPause() {
    TraceLog(LOG_INFO, "DungeonPause");
}

void DungeonResume() {
    TraceLog(LOG_INFO, "DungeonResume");
}

void SetupDungeonGameMode() {
    CreateGameMode(GameModes::Dungeon, DungeonInit, DungeonUpdate, DungeonHandleInput, DungeonRender, DungeonPreRender, DungeonDestroy, DungeonPause, DungeonResume);
}