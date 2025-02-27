//
// Created by bison on 16-01-25.
//

#ifndef SANDBOX_PLAYFIELD_H
#define SANDBOX_PLAYFIELD_H

#include "util/MathUtil.h"
#include "ai/PathFinding.h"
#include "graphics/TileMap.h"
#include "graphics/ParticleSystem.h"
#include "util/GameEventQueue.h"

enum class PlayFieldMode {
    None,
    SelectingTile,
    SelectingEnemyTarget,
    Move,
};

struct CharacterMove {
    int character;
    Path path;
    bool isDone;
};

struct PlayField {
    bool moving;
    PlayFieldMode mode;
    Vector2 selectedTile;
    Vector2i selectedTilePos;
    int selectedCharacter;
    Path path;
    // Static variables for pulsing highlight
    float highlightAlpha = 0.25f; // Current alpha value
    bool increasing = true;       // Direction of the alpha
    float pulseSpeed = 4.0f; // Speed of the pulsing
    ParticleManager* particleManager;
    GameEventQueue* eventQueue;
    std::vector<CharacterMove> activeMoves;
};

void CreatePlayField(PlayField &playField, ParticleManager* particleManager, GameEventQueue* eventQueue);
void UpdatePlayField(CharacterData& charData, PlayField &playField, Level &level, float dt);
void HandleInputPlayField(CharacterData& charData, PlayField &playField, Level &level);
void DrawPlayField(CharacterData& charData, PlayField &playField, Level &level);
void MoveCharacter(CharacterData& charData, PlayField &playField, Level &level, int character, Vector2i target);
void MoveCharacterPartial(CharacterData& charData, PlayField &playField, Level &level, int character, Vector2i target);

#endif //SANDBOX_PLAYFIELD_H
