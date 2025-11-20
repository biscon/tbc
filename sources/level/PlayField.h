//
// Created by bison on 16-01-25.
//

#ifndef SANDBOX_PLAYFIELD_H
#define SANDBOX_PLAYFIELD_H

#include "util/MathUtil.h"
#include "graphics/ParticleSystem.h"
#include "Level.h"

void CreatePlayField(PlayField &playField, ParticleManager* particleManager);
void UpdatePlayField(GameData& data, PlayField &playField, Level &level, float dt);
void MoveCharacter(GameData& data, PlayField &playField, Level &level, int character, Vector2i target);
void MoveCharacterPartial(GameData& data, PlayField &playField, Level &level, int character, Vector2i target);
void ResetPlayField(PlayField &playField);

#endif //SANDBOX_PLAYFIELD_H
