//
// Created by bison on 10-01-25.
//

#ifndef SANDBOX_COMBATSCREEN_H
#define SANDBOX_COMBATSCREEN_H

#include "Character.h"
#include "Combat.h"
#include "Grid.h"

struct ActionIcon {
    char text[64];
    char description[256];
    bool disabled;
    Skill* skill;
};

struct CombatUIState {
    int actionIconScrollIndex;
    bool showActionBarTitle;
};

void InitCombatUIState(CombatUIState &uiState);
void DisplayCombatScreen(CombatState &combat, CombatUIState &uiState, GridState &gridState);
void UpdateCombatScreen(CombatState &combat, float dt);
Character* SelectTargetBasedOnThreat(CombatState& combat);

#endif //SANDBOX_COMBATSCREEN_H
