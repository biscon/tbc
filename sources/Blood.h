//
// Created by bison on 21-01-25.
//

#ifndef SANDBOX_BLOOD_H
#define SANDBOX_BLOOD_H

#include "CombatState.h"

void InitializeBloodRendering();
void UnloadBloodRendering();
void PreRenderBloodPools(CombatState &combat);
void DrawBloodPools();

#endif //SANDBOX_BLOOD_H
