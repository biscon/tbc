//
// Created by bison on 20-11-25.
//

#ifndef SANDBOX_LEVELRENDERER_H
#define SANDBOX_LEVELRENDERER_H

#include "data/GameData.h"

void RenderLevelUi(GameData& data, Level &level);
void RenderFloatingStats(GameData& data, Level& level);
Vector2 GetAnimatedCharPos(GameData& data, Level &level, int character);
void RenderLevel(GameData& data, Level &level);

#endif //SANDBOX_LEVELRENDERER_H
