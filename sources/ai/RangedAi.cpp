//
// Created by bison on 30-07-25.
//

#include "RangedAi.h"

#include <algorithm>
#include "Ai.h"
#include "level/LevelCamera.h"
#include "util/Random.h"
#include "level/Combat.h"
#include "character/Character.h"
#include "PathFinding.h"

static bool CanShoot(GameData& data, Level &level) {
    CharacterStats& stats = data.charData.stats[level.currentCharacter];
    int apCost = GetAttackAPCost(data, level.currentCharacter, 0);
    if(apCost > stats.AP) {
        return false;
    }
    return true;
}

static bool CanRangeAttack(GameData& data, Level &level) {
    int range = GetCurrentWeaponRange(data, level.currentCharacter);
    auto playersWithinRange = GetCharactersWithinShootingRange(data, level, level.currentCharacter, range, CharacterFaction::Player);
    if((int) playersWithinRange.size() > 0) {
        return true;
    }
    return false;
}

static bool SetupRangedAttack(GameData& data, Level &level) {
    CharacterStats& stats = data.charData.stats[level.currentCharacter];
    int apCost = GetAttackAPCost(data, level.currentCharacter, -1);
    if(apCost > stats.AP) {
        return false;
    }
    int range = GetCurrentWeaponRange(data, level.currentCharacter);
    auto playersWithinRange = GetCharactersWithinShootingRange(data, level, level.currentCharacter, range, CharacterFaction::Player);
    if((int) playersWithinRange.size() > 0) {
        level.selectedCharacter = playersWithinRange[0];
        level.turnState = TurnState::AttackRanged;
        TraceLog(LOG_INFO, "Enemy attacking ranged: %s", data.charData.name[level.selectedCharacter].c_str());
        return true;
    }
    return false;
}

static bool CanMoveIntoRange(GameData& data, Level& level, PlayField& playField) {
    int range = GetCurrentWeaponRange(data, level.currentCharacter);
    Vector2i charPos = GetCharacterGridPosI(data.spriteData, data.charData.sprite[level.currentCharacter]);
    CharacterStats& stats = data.charData.stats[level.currentCharacter];

    int targetChar = GetClosestCharacter(data, level, charPos, CharacterFaction::Player);
    if(targetChar == -1) {
        return false;
    }
    Vector2i targetPos = GetCharacterGridPosI(data.spriteData, data.charData.sprite[targetChar]);

    std::vector<Vector2i> reachable = GetReachableTiles(level, charPos, stats.AP);
    std::sort(reachable.begin(), reachable.end(), [&](const Vector2i& a, const Vector2i& b) {
        return Distance(a, charPos) < Distance(b, charPos);
    });

    for(auto& tilePos : reachable) {
        float distance = Distance(tilePos, targetPos);
        if((int) distance <= range && HasLineOfSight(level, tilePos, targetPos, range)) {
            return true;
        }
    }
    return false;
}

static bool SetupMoveToTile(GameData& data, Level& level, PlayField& playField, int charId, Vector2i tilePos) {
    Path path;
    Vector2i charPos = GetCharacterGridPosI(data.spriteData, data.charData.sprite[charId]);
    if(tilePos == charPos) {
        TraceLog(LOG_INFO, "Not moving, already there.");
        return false;
    }
    CalcPath(data.spriteData, data.charData, level, path, charPos, tilePos, charId, IsTileOccupied);

    CharacterStats& stats = data.charData.stats[level.currentCharacter];
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

static bool MoveTowards(GameData& data, Level& level, PlayField& playField, Vector2i targetPos, int attackRange) {
    Path path;
    Vector2i charPos = GetCharacterGridPosI(data.spriteData, data.charData.sprite[level.currentCharacter]);
    CalcPathWithRangePartial(data.spriteData, data.charData, level, path, charPos, targetPos, attackRange, level.currentCharacter, IsTileOccupied);
    CharacterStats& stats = data.charData.stats[level.currentCharacter];

    if(stats.AP > 0) {
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

static bool SetupMoveIntoRange(GameData& data, Level& level, PlayField& playField) {
    int range = GetCurrentWeaponRange(data, level.currentCharacter);
    Vector2i charPos = GetCharacterGridPosI(data.spriteData, data.charData.sprite[level.currentCharacter]);
    CharacterStats& stats = data.charData.stats[level.currentCharacter];

    int targetChar = GetClosestCharacter(data, level, charPos, CharacterFaction::Player);
    if(targetChar == -1) {
        return false;
    }
    Vector2i targetPos = GetCharacterGridPosI(data.spriteData, data.charData.sprite[targetChar]);

    std::vector<Vector2i> reachable = GetReachableTiles(level, charPos, stats.AP);
    std::sort(reachable.begin(), reachable.end(), [&](const Vector2i& a, const Vector2i& b) {
        return Distance(a, charPos) < Distance(b, charPos);
    });

    for(auto& tilePos : reachable) {
        float distance = Distance(tilePos, targetPos);
        if((int) distance <= range && HasLineOfSight(level, tilePos, targetPos, range)) {
            return SetupMoveToTile(data, level, playField, level.currentCharacter, tilePos);
        }
    }

    return MoveTowards(data, level, playField, targetPos, range);
}

static bool ShouldRetreat(GameData& data, Level& level, float threshold) {
    CharacterStats& stats = data.charData.stats[level.currentCharacter];
    int maxHP = CalculateCharHealth(stats);
    if (maxHP <= 0) return false; // safety check to avoid division by zero

    float hpRatio = static_cast<float>(stats.HP) / static_cast<float>(maxHP);
    return hpRatio < threshold;
}

static AiState aiState = AiState::Idle;

static void StartTurn(GameData& data, Level &level, PlayField &playField) {
    TraceLog(LOG_INFO, "RangedAi::StartTurn");
    aiState = AiState::Idle;
}

static void HandleTurn(GameData& data, Level &level, PlayField &playField) {
    CharacterStats& stats = data.charData.stats[level.currentCharacter];
    switch(aiState) {
        case AiState::Idle: {
            TraceLog(LOG_INFO, "RangedAi state: Idle");
            bool canRangeAttack = CanRangeAttack(data, level);
            bool canShoot = CanShoot(data, level);
            if(ShouldRetreat(data, level, 0.2f)) {
                aiState = AiState::Retreating;
            } else if(canShoot && canRangeAttack) {
                aiState = AiState::Attacking;
            } else if(stats.AP > 0 && !canRangeAttack) {
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
            TraceLog(LOG_INFO, "RangedAi state: MovingToRange");
            if(SetupMoveIntoRange(data, level, playField)) {
                aiState = AiState::Idle;
            } else {
                aiState = AiState::Done;
            }
            break;
        }
        case AiState::Attacking: {
            TraceLog(LOG_INFO, "RangedAi state: Attacking");
            if(SetupRangedAttack(data, level)) {
                aiState = AiState::Idle;
            } else {
                aiState = AiState::Done;
            }
            break;
        }
        case AiState::Retreating: {
            TraceLog(LOG_INFO, "RangedAi state: Retreating");
            CharacterStats& stats = data.charData.stats[level.currentCharacter];
            Vector2i fleeTile = ChooseBestFleeTile(data, level, level.currentCharacter, stats.AP);
            if(fleeTile != GetCharacterGridPosI(data.spriteData, data.charData.sprite[level.currentCharacter]))
                SetupMoveToTile(data, level, playField, level.currentCharacter, fleeTile);
            aiState = AiState::Done;
            break;
        }
        case AiState::Done: {
            TraceLog(LOG_INFO, "RangedAi state: Done");
            level.turnState = TurnState::EndTurn;
            break;
        }
    }
}


void CreateRangedAi(const std::string &name) {
    CreateAiInterface(name, HandleTurn, StartTurn);
}
