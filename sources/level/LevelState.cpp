//
// Created by bison on 20-01-25.
//

#include "LevelState.h"

void WaitTurnState(LevelState &combat, TurnState state, float waitTime) {
    combat.nextState = state;
    combat.waitTime = waitTime;
    combat.turnState = TurnState::Waiting;
}
