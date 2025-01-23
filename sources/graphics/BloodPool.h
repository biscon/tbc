//
// Created by bison on 21-01-25.
//

#ifndef SANDBOX_BLOODPOOL_H
#define SANDBOX_BLOODPOOL_H

#include "combat/CombatState.h"

void InitBloodRendering();
void DestroyBloodRendering();
void PreRenderBloodPools(CombatState &combat);
void DrawBloodPools();

#endif //SANDBOX_BLOODPOOL_H
