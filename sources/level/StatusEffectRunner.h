//
// Created by bison on 23-01-25.
//

#ifndef SANDBOX_STATUSEFFECTRUNNER_H
#define SANDBOX_STATUSEFFECTRUNNER_H

#include "Level.h"

void ApplyStatusEffects(GameData& data, Level &level, PlayField &playField);
void UpdateStatusEffects(CharacterData& charData, Level &level);

#endif //SANDBOX_STATUSEFFECTRUNNER_H
