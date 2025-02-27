//
// Created by bison on 27-02-25.
//

#ifndef SANDBOX_COMBATENGINE_H
#define SANDBOX_COMBATENGINE_H

#include "Combat.h"
#include "PlayField.h"

void UpdateCombat(CharacterData& charData, WeaponData& weaponData, Level &level, PlayField& playField, float dt);

#endif //SANDBOX_COMBATENGINE_H
