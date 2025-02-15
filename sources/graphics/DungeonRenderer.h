//
// Created by bison on 31-01-25.
//

#ifndef SANDBOX_DUNGEONRENDERER_H
#define SANDBOX_DUNGEONRENDERER_H

#include <string>
#include "raylib.h"
#include "dungeon/DungeonMap.h"
#include "dungeon/DungeonState.h"

const float CUBE_SIZE = 3.0f;

enum class CellSide { NORTH, SOUTH, WEST, EAST };

struct CubeFaces {
    int front;
    int back;
    int left;
    int right;
    int top;
    int bottom;
};


void InitDungeonRenderer(DungeonState& state, DungeonMap* map);
void DestroyDungeonRenderer(DungeonState& state);
void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color);
void DungeonRendererUpdate(DungeonState& state, float dt);
void RenderDungeon(DungeonState &state);
void StartMove(DungeonState& state, Vector2 newPosition);
void StartTurn(DungeonState& state, int direction);
void LoadDungeonTexture(DungeonState& state, int type, const std::string& filename);

#endif //SANDBOX_DUNGEONRENDERER_H
