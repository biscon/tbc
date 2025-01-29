//
// Created by bison on 29-01-25.
//

#include "DungeonGameMode.h"

#include "raylib.h"
#include "rlgl.h"
#include <cmath>
#include <iostream>

const int screenWidth = 480;
const int screenHeight = 270;

// Dungeon map (1 = wall, 0 = empty space)
const int mapWidth = 8;
const int mapHeight = 8;
int dungeonMap[mapHeight][mapWidth] = {
        {1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 1, 0, 1, 1, 0, 1},
        {1, 0, 1, 0, 0, 1, 0, 1},
        {1, 0, 0, 0, 0, 1, 0, 1},
        {1, 1, 1, 1, 0, 1, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1}
};

// Player structure
struct Player {
    Vector3 position;
    int direction; // 0 = North, 1 = East, 2 = South, 3 = West
} player = {{0.0f, 0.0f, 3.0f}, 0};

// Movement directions (North, East, South, West)
Vector3 forwardDirs[4] = {{0, 0, -1}, {1, 0, 0}, {0, 0, 1}, {-1, 0, 0}};

// Load wall texture
Texture2D wallTexture;
Camera3D camera = {0};

void LoadTextures() {
    wallTexture = LoadTexture(ASSETS_PATH"eye_wall.png");

    SetTextureFilter(wallTexture, TEXTURE_FILTER_POINT);  // Nearest neighbor filtering
    SetTextureWrap(wallTexture, TEXTURE_WRAP_REPEAT);  // Allow tiling
    GenTextureMipmaps(&wallTexture);  // Ensure mipmaps are generated
}

void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color)
{
    float x = position.x;
    float y = position.y;
    float z = position.z;

    // Set desired texture to be enabled while drawing following vertex data
    rlSetTexture(texture.id);

    rlBegin(RL_QUADS);
    rlColor4ub(color.r, color.g, color.b, color.a);

    // Front Face
    rlNormal3f(0.0f, 0.0f, 1.0f);       // Normal Pointing Towards Viewer
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Right Of The Texture and Quad
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left Of The Texture and Quad

    // Back Face
    rlNormal3f(0.0f, 0.0f, -1.0f);      // Normal Pointing Away From Viewer
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Right Of The Texture and Quad
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Left Of The Texture and Quad

    // Top Face
    rlNormal3f(0.0f, 1.0f, 0.0f);       // Normal Pointing Up
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Bottom Left Of The Texture and Quad
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Bottom Right Of The Texture and Quad
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad

    // Bottom Face
    rlNormal3f(0.0f, -1.0f, 0.0f);      // Normal Pointing Down
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Top Right Of The Texture and Quad
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Top Left Of The Texture and Quad
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad

    // Right Face
    rlNormal3f(1.0f, 0.0f, 0.0f);       // Normal Pointing Right
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right Of The Texture and Quad
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Left Of The Texture and Quad
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad

    // Left Face
    rlNormal3f(-1.0f, 0.0f, 0.0f);      // Normal Pointing Left
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Left Of The Texture and Quad
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Right Of The Texture and Quad
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad

    rlEnd();

    rlSetTexture(0);
}


void DungeonInit() {
    TraceLog(LOG_INFO, "DungeonInit");
    LoadTextures();

    camera.position = (Vector3){0, 4.0f, 9.0f};
    camera.target = (Vector3){0, 1.0f, 0};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
}

void DungeonUpdate(float dt) {

}


void DungeonRender() {
    ClearBackground(BLACK);
    //DrawTexture(wallTexture, 10, 10, WHITE);
    BeginMode3D(camera);

    //DrawCube({0,0}, 2.0f, 2.0f, 2.0f, RED);
    DrawGrid(10, 1.0f);
    //RenderDungeon();

    DrawCubeTexture(wallTexture, {0, 1.0f, 0}, 2.0f, 2.0f, 2.0f, WHITE);


    EndMode3D();


    //DrawText("Use Arrow Keys to Move", 10, 10, 20, WHITE);
    DrawFPS(4, 4);
}

void DungeonHandleInput() {
    // Player movement
    if (IsKeyPressed(KEY_RIGHT)) player.direction = (player.direction + 1) % 4;  // Turn right
    if (IsKeyPressed(KEY_LEFT))  player.direction = (player.direction + 3) % 4;  // Turn left

    // Move forward
    if (IsKeyPressed(KEY_UP)) {
        Vector3 nextPos = {
                player.position.x + forwardDirs[player.direction].x,
                player.position.y,
                player.position.z + forwardDirs[player.direction].z
        };

        int newX = (int)nextPos.x;
        int newZ = (int)nextPos.z;

        if (dungeonMap[newZ][newX] == 0) {
            player.position = nextPos;
        }
    }

    // Update camera position
    //camera.position = (Vector3){player.position.x, 2.0f, player.position.z + 2.0f};
    //camera.target = (Vector3){player.position.x, 1.0f, player.position.z};
}


void DungeonPreRender() {

}

void DungeonDestroy() {
    UnloadTexture(wallTexture);
}

void SetupDungeonGameMode() {
    CreateGameMode(GameModes::Dungeon, DungeonInit, DungeonUpdate, DungeonHandleInput, DungeonRender, DungeonPreRender, DungeonDestroy);
}