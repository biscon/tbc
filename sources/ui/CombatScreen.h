//
// Created by bison on 10-01-25.
//

#ifndef SANDBOX_COMBATSCREEN_H
#define SANDBOX_COMBATSCREEN_H

#include "character/Character.h"
#include "combat/Combat.h"
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
    Camera2D camera;
    Vector2 cameraVelocity;
    Vector2 cameraTarget;
    bool cameraFollowing;
};

void InitCombatUIState(CombatUIState &uiState);
void DestroyCombatUIState(CombatUIState &uiState);
void DisplayCombatScreen(CombatState &combat, CombatUIState &uiState, GridState &gridState);
void UpdateCombatScreen(CombatState &combat, CombatUIState &uiState, GridState& gridState, float dt);
Character* SelectTargetBasedOnThreat(CombatState& combat);

#endif //SANDBOX_COMBATSCREEN_H
