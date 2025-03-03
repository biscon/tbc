//
// Created by bison on 23-01-25.
//

#ifndef SANDBOX_STATUSEFFECTRUNNER_H
#define SANDBOX_STATUSEFFECTRUNNER_H

#include "Level.h"
#include "PlayField.h"

void ApplyStatusEffects(SpriteData& spriteData, CharacterData& charData, WeaponData& weaponData, Level &level, PlayField &playField);
void UpdateStatusEffects(CharacterData& charData, Level &level);

#endif //SANDBOX_STATUSEFFECTRUNNER_H
