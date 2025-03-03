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

void CreateLevel(Level &level) {
    level.turnState = TurnState::None;
    InitLevelCamera(level.camera);
}

static std::string GetFilePath(const std::string &filename) {
    return ASSETS_PATH + std::string("levels/") + filename;
}

void LoadLevel(SpriteSheetData& sheetData, Level &level, const std::string &filename) {
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
    level.tileSet = LoadSpriteSheet(sheetData, GetFilePath(j["tileset"].get<std::string>()).c_str(), 16, 16);
    LoadTileMap(level.tileMap, GetFilePath(j["map"].get<std::string>()).c_str(), level.tileSet);

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
    level.turnState = TurnState::None;
    level.camera.worldWidth = level.tileMap.width * level.tileMap.tileWidth;
    level.camera.worldHeight = level.tileMap.height * level.tileMap.tileHeight;
}

void DestroyLevel(SpriteSheetData& sheetData, Level &level) {
    UnloadTileMap(level.tileMap);
    UnloadSpriteSheet(sheetData, level.tileSet);
}

static void setInitialGridPositions(SpriteData& spriteData, CharacterData& charData, Level &level, SpawnPoint &sp, std::vector<int>& characters) {
    auto positions = FindFreePositionsCircular(level, sp.x, sp.y, sp.radius);
    if(characters.size() > positions.size()) {
        TraceLog(LOG_WARNING, "Not enough positions for all characters");
    }
    // Set initial grid positions for characters
    for (auto & character : characters) {
        // take a position from the list
        auto pos = positions.back();
        positions.pop_back();
        SetCharacterSpritePos(spriteData, charData.sprite[character], GridToPixelPosition(pos.x, pos.y));
        // Set initial animation to paused
        StartPausedCharacterSpriteAnim(spriteData, charData.sprite[character], SpriteAnimationType::WalkRight, true);
        charData.orientation[character] = Orientation::Right;
    }
}

void AddPartyToLevel(SpriteData& spriteData, CharacterData& charData, Level &level, std::vector<int> &party, const std::string& spawnPoint) {
    SpawnPoint& sp = level.spawnPoints[spawnPoint];
    for (auto &character : party) {
        level.partyCharacters.push_back(character);
        level.allCharacters.push_back(character);
    }
    setInitialGridPositions(spriteData, charData, level, sp, level.partyCharacters);
}

void AddEnemiesToLevel(SpriteData& spriteData, CharacterData& charData, Level &level, std::vector<int> &enemies, const std::string& spawnPoint) {
    SpawnPoint& sp = level.spawnPoints[spawnPoint];
    std::vector<int> enemyCharacters;
    for (auto &character : enemies) {
        enemyCharacters.push_back(character);
        level.allCharacters.push_back(character);
    }
    setInitialGridPositions(spriteData, charData, level, sp, enemyCharacters);
}
