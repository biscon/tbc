//
// Created by bison on 20-01-25.
//

#include <algorithm>
#include <fstream>
#include "Level.h"
#include "../util/json.hpp"
#include "ai/PathFinding.h"

using json = nlohmann::json;

void WaitTurnState(Level &level, TurnState state, float waitTime) {
    level.nextState = state;
    level.waitTime = waitTime;
    level.turnState = TurnState::Waiting;
}

static void InitializeThreatTable(Level& combat) {
    combat.threatTable.clear();
    for (const auto& player : combat.partyCharacters) {
        combat.threatTable[player] = 0;
    }
}

void CreateLevel(Level &level) {
    /*
    std::vector<std::pair<int, Character*>> allCharacters;
    for (auto & playerCharacter : playerCharacters) {
        allCharacters.emplace_back(playerCharacter.speed, &playerCharacter);
        level.playerCharacters.emplace_back(&playerCharacter);
    }
    for (auto & enemyCharacter : enemyCharacters) {
        allCharacters.emplace_back(enemyCharacter.speed, &enemyCharacter);
        level.enemyCharacters.emplace_back(&enemyCharacter);
    }

    // Sort by speed, then randomize in case of tie
    std::sort(allCharacters.begin(), allCharacters.end(), [](std::pair<int, Character*> &left, std::pair<int, Character*> &right) {
        if (left.first != right.first) return left.first > right.first;
        return left.second < right.second;  // Compare by pointer address
    });

    // Now set the current order
    for (auto &pair : allCharacters) {
        level.turnOrder.push_back(pair.second);
    }

    InitializeThreatTable(level);

    level.currentCharacter = level.turnOrder[0];
    level.currentCharacterIdx = 0;

    combat.nextState = TurnState::StartRound;
    combat.waitTime = 3.0f;
    combat.turnState = TurnState::Waiting;
    Animation textAnim{};
    SetupTextAnimation(textAnim, "First round!", 125, 2.0f, 0.0f);
    combat.animations.push_back(textAnim);

    level.turnState = TurnState::Explore;
    */
    InitLevelCamera(level.camera);
}

static std::string GetFilePath(const std::string &filename) {
    return ASSETS_PATH + std::string("levels/") + filename;
}

void LoadLevel(Level &level, const std::string &filename) {
    std::string filePath = GetFilePath(filename);
    TraceLog(LOG_INFO, "Loading level from %s", filePath.c_str());

    std::ifstream file(filePath);
    if (!file) {
        TraceLog(LOG_ERROR, "Failed to open level file: %s", filePath.c_str());
        return;
    }

    json j;
    file >> j;
    std::string n = j["name"].get<std::string>();
    level.name = n;
    TraceLog(LOG_INFO, "Level name: %s", n.c_str());
    LoadSpriteSheet(level.tileSet, GetFilePath(j["tileset"].get<std::string>()).c_str(), 16, 16);
    LoadTileMap(level.tileMap, GetFilePath(j["map"].get<std::string>()).c_str(), &level.tileSet);

    // load spawn points
    for (auto &spawn : j["spawnPoints"]) {
        SpawnPoint sp;
        std::string name = spawn["name"].get<std::string>();
        sp.name = name;
        sp.x = spawn["x"].get<int>();
        sp.y = spawn["y"].get<int>();
        sp.radius = spawn["radius"].get<int>();
        level.spawnPoints[name] = sp;
    }
    level.inCombat = false;
    level.turnState = TurnState::Explore;
    level.camera.worldWidth = level.tileMap.width * level.tileMap.tileWidth;
    level.camera.worldHeight = level.tileMap.height * level.tileMap.tileHeight;
}

void DestroyLevel(Level &level) {
    UnloadTileMap(level.tileMap);
    UnloadSpriteSheet(level.tileSet);
}

static void SetInitialGridPositions(Level &level, SpawnPoint &sp, std::vector<Character*>& characters) {
    auto positions = FindFreePositionsCircular(level, sp.x, sp.y, sp.radius);
    if(characters.size() > positions.size()) {
        TraceLog(LOG_WARNING, "Not enough positions for all characters");
    }
    // Set initial grid positions for characters
    for (auto & character : characters) {
        // take a position from the list
        auto pos = positions.back();
        positions.pop_back();
        SetCharacterSpritePos(character->sprite, GridToPixelPosition(pos.x, pos.y));
        // Set initial animation to paused
        StartPausedCharacterSpriteAnim(character->sprite, SpriteAnimationType::WalkRight, true);
        character->orientation = Orientation::Right;
    }
    /*
    positions = FindFreePositionsCircular(level, 55, 7, 5);
    if(level.enemyCharacters.size() > positions.size()) {
        TraceLog(LOG_WARNING, "Not enough positions for all enemy characters");
    }
    // Set initial grid positions for enemy characters
    for (int i = 0; i < level.enemyCharacters.size(); i++) {
        // take a position from the list
        auto pos = positions.back();
        positions.pop_back();
        SetCharacterSpritePos(level.enemyCharacters[i]->sprite, GridToPixelPosition(pos.x, pos.y));
        // Set initial animation to paused
        StartPausedCharacterSpriteAnim(level.enemyCharacters[i]->sprite, SpriteAnimationType::WalkLeft, true);
        level.enemyCharacters[i]->orientation = Orientation::Left;
    }
    */
}

void AddPartyToLevel(Level &level, std::vector<Character> &party, const std::string& spawnPoint) {
    SpawnPoint& sp = level.spawnPoints[spawnPoint];
    for (auto &character : party) {
        level.partyCharacters.push_back(&character);
        level.allCharacters.push_back(&character);
    }
    SetInitialGridPositions(level, sp, level.partyCharacters);
}

void AddEnemiesToLevel(Level &level, std::vector<Character> &enemies, const std::string& spawnPoint) {
    SpawnPoint& sp = level.spawnPoints[spawnPoint];
    std::vector<Character*> enemyCharacters;
    for (auto &character : enemies) {
        enemyCharacters.push_back(&character);
        level.allCharacters.push_back(&character);
    }
    SetInitialGridPositions(level, sp, enemyCharacters);
}
