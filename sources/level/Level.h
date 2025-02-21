//
// Created by bison on 20-01-25.
//

#ifndef SANDBOX_LEVEL_H
#define SANDBOX_LEVEL_H

#include <string>
#include <unordered_map>
#include "character/Character.h"
#include "graphics/Animation.h"
#include "graphics/TileMap.h"
#include "LevelCamera.h"

enum class TurnState {
    Explore,
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

struct SpawnPoint {
    std::string name;
    int x;
    int y;
    int radius;
};

struct Level {
    std::string name;
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
    SpriteSheet tileSet;
    AttackResult attackResult;
    LevelCamera camera;
    bool inCombat;
    std::unordered_map<std::string, SpawnPoint> spawnPoints;
};

void CreateLevel(Level &level);
void LoadLevel(Level &level, const std::string &filename);
void DestroyLevel(Level &level);
void WaitTurnState(Level &level, TurnState state, float waitTime);
void AddPartyToLevel(Level &level, std::vector<Character>& party);

#endif //SANDBOX_LEVEL_H
