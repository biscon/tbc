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
bool HandleActionBarInput(GameData& data, Level& level, PlayField& playField); // returns true if it "swallowed" the input
void ExecuteAction(GameData& data, ActionBarAction action, Level& level, PlayField& playField, bool wasSelected);

#endif //SANDBOX_ACTIONBAR_H
