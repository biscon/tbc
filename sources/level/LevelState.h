//
// Created by bison on 20-01-25.
//

#ifndef SANDBOX_LEVELSTATE_H
#define SANDBOX_LEVELSTATE_H

#include <map>
#include "character/Character.h"
#include "graphics/Animation.h"
#include "graphics/TileMap.h"
#include "LevelCamera.h"

enum class TurnState {
    StartTurn,
    EndTurn,
    SelectDestination,
    Move,
    SelectAction,
    SelectEnemy,
    UseSkill,
    Attack,
    AttackDone,
    EnemyTurn,
    Waiting,
    Victory,
    Defeat,
    StartRound,
    EndRound,
};

struct AttackResult {
    bool hit;
    bool crit;
    int damage;
    Character *attacker;
    Character *defender;
};

struct LevelState {
    std::vector<std::string> log;
    std::vector<Character*> playerCharacters;
    std::vector<Character*> enemyCharacters;
    std::vector<Character*> turnOrder;
    Character* currentCharacter = nullptr;
    Character* selectedCharacter = nullptr;
    Skill* selectedSkill = nullptr;
    int currentCharacterIdx;
    TurnState turnState;
    std::vector<Animation> animations;
    float waitTime;
    TurnState nextState;
    std::map<Character*, int> threatTable;
    TileMap tileMap;
    AttackResult attackResult;
    LevelCamera camera;
};

void WaitTurnState(LevelState &combat, TurnState state, float waitTime);

#endif //SANDBOX_LEVELSTATE_H
