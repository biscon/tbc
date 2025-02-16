//
// Created by bison on 10-01-25.
//

#ifndef SANDBOX_COMBATSCREEN_H
#define SANDBOX_COMBATSCREEN_H

#include "character/Character.h"
#include "level/Combat.h"
#include "Grid.h"
#include "graphics/ParticleSystem.h"

struct ActionIcon {
    char text[64];
    char description[256];
    bool disabled;
    Skill* skill;
};

struct CombatUIState {
    int actionIconScrollIndex;
    bool showActionBarTitle;
    Music combatMusic;
    Music combatVictoryMusic;
    Music combatDefeatMusic;
};

void InitCombatUIState(CombatUIState &uiState);
void DestroyCombatUIState(CombatUIState &uiState);
void DisplayCombatScreen(LevelState &combat, CombatUIState &uiState, GridState &gridState);
void UpdateCombatScreen(LevelState &combat, CombatUIState &uiState, GridState& gridState, float dt);
Character* SelectTargetBasedOnThreat(LevelState& combat);


#endif //SANDBOX_COMBATSCREEN_H
