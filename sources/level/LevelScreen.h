//
// Created by bison on 10-01-25.
//

#ifndef SANDBOX_LEVELSCREEN_H
#define SANDBOX_LEVELSCREEN_H

#include "PlayField.h"
#include "graphics/ParticleSystem.h"
#include "util/GameEventQueue.h"


void CreateLevelScreen(GameData& data);
void DestroyLevelScreen(GameData& data);
void DrawLevelScreen(GameData& data, Level &level, PlayField &playField);
void UpdateLevelScreen(GameData& data, Level &level, float dt);
void HandleInputLevelScreen(GameData& data, Level &level);
//Character* SelectTargetBasedOnThreat(Level& level);


#endif //SANDBOX_LEVELSCREEN_H
