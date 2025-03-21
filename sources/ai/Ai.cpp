//
// Created by bison on 20-01-25.
//

#include <algorithm>
#include "Ai.h"
#include "PathFinding.h"

static std::map<std::string, AiInterface> aiInterfaces;

void CreateAiInterface(const std::string& name, void (*HandleTurn)(SpriteData& spriteData, CharacterData& charData, Level &, PlayField &)) {
    AiInterface ai{};
    ai.HandleTurn = HandleTurn;
    aiInterfaces[name] = ai;
}

AiInterface* GetAiInterface(const std::string& name) {
    auto it = aiInterfaces.find(name);
    if (it != aiInterfaces.end()) {
        return &it->second; // Return a pointer to the found value
    }
    return nullptr; // Key not found, return nullptr
}

void HandleTurn(AiInterface &ai, SpriteData& spriteData, CharacterData& charData, Level &level, PlayField &gridState) {
    ai.HandleTurn(spriteData, charData, level, gridState);
}

void SortCharactersByThreat(Level& level, std::vector<int>& characters) {
    // Sort the characters vector using a custom comparator based on the threatTable in CombatState
    std::sort(characters.begin(), characters.end(), [&level](int a, int b) {
        int threatA = level.threatTable.count(a) ? level.threatTable[a] : 0;
        int threatB = level.threatTable.count(b) ? level.threatTable[b] : 0;
        return threatA > threatB; // Higher threat values come first
    });
}

void SortCharactersByThreat(Level& level, std::vector<std::pair<int, Path>>& characters) {
    // Sort the characters vector using a custom comparator based on the threatTable in CombatState
    std::sort(characters.begin(), characters.end(), [&level](const std::pair<int, Path>& a, const std::pair<int, Path>& b) {
        int threatA = level.threatTable.count(a.first) ? level.threatTable[a.first] : 0;
        int threatB = level.threatTable.count(b.first) ? level.threatTable[b.first] : 0;
        return threatA > threatB; // Higher threat values come first
    });
}

std::vector<int> GetCharactersWithinAttackRange(SpriteData& spriteData, CharacterData& charData, Level &level, int character, int range, CharacterFaction faction) {
    // loop through all characters in combat
    Vector2i charPos = GetCharacterSpritePosI(spriteData, charData.sprite[character]);
    Vector2i charGridPos = PixelToGridPositionI(charPos.x, charPos.y);
    std::vector<int> charactersInRange;
    for(auto &c : level.allCharacters) {
        // skip same and death characters
        if(c == character || charData.stats[c].health <= 0 || charData.faction[c] != faction) {
            continue;
        }
        Path path;
        Vector2i cCharPos = GetCharacterSpritePosI(spriteData, charData.sprite[c]);
        Vector2i cGridPos = PixelToGridPositionI(cCharPos.x, cCharPos.y);
        if(CalcPathWithRange(spriteData, charData, level, path, charGridPos, cGridPos, range, character, IsTileOccupied)) {
            if(path.cost <= range) {
                charactersInRange.push_back(c);
            }
        }
    }
    return charactersInRange;
}

std::vector<std::pair<int, Path>> GetCharactersWithinMoveRange(SpriteData& spriteData, CharacterData& charData, Level &level, int character, int attackRange, bool checkPoints, CharacterFaction faction) {
    // loop through all characters in combat
    Vector2i charPos = GetCharacterSpritePosI(spriteData, charData.sprite[character]);
    Vector2i charGridPos = PixelToGridPositionI(charPos.x, charPos.y);
    std::vector<std::pair<int, Path>> charactersInRange;
    for(auto &c : level.allCharacters) {
        // skip same and death characters
        if(c == character || charData.stats[c].health <= 0 || charData.faction[c] != faction) {
            continue;
        }
        Path path;
        Vector2i cCharPos = GetCharacterSpritePosI(spriteData, charData.sprite[c]);
        Vector2i cGridPos = PixelToGridPositionI(cCharPos.x, cCharPos.y);
        if(CalcPathWithRange(spriteData, charData, level, path, charGridPos, cGridPos, attackRange, character, IsTileOccupied)) {
            if(checkPoints) {
                if (path.cost <= charData.stats[character].movePoints) {
                    charactersInRange.emplace_back(c, path);
                }
            } else {
                charactersInRange.emplace_back(c, path);
            }
        }
    }
    return charactersInRange;
}

std::vector<std::pair<int, Path>> GetCharactersWithinMoveRangePartial(SpriteData& spriteData, CharacterData& charData, Level &level, int character, int attackRange, bool checkPoints, CharacterFaction faction) {
    // loop through all characters in combat
    Vector2i charPos = GetCharacterSpritePosI(spriteData, charData.sprite[character]);
    Vector2i charGridPos = PixelToGridPositionI(charPos.x, charPos.y);
    std::vector<std::pair<int, Path>> charactersInRange;
    for(auto &c : level.allCharacters) {
        // skip same and death characters
        if(c == character || charData.stats[c].health <= 0 || charData.faction[c] != faction) {
            continue;
        }
        Path path;
        Vector2i cCharPos = GetCharacterSpritePosI(spriteData, charData.sprite[c]);
        Vector2i cGridPos = PixelToGridPositionI(cCharPos.x, cCharPos.y);
        CalcPathWithRangePartial(spriteData, charData, level, path, charGridPos, cGridPos, attackRange, character, IsTileOccupied);
        if(!path.path.empty()) {
            if(checkPoints) {
                if (path.cost <= charData.stats[character].movePoints) {
                    charactersInRange.emplace_back(c, path);
                }
            } else {
                charactersInRange.emplace_back(c, path);
            }
        }
    }
    return charactersInRange;
}

std::vector<int> GetAdjacentCharacters(SpriteData& spriteData, CharacterData& charData, Level &level, int character, CharacterFaction faction) {
    Vector2i charPos = GetCharacterSpritePosI(spriteData, charData.sprite[character]);
    Vector2i charGridPos = PixelToGridPositionI(charPos.x, charPos.y);
    std::vector<int> charactersInRange;
    for(auto &c : level.allCharacters) {
        // skip same and death characters
        if(c == character || charData.stats[c].health <= 0 || charData.faction[c] != faction) {
            continue;
        }
        Vector2i cCharPos = GetCharacterSpritePosI(spriteData, charData.sprite[c]);
        Vector2i cGridPos = PixelToGridPositionI(cCharPos.x, cCharPos.y);
        if(abs(cGridPos.x - charGridPos.x) <= 1 && abs(cGridPos.y - charGridPos.y) <= 1) {
            charactersInRange.push_back(c);
        }
    }
    return charactersInRange;
}
