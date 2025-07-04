//
// Created by bison on 16-01-25.
//

#ifndef SANDBOX_PLAYFIELD_H
#define SANDBOX_PLAYFIELD_H

#include "util/MathUtil.h"
#include "graphics/ParticleSystem.h"
#include "util/GameEventQueue.h"
#include "data/PathFindingData.h"
#include "Level.h"

enum class PlayFieldMode {
    None,
    SelectingTile,
    SelectingEnemyTarget,
    Explore,
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
    std::vector<CharacterMove> activeMoves;
    std::string hintText;
};

void CreatePlayField(PlayField &playField, ParticleManager* particleManager);
void UpdatePlayField(GameData& data, PlayField &playField, Level &level, float dt);
void HandleInputPlayField(GameData& data, PlayField &playField, Level &level);
void DrawPlayField(GameData& data, PlayField &playField, Level &level);
void MoveCharacter(GameData& data, PlayField &playField, Level &level, int character, Vector2i target);
void MoveCharacterPartial(GameData& data, PlayField &playField, Level &level, int character, Vector2i target);
void ResetPlayField(PlayField &playField);
Vector2 GetAnimatedCharPos(GameData& data, Level &level, int character);

#endif //SANDBOX_PLAYFIELD_H
