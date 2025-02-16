//
// Created by bison on 20-01-25.
//

#include <algorithm>
#include "FighterAi.h"
#include "ui/Grid.h"
#include "Ai.h"
#include "audio/SoundEffect.h"

static bool AttackIfPossible(LevelState &combat) {
    auto playersWithinRange = GetAdjacentPlayers(combat, *combat.currentCharacter);
    combat.selectedSkill = nullptr;
    if((int) playersWithinRange.size() > 0) {
        // attack player
        SortCharactersByThreat(combat, playersWithinRange);
        Character* target = playersWithinRange[0];
        combat.selectedCharacter = target;
        combat.turnState = TurnState::Attack;
        TraceLog(LOG_INFO, "Enemy attacking: %s", combat.selectedCharacter->name.c_str());
        return true;
    }
    else {
        combat.selectedCharacter = nullptr;
        combat.turnState = TurnState::EndTurn;
        return false;
    }
}

static bool MoveIfPossible(LevelState& combat, GridState& gridState) {
    auto playersWithinRange = GetPlayersWithinMoveRange(combat, *combat.currentCharacter, 1, true);
    SortCharactersByThreat(combat, playersWithinRange);

    if((int) playersWithinRange.size() > 0) {
        gridState.mode = GridMode::Normal;
        gridState.path = playersWithinRange[0].second;
        gridState.moving = true;
        combat.currentCharacter->movePoints -= playersWithinRange[0].second.cost;
        // cap at zero
        if(combat.currentCharacter->movePoints < 0) {
            combat.currentCharacter->movePoints = 0;
        }
        combat.turnState = TurnState::Move;
        StartCameraPanToTargetChar(combat.camera, playersWithinRange[0].first, 250.0f);
        return true;
    }
    combat.turnState = TurnState::EndTurn;
    return false;
}

static bool PartialMoveIfPossible(LevelState& combat, GridState& gridState) {
    auto playersWithinRange = GetPlayersWithinMoveRangePartial(combat, *combat.currentCharacter, 1, false);
    SortCharactersByThreat(combat, playersWithinRange);

    if((int) playersWithinRange.size() > 0 && combat.currentCharacter->movePoints > 0) {
        auto path = playersWithinRange[0].second;
        // truncate path to move points steps
        if(path.path.size() > combat.currentCharacter->movePoints) {
            TraceLog(LOG_INFO, "Truncating path to %d steps", combat.currentCharacter->movePoints);
            path.path.resize(combat.currentCharacter->movePoints);
            path.cost = combat.currentCharacter->movePoints;
        }
        if(path.cost > combat.currentCharacter->movePoints || path.cost == 0) {
            TraceLog(LOG_INFO, "Cant move further toward player");
            combat.turnState = TurnState::EndTurn;
            return false;
        }
        gridState.mode = GridMode::Normal;
        gridState.path = path;
        gridState.moving = true;
        combat.currentCharacter->movePoints -= path.cost;
        // cap at zero
        if(combat.currentCharacter->movePoints < 0) {
            combat.currentCharacter->movePoints = 0;
        }
        combat.turnState = TurnState::Move;
        auto lastStep = path.path.back();
        Vector2 lastStepPos = {(float) lastStep.x * 16, (float) lastStep.y * 16};
        StartCameraPanToTargetPos(combat.camera, lastStepPos, 250.0f);
        return true;
    }
    combat.turnState = TurnState::EndTurn;
    return false;
}

static void HandleTurn(LevelState &combat, GridState &gridState) {
    // do something
    TraceLog(LOG_INFO, "FighterAi::HandleTurn");
    switch(combat.turnState) {
        case TurnState::EnemyTurn: {
            if(!AttackIfPossible(combat)) {
                TraceLog(LOG_INFO, "Attack not possible, move if possible");
                if(!MoveIfPossible(combat, gridState)) {
                    TraceLog(LOG_INFO, "Move not possible, partial move if possible");
                    if(!PartialMoveIfPossible(combat, gridState)) {
                        TraceLog(LOG_INFO, "Partial move not possible, end turn");
                    } else {
                        TraceLog(LOG_INFO, "Partial move possible, moving");
                        PlaySoundEffect(SoundEffectType::Footstep);
                    }
                } else {
                    TraceLog(LOG_INFO, "Move possible, moving");
                    PlaySoundEffect(SoundEffectType::Footstep);
                }
            } else {
                TraceLog(LOG_INFO, "Attack possible, attacking");
            }
            break;
        }
    }
}

void CreateFighterAi(const std::string &name) {
    CreateAiInterface(name, HandleTurn);
}
