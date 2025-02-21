//
// Created by bison on 20-01-25.
//

#include <algorithm>
#include "FighterAi.h"
#include "level/PlayField.h"
#include "Ai.h"
#include "audio/SoundEffect.h"

static bool AttackIfPossible(Level &level) {
    auto playersWithinRange = GetAdjacentPlayers(level, *level.currentCharacter);
    level.selectedSkill = nullptr;
    if((int) playersWithinRange.size() > 0) {
        // attack player
        SortCharactersByThreat(level, playersWithinRange);
        Character* target = playersWithinRange[0];
        level.selectedCharacter = target;
        level.turnState = TurnState::Attack;
        TraceLog(LOG_INFO, "Enemy attacking: %s", level.selectedCharacter->name.c_str());
        return true;
    }
    else {
        level.selectedCharacter = nullptr;
        level.turnState = TurnState::EndTurn;
        return false;
    }
}

static bool MoveIfPossible(Level& level, PlayField& playField) {
    auto playersWithinRange = GetPlayersWithinMoveRange(level, *level.currentCharacter, 1, true);
    SortCharactersByThreat(level, playersWithinRange);

    if((int) playersWithinRange.size() > 0) {
        playField.mode = PlayFieldMode::Normal;
        playField.path = playersWithinRange[0].second;
        playField.moving = true;
        level.currentCharacter->movePoints -= playersWithinRange[0].second.cost;
        // cap at zero
        if(level.currentCharacter->movePoints < 0) {
            level.currentCharacter->movePoints = 0;
        }
        level.turnState = TurnState::Move;
        StartCameraPanToTargetChar(level.camera, playersWithinRange[0].first, 250.0f);
        return true;
    }
    level.turnState = TurnState::EndTurn;
    return false;
}

static bool PartialMoveIfPossible(Level& level, PlayField& playField) {
    auto playersWithinRange = GetPlayersWithinMoveRangePartial(level, *level.currentCharacter, 1, false);
    SortCharactersByThreat(level, playersWithinRange);

    if((int) playersWithinRange.size() > 0 && level.currentCharacter->movePoints > 0) {
        auto path = playersWithinRange[0].second;
        // truncate path to move points steps
        if(path.path.size() > level.currentCharacter->movePoints) {
            TraceLog(LOG_INFO, "Truncating path to %d steps", level.currentCharacter->movePoints);
            path.path.resize(level.currentCharacter->movePoints);
            path.cost = level.currentCharacter->movePoints;
        }
        if(path.cost > level.currentCharacter->movePoints || path.cost == 0) {
            TraceLog(LOG_INFO, "Cant move further toward player");
            level.turnState = TurnState::EndTurn;
            return false;
        }
        playField.mode = PlayFieldMode::Normal;
        playField.path = path;
        playField.moving = true;
        level.currentCharacter->movePoints -= path.cost;
        // cap at zero
        if(level.currentCharacter->movePoints < 0) {
            level.currentCharacter->movePoints = 0;
        }
        level.turnState = TurnState::Move;
        auto lastStep = path.path.back();
        Vector2 lastStepPos = {(float) lastStep.x * 16, (float) lastStep.y * 16};
        StartCameraPanToTargetPos(level.camera, lastStepPos, 250.0f);
        return true;
    }
    level.turnState = TurnState::EndTurn;
    return false;
}

static void HandleTurn(Level &level, PlayField &playField) {
    // do something
    TraceLog(LOG_INFO, "FighterAi::HandleTurn");
    switch(level.turnState) {
        case TurnState::EnemyTurn: {
            if(!AttackIfPossible(level)) {
                TraceLog(LOG_INFO, "Attack not possible, move if possible");
                if(!MoveIfPossible(level, playField)) {
                    TraceLog(LOG_INFO, "Move not possible, partial move if possible");
                    if(!PartialMoveIfPossible(level, playField)) {
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
