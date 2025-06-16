//
// Created by bison on 20-01-25.
//

#ifndef SANDBOX_LEVEL_H
#define SANDBOX_LEVEL_H

#include <string>
#include <unordered_map>
#include "data/GameData.h"
#include "LevelCamera.h"
#include "graphics/Lighting.h"

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

struct LevelExit {
    std::string levelFile;
    std::string spawnPoint;
    int x;
    int y;
};

struct Level {
    std::string name;
    std::vector<std::string> log;
    std::vector<int> partyCharacters;
    std::vector<int> enemyCharacters;
    std::vector<int> npcCharacters;
    std::vector<int> allCharacters;
    std::vector<int> turnOrder;
    int currentCharacter = -1;
    int selectedCharacter = -1;
    Skill* selectedSkill = nullptr;
    int currentCharacterIdx = -1;
    TurnState turnState;
    std::vector<Animation> animations;
    float waitTime = 0;
    TurnState nextState;
    std::map<int, int> threatTable;
    TileMap tileMap;
    int tileSet = -1;
    AttackResult attackResult;
    LevelCamera camera;
    std::unordered_map<std::string, SpawnPoint> spawnPoints;
    std::vector<LevelExit> exits;
    std::unordered_map<int, std::string> enemyGroups;
    std::string currentEnemyGroup;
    std::unordered_map<int, int> npcDialogueNodeIds;

    // lighting
    LightingData lighting;
};

void CreateLevel(Level &level);
void LoadLevel(GameData& data, Level &level, const std::string &filename);
void DestroyLevel(SpriteSheetData& sheetData, Level &level);
void WaitTurnState(Level &level, TurnState state, float waitTime);
void AddPartyToLevelNoPositioning(SpriteData& spriteData, CharacterData& charData, Level &level, std::vector<int> &party);
void AddPartyToLevel(SpriteData& spriteData, CharacterData& charData, Level &level, std::vector<int> &party, const std::string& spawnPoint);
void AddEnemiesToLevel(SpriteData& spriteData, CharacterData& charData, Level &level, std::vector<int> &enemies, const std::string& spawnPoint);

#endif //SANDBOX_LEVEL_H
