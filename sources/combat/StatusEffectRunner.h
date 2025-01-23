//
// Created by bison on 23-01-25.
//

#ifndef SANDBOX_STATUSEFFECTRUNNER_H
#define SANDBOX_STATUSEFFECTRUNNER_H

#include "CombatState.h"
#include "ui/Grid.h"

void ApplyStatusEffects(CombatState& combat, GridState& gridState);
void UpdateStatusEffects(CombatState &combat);

#endif //SANDBOX_STATUSEFFECTRUNNER_H
