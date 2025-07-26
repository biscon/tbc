//
// Created by bison on 20-01-25.
//

#include <algorithm>
#include "FighterAi.h"
#include "level/PlayField.h"
#include "Ai.h"
#include "audio/SoundEffect.h"
#include "level/LevelCamera.h"
#include "util/Random.h"
#include "level/Combat.h"

static bool CanMeleeAttack(GameData& data, Level &level) {
    CharacterStats& stats = data.charData.stats[level.currentCharacter];
    int apCost = GetAttackAPCost(data, level.currentCharacter, -1);
    if(apCost > stats.AP) {
        return false;
    }
    auto playersWithinRange = GetAdjacentCharacters(data, level, level.currentCharacter, CharacterFaction::Player);
    if((int) playersWithinRange.size() > 0) {
        return true;
    }
    return false;
}

static bool SetupMeleeAttack(GameData& data, Level &level) {
    auto playersWithinRange = GetAdjacentCharacters(data, level, level.currentCharacter, CharacterFaction::Player);
    if((int) playersWithinRange.size() > 0) {
        int target = GetRandomElement(playersWithinRange);
        level.selectedCharacter = target;
        level.turnState = TurnState::Attack;
        TraceLog(LOG_INFO, "Enemy attacking: %s", data.charData.name[level.selectedCharacter].c_str());
        return true;
    }
    return false;
}

static bool CanMoveIntoRange(GameData& data, Level& level, PlayField& playField) {
    auto playersWithinRange = GetCharactersWithinMoveRangePartial(data, level, level.currentCharacter, 1, false, CharacterFaction::Player);
    return !playersWithinRange.empty();
}

static bool SetupMoveIntoRange(GameData& data, Level& level, PlayField& playField) {
    auto playersWithinRange = GetCharactersWithinMoveRangePartial(data, level, level.currentCharacter, 1, false, CharacterFaction::Player);
    //SortCharactersByThreat(level, playersWithinRange);
    CharacterStats& stats = data.charData.stats[level.currentCharacter];

    if((int) playersWithinRange.size() > 0 && stats.AP > 0) {
        auto path = playersWithinRange[0].second;
        // truncate path to move points steps
        if(path.path.size() > stats.AP) {
            TraceLog(LOG_INFO, "Truncating path to %d steps", stats.AP);
            path.path.resize(stats.AP);
            path.cost = stats.AP;
        }
        if(path.cost > stats.AP || path.cost == 0) {
            TraceLog(LOG_INFO, "Cant move further toward player");
            return false;
        }
        playField.mode = PlayFieldMode::None;
        playField.path = path;
        playField.moving = true;
        stats.AP -= path.cost;
        // cap at zero
        if(stats.AP < 0) {
            stats.AP = 0;
        }
        level.turnState = TurnState::Move;
        auto lastStep = path.path.back();
        Vector2 lastStepPos = {(float) lastStep.x * 16, (float) lastStep.y * 16};
        StartCameraPanToTargetPos(level.camera, lastStepPos, 250.0f);
        return true;
    }
    return false;
}

static bool AttackIfPossible(GameData& data, Level &level) {
    auto playersWithinRange = GetAdjacentCharacters(data, level, level.currentCharacter, CharacterFaction::Player);
    if((int) playersWithinRange.size() > 0) {
        // attack player
        SortCharactersByThreat(level, playersWithinRange);
        int target = playersWithinRange[0];
        level.selectedCharacter = target;
        level.turnState = TurnState::Attack;
        TraceLog(LOG_INFO, "Enemy attacking: %s", data.charData.name[level.selectedCharacter].c_str());
        return true;
    }
    else {
        level.selectedCharacter = -1;
        level.turnState = TurnState::EndTurn;
        return false;
    }
}

static bool MoveIfPossible(GameData& data, Level& level, PlayField& playField) {
    auto playersWithinRange = GetCharactersWithinMoveRange(data, level, level.currentCharacter, 1, true, CharacterFaction::Player);
    SortCharactersByThreat(level, playersWithinRange);

    if((int) playersWithinRange.size() > 0) {
        playField.mode = PlayFieldMode::None;
        playField.path = playersWithinRange[0].second;
        playField.moving = true;
        CharacterStats& stats = data.charData.stats[level.currentCharacter];
        stats.AP -= playersWithinRange[0].second.cost;
        // cap at zero
        if(stats.AP < 0) {
            stats.AP = 0;
        }
        level.turnState = TurnState::Move;
        StartCameraPanToTargetChar(data.spriteData, data.charData, level.camera, playersWithinRange[0].first, 250.0f);
        return true;
    }
    level.turnState = TurnState::EndTurn;
    return false;
}

static bool PartialMoveIfPossible(GameData& data, Level& level, PlayField& playField) {
    auto playersWithinRange = GetCharactersWithinMoveRangePartial(data, level, level.currentCharacter, 1, false, CharacterFaction::Player);
    SortCharactersByThreat(level, playersWithinRange);
    CharacterStats& stats = data.charData.stats[level.currentCharacter];

    if((int) playersWithinRange.size() > 0 && stats.AP > 0) {
        auto path = playersWithinRange[0].second;
        // truncate path to move points steps
        if(path.path.size() > stats.AP) {
            TraceLog(LOG_INFO, "Truncating path to %d steps", stats.AP);
            path.path.resize(stats.AP);
            path.cost = stats.AP;
        }
        if(path.cost > stats.AP || path.cost == 0) {
            TraceLog(LOG_INFO, "Cant move further toward player");
            level.turnState = TurnState::EndTurn;
            return false;
        }
        playField.mode = PlayFieldMode::None;
        playField.path = path;
        playField.moving = true;
        stats.AP -= path.cost;
        // cap at zero
        if(stats.AP < 0) {
            stats.AP = 0;
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

static void HandleTurnOLD(GameData& data, Level &level, PlayField &playField) {
    // do something
    TraceLog(LOG_INFO, "FighterAi::HandleTurn");
    switch(level.turnState) {
        case TurnState::EnemyTurn: {
            if(!AttackIfPossible(data, level)) {
                TraceLog(LOG_INFO, "Attack not possible, move if possible");
                if(!MoveIfPossible(data, level, playField)) {
                    TraceLog(LOG_INFO, "Move not possible, partial move if possible");
                    if(!PartialMoveIfPossible(data, level, playField)) {
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

static AiState aiState = AiState::Idle;

static void StartTurn(GameData& data, Level &level, PlayField &playField) {
    TraceLog(LOG_INFO, "FighterAi::StartTurn");
    aiState = AiState::Idle;
}

static void HandleTurn(GameData& data, Level &level, PlayField &playField) {
    switch(aiState) {
        case AiState::Idle: {
            TraceLog(LOG_INFO, "FighterAi state: Idle");
            if(CanMeleeAttack(data, level)) {
                aiState = AiState::Attacking;
            } else if(CanMoveIntoRange(data, level, playField)) {
                aiState = AiState::MovingToRange;
            } else {
                aiState = AiState::Done;
            }
            break;
        }
        case AiState::Reloading: {
            TraceLog(LOG_INFO, "FighterAi state: Reloading");
            break;
        }
        case AiState::MovingToRange: {
            TraceLog(LOG_INFO, "FighterAi state: MovingToRange");
            if(SetupMoveIntoRange(data, level, playField)) {
                aiState = AiState::Idle;
            } else {
                aiState = AiState::Done;
            }
            break;
        }
        case AiState::Attacking: {
            TraceLog(LOG_INFO, "FighterAi state: Attacking");
            if(SetupMeleeAttack(data, level)) {
                aiState = AiState::Idle;
            } else {
                aiState = AiState::Done;
            }
            break;
        }
        case AiState::Retreating: {
            TraceLog(LOG_INFO, "FighterAi state: Retreating");
            break;
        }
        case AiState::Done: {
            TraceLog(LOG_INFO, "FighterAi state: Done");
            level.turnState = TurnState::EndTurn;
            break;
        }
    }
}


void CreateFighterAi(const std::string &name) {
    CreateAiInterface(name, HandleTurn, StartTurn);
}
