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
    None,
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
    int attacker;
    int defender;
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
    std::vector<int> partyCharacters;
    std::vector<int> enemyCharacters;
    std::vector<int> allCharacters;
    std::vector<int> turnOrder;
    int currentCharacter = -1;
    int selectedCharacter = -1;
    Skill* selectedSkill = nullptr;
    int currentCharacterIdx;
    TurnState turnState;
    std::vector<Animation> animations;
    float waitTime;
    TurnState nextState;
    std::map<int, int> threatTable;
    TileMap tileMap;
    int tileSet;
    AttackResult attackResult;
    LevelCamera camera;
    std::unordered_map<std::string, SpawnPoint> spawnPoints;
};

void CreateLevel(Level &level);
void LoadLevel(SpriteSheetData& sheetData, Level &level, const std::string &filename);
void DestroyLevel(SpriteSheetData& sheetData, Level &level);
void WaitTurnState(Level &level, TurnState state, float waitTime);
void AddPartyToLevel(SpriteData& spriteData, CharacterData& charData, Level &level, std::vector<int> &party, const std::string& spawnPoint);
void AddEnemiesToLevel(SpriteData& spriteData, CharacterData& charData, Level &level, std::vector<int> &enemies, const std::string& spawnPoint);

#endif //SANDBOX_LEVEL_H
