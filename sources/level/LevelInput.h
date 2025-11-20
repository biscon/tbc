//
// Created by bison on 20-11-25.
//

#ifndef SANDBOX_LEVELINPUT_H
#define SANDBOX_LEVELINPUT_H

#include "data/GameData.h"

void HandleInputCombat(GameData& data, Level &level, LevelSystemData &playField);
void HandleInputRealtime(GameData& data, LevelSystemData &playField, Level &level);

#endif //SANDBOX_LEVELINPUT_H
