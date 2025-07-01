//
// Created by bison on 01-07-25.
//

#ifndef SANDBOX_ACTIONBAR_H
#define SANDBOX_ACTIONBAR_H

#include "data/GameData.h"
#include "util/GameEventQueue.h"

void InitActionBar(GameData& data);
void RenderActionBarUI(GameData& data);
void UpdateActionBar(GameData& data, float dt);
bool HandleActionBarInput(GameData& data); // returns true if it "swallowed" the input

#endif //SANDBOX_ACTIONBAR_H
