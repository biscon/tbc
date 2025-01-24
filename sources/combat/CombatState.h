//
// Created by bison on 20-01-25.
//

#ifndef SANDBOX_COMBATSTATE_H
#define SANDBOX_COMBATSTATE_H

#include <map>
#include "character/Character.h"
#include "graphics/Animation.h"
#include "graphics/TileMap.h"

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

struct CombatState {
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
};

void WaitTurnState(CombatState &combat, TurnState state, float waitTime);

#endif //SANDBOX_COMBATSTATE_H
