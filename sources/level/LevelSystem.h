//
// Created by bison on 20-11-25.
//

#ifndef SANDBOX_LEVELSYSTEM_H
#define SANDBOX_LEVELSYSTEM_H

#include "data/GameData.h"

void InitLevelSystem(LevelSystemData &systemData, ParticleManager* particleManager);
void UpdateLevelSystem(GameData& data, LevelSystemData &systemData, Level &level, float dt);
void MoveCharacter(GameData& data, LevelSystemData &systemData, Level &level, int character, Vector2i target);
void MoveCharacterPartial(GameData& data, LevelSystemData &systemData, Level &level, int character, Vector2i target);
void ResetLevelSystem(LevelSystemData &playField);

void UpdateLevelSystem(GameData& data, Level &level, float dt);

#endif //SANDBOX_LEVELSYSTEM_H
