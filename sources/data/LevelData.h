//
// Created by bison on 01-07-25.
//

#ifndef SANDBOX_LEVELDATA_H
#define SANDBOX_LEVELDATA_H

#include <string>
#include <unordered_map>
#include "util/MathUtil.h"

#include "LightingData.h"
#include "AnimationData.h"
#include "SaveData.h"
#include "TileMapData.h"
#include "WeatherData.h"

struct LevelCamera {
    Camera2D camera;
    Vector2 cameraVelocity;
    Vector2 cameraPanTarget;
    Vector2 cameraStartPos;
    bool cameraPanning;
    float cameraPanDuration;
    float cameraPanElapsed;
    int worldWidth;
    int worldHeight;
    bool cameraLockX;
    bool cameraLockY;
    // Extra margin to allow overscroll (in pixels)
    int overscrollLeft = 0;
    int overscrollRight = 0;
    int overscrollTop = 0;
    int overscrollBottom = 0;
};

enum class TurnState {
    None,
    StartTurn,
    EndTurn,
    SelectDestination,
    Move,
    SelectAction,
    SelectEnemy,
    Attack,
    AttackRanged,
    AttackDone,
    AttackRangedDone,
    KillCharacters,
    EnemyTurn,
    Waiting,
    StartRound,
    EndRound,
};

struct AttackHit {
    bool hit;
    bool crit;
    int damage;
};
struct AttackResult {
    int minDmg;
    int maxDmg;
    int attacker;
    int defender;
    std::vector<AttackHit> hits;
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
    int width;
    int height;
};

struct LevelObject {
    std::string id;
    std::string spriteTemplate;
    Vector2i gridPos;
    bool loop;
    bool lit;
    int animPlayer;
};

struct LevelDoor {
    std::string id;
    std::string spriteTemplate;
    Vector2i gridPos;
    std::vector<Vector2i> blockedTiles;
    std::vector<Vector2i> shadowTiles;
    int animPlayer;
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
    std::unordered_map<std::string, LevelObject> objects;
    std::unordered_map<std::string, LevelDoor> doors;
    // lighting
    LightingData lighting;
    float hourOfDay = 6;
    WeatherData weather;
};

struct LevelState {
    std::unordered_set<std::string> defeatedGroups;
    std::unordered_map<std::string, DoorSaveState> doors;
    std::unordered_map<std::string, bool> flags;
    std::unordered_map<std::string, int> objectInventories;
};


#endif //SANDBOX_LEVELDATA_H
