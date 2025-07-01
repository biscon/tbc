//
// Created by bison on 20-01-25.
//

#include <algorithm>
#include "FighterAi.h"
#include "level/PlayField.h"
#include "Ai.h"
#include "audio/SoundEffect.h"
#include "level/LevelCamera.h"

static bool AttackIfPossible(SpriteData& spriteData, CharacterData& charData, Level &level) {
    auto playersWithinRange = GetAdjacentCharacters(spriteData, charData, level, level.currentCharacter, CharacterFaction::Player);
    level.selectedSkill = nullptr;
    if((int) playersWithinRange.size() > 0) {
        // attack player
        SortCharactersByThreat(level, playersWithinRange);
        int target = playersWithinRange[0];
        level.selectedCharacter = target;
        level.turnState = TurnState::Attack;
        TraceLog(LOG_INFO, "Enemy attacking: %s", charData.name[level.selectedCharacter].c_str());
        return true;
    }
    else {
        level.selectedCharacter = -1;
        level.turnState = TurnState::EndTurn;
        return false;
    }
}

static bool MoveIfPossible(SpriteData& spriteData, CharacterData& charData, Level& level, PlayField& playField) {
    auto playersWithinRange = GetCharactersWithinMoveRange(spriteData, charData, level, level.currentCharacter, 1, true, CharacterFaction::Player);
    SortCharactersByThreat(level, playersWithinRange);

    if((int) playersWithinRange.size() > 0) {
        playField.mode = PlayFieldMode::None;
        playField.path = playersWithinRange[0].second;
        playField.moving = true;
        CharacterStats& stats = charData.stats[level.currentCharacter];
        stats.AP -= playersWithinRange[0].second.cost;
        // cap at zero
        if(stats.AP < 0) {
            stats.AP = 0;
        }
        level.turnState = TurnState::Move;
        StartCameraPanToTargetChar(spriteData, charData, level.camera, playersWithinRange[0].first, 250.0f);
        return true;
    }
    level.turnState = TurnState::EndTurn;
    return false;
}

static bool PartialMoveIfPossible(SpriteData& spriteData, CharacterData& charData, Level& level, PlayField& playField) {
    auto playersWithinRange = GetCharactersWithinMoveRangePartial(spriteData, charData, level, level.currentCharacter, 1, false, CharacterFaction::Player);
    SortCharactersByThreat(level, playersWithinRange);
    CharacterStats& stats = charData.stats[level.currentCharacter];

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

static void HandleTurn(SpriteData& spriteData, CharacterData& charData, Level &level, PlayField &playField) {
    // do something
    TraceLog(LOG_INFO, "FighterAi::HandleTurn");
    switch(level.turnState) {
        case TurnState::EnemyTurn: {
            if(!AttackIfPossible(spriteData, charData, level)) {
                TraceLog(LOG_INFO, "Attack not possible, move if possible");
                if(!MoveIfPossible(spriteData, charData, level, playField)) {
                    TraceLog(LOG_INFO, "Move not possible, partial move if possible");
                    if(!PartialMoveIfPossible(spriteData, charData, level, playField)) {
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
