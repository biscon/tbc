//
// Created by bison on 20-11-25.
//

#ifndef SANDBOX_LEVELSYSTEM_H
#define SANDBOX_LEVELSYSTEM_H

#include "data/GameData.h"

void CreatePlayField(PlayField &playField, ParticleManager* particleManager);
void UpdatePlayField(GameData& data, PlayField &playField, Level &level, float dt);
void MoveCharacter(GameData& data, PlayField &playField, Level &level, int character, Vector2i target);
void MoveCharacterPartial(GameData& data, PlayField &playField, Level &level, int character, Vector2i target);
void ResetPlayField(PlayField &playField);

void UpdateLevelSystem(GameData& data, Level &level, float dt);

#endif //SANDBOX_LEVELSYSTEM_H
