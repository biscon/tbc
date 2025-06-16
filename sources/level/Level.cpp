//
// Created by bison on 20-01-25.
//

#include <algorithm>
#include <fstream>
#include "Level.h"
#include "../util/json.hpp"
#include "ai/PathFinding.h"
#include "graphics/SpriteSheet.h"
#include "graphics/TileMap.h"
#include "graphics/CharacterSprite.h"
#include "character/Npc.h"

using json = nlohmann::json;

void WaitTurnState(Level &level, TurnState state, float waitTime) {
    level.nextState = state;
    level.waitTime = waitTime;
    level.turnState = TurnState::Waiting;
}

void CreateLevel(Level &level) {
    level.turnState = TurnState::None;
    level.tileSet = -1;
    InitLevelCamera(level.camera);
}

static std::string GetFilePath(const std::string &filename) {
    return ASSETS_PATH + std::string("levels/") + filename;
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
        Vector2 charPos = GetCharacterSpritePos(spriteData, charData.sprite[character]);
        Vector2i gridPos = GetCharacterGridPosI(spriteData, charData.sprite[character]);
        TraceLog(LOG_INFO, "Placed character %s at %f,%f, grid: %i,%i", charData.name[character].c_str(), charPos.x, charPos.y, gridPos.x, gridPos.y);
        charData.orientation[character] = Orientation::Right;
    }
}

void AddPartyToLevelNoPositioning(SpriteData& spriteData, CharacterData& charData, Level &level, std::vector<int> &party) {
    for (auto &character : party) {
        level.partyCharacters.push_back(character);
        level.allCharacters.push_back(character);
        // Set initial animation to paused
        StartPausedCharacterSpriteAnim(spriteData, charData.sprite[character], SpriteAnimationType::WalkRight, true);
        Vector2 charPos = GetCharacterSpritePos(spriteData, charData.sprite[character]);
        Vector2i gridPos = GetCharacterGridPosI(spriteData, charData.sprite[character]);
        TraceLog(LOG_INFO, "Placed character %s at %f,%f, grid: %i,%i", charData.name[character].c_str(), charPos.x, charPos.y, gridPos.x, gridPos.y);
        charData.orientation[character] = Orientation::Right;
    }
}

void AddNpcToLevel(SpriteData& spriteData, CharacterData& charData, Level &level, int id) {
    level.npcCharacters.push_back(id);
    level.allCharacters.push_back(id);
    // Set initial animation to paused
    StartPausedCharacterSpriteAnim(spriteData, charData.sprite[id], SpriteAnimationType::WalkRight, true);
    Vector2 charPos = GetCharacterSpritePos(spriteData, charData.sprite[id]);
    Vector2i gridPos = GetCharacterGridPosI(spriteData, charData.sprite[id]);
    TraceLog(LOG_INFO, "Placed npc %s at %f,%f, grid: %i,%i", charData.name[id].c_str(), charPos.x, charPos.y, gridPos.x, gridPos.y);
    charData.orientation[id] = Orientation::Right;
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

void LoadLevel(GameData& data, Level &level, const std::string &filename) {
    level.animations.clear();
    level.partyCharacters.clear();
    level.allCharacters.clear();
    level.enemyCharacters.clear();
    level.currentCharacter = -1;
    level.selectedCharacter = -1;
    level.selectedSkill = nullptr;
    level.turnState = TurnState::None;
    std::string filePath = GetFilePath(filename);
    TraceLog(LOG_INFO, "Loading level from %s", filePath.c_str());

    std::ifstream file(filePath);
    if (!file) {
        TraceLog(LOG_ERROR, "Failed to open level file: %s", filePath.c_str());
        std::abort();
        return;
    }

    json j;
    file >> j;
    std::string n = j["name"].get<std::string>();
    level.name = n;
    TraceLog(LOG_INFO, "Level name: %s", n.c_str());
    if(level.tileSet != -1) {
        UnloadSpriteSheet(data.spriteData.sheet, level.tileSet);
    }
    level.tileSet = LoadSpriteSheet(data.spriteData.sheet, GetFilePath(j["tileset"].get<std::string>()).c_str(), 16, 16);
    LoadTileMap(level.tileMap, GetFilePath(j["map"].get<std::string>()).c_str(), level.tileSet);

    level.spawnPoints.clear();
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

    // load exits
    level.exits.clear();
    for (auto &jExit : j["exits"]) {
        LevelExit e;
        std::string levelFile = jExit["level"].get<std::string>();
        e.levelFile = levelFile;
        std::string spawnPoint = jExit["spawnPoint"].get<std::string>();
        e.spawnPoint = spawnPoint;
        e.x = jExit["x"].get<int>();
        e.y = jExit["y"].get<int>();
        level.exits.emplace_back(e);
    }

    data.levelState[level.name]; // this line alone ensures the key exists with a default-constructed value
    for(auto& grp : data.levelState[level.name].defeatedGroups) {
        TraceLog(LOG_INFO, "Defeated group: %s", grp.c_str());
    }

    // load enemies
    level.enemyGroups.clear();
    for (auto &jGroup : j["enemies"]) {
        std::vector<int> enemyGroup;
        std::string group = jGroup["group"].get<std::string>();
        if (data.levelState[level.name].defeatedGroups.count(group)) {
            // skip creating defeated group
            TraceLog(LOG_INFO, "Skipped creating defeated group: %s", group.c_str());
            continue;
        }

        std::string spawnAt = jGroup["spawnAt"].get<std::string>();
        for (auto &jCharTplName : jGroup["characters"]) {
            std::string charTplName = jCharTplName.get<std::string>();
            TraceLog(LOG_INFO, "Spawning enemy npc from template %s at %s", charTplName.c_str(), spawnAt.c_str());
            int id = CreateCharacterFromTemplate(data, charTplName);
            level.enemyGroups[id] = group;
            enemyGroup.emplace_back(id);
        }
        AddEnemiesToLevel(data.spriteData, data.charData, level, enemyGroup, spawnAt);
    }

    // load npcs
    level.npcCharacters.clear();
    level.npcDialogueNodeIds.clear();
    for (auto &jNpc : j["npcs"]) {
        NpcInstance npc;
        jNpc.get_to(npc);
        TraceLog(LOG_INFO, "Reading npc %s", npc.npcTemplate.c_str());
        int id = CreateCharacterFromTemplate(data, npc.npcTemplate);
        Vector2 pos = GridToPixelPosition(npc.position.x, npc.position.y);
        SetCharacterSpritePos(data.spriteData, data.charData.sprite[id], pos);
        AddNpcToLevel(data.spriteData, data.charData, level, id);
        level.npcDialogueNodeIds[id] = npc.dialogueNodeId;
    }

    level.camera.worldWidth = level.tileMap.width * level.tileMap.tileWidth;
    level.camera.worldHeight = level.tileMap.height * level.tileMap.tileHeight;

    level.camera.cameraLockX = (level.camera.worldWidth <= gameScreenWidth);
    level.camera.cameraLockY = (level.camera.worldHeight <= gameScreenHeight);
}

void DestroyLevel(SpriteSheetData& sheetData, Level &level) {
    UnloadTileMap(level.tileMap);
    if(level.tileSet > 0) {
        UnloadSpriteSheet(sheetData, level.tileSet);
    }
}


