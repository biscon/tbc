//
// Created by bison on 20-01-25.
//

#include <algorithm>
#include "Ai.h"

static std::map<std::string, AiInterface> aiInterfaces;

void CreateAiInterface(const std::string& name, void (*HandleTurn)(Level &, PlayField &)) {
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

void HandleTurn(AiInterface &ai, Level &level, PlayField &gridState) {
    ai.HandleTurn(level, gridState);
}

void SortCharactersByThreat(Level& level, std::vector<Character*>& characters) {
    // Sort the characters vector using a custom comparator based on the threatTable in CombatState
    std::sort(characters.begin(), characters.end(), [&level](Character* a, Character* b) {
        int threatA = level.threatTable.count(a) ? level.threatTable[a] : 0;
        int threatB = level.threatTable.count(b) ? level.threatTable[b] : 0;
        return threatA > threatB; // Higher threat values come first
    });
}

void SortCharactersByThreat(Level& level, std::vector<std::pair<Character*, Path>>& characters) {
    // Sort the characters vector using a custom comparator based on the threatTable in CombatState
    std::sort(characters.begin(), characters.end(), [&level](const std::pair<Character*, Path>& a, const std::pair<Character*, Path>& b) {
        int threatA = level.threatTable.count(a.first) ? level.threatTable[a.first] : 0;
        int threatB = level.threatTable.count(b.first) ? level.threatTable[b.first] : 0;
        return threatA > threatB; // Higher threat values come first
    });
}

std::vector<Character*> GetCharactersWithinAttackRange(Level &level, Character &character, int range, CharacterFaction faction) {
    // loop through all characters in combat
    Vector2 charPos = GetCharacterSpritePos(character.sprite);
    Vector2i charGridPos = PixelToGridPositionI(charPos.x, charPos.y);
    std::vector<Character*> charactersInRange;
    for(auto &c : level.allCharacters) {
        // skip same and death characters
        if(c == &character || c->health <= 0 || c->faction != faction) {
            continue;
        }
        Path path;
        Vector2i cCharPos = GetCharacterSpritePosI(c->sprite);
        Vector2i cGridPos = PixelToGridPositionI(cCharPos.x, cCharPos.y);
        if(CalcPathWithRange(level, path, charGridPos, cGridPos, range, &character, IsTileOccupied)) {
            if(path.cost <= range) {
                charactersInRange.push_back(c);
            }
        }
    }
    return charactersInRange;
}

std::vector<std::pair<Character*, Path>> GetCharactersWithinMoveRange(Level &level, Character &character, int attackRange, bool checkPoints, CharacterFaction faction) {
    // loop through all characters in combat
    Vector2i charPos = GetCharacterSpritePosI(character.sprite);
    Vector2i charGridPos = PixelToGridPositionI(charPos.x, charPos.y);
    std::vector<std::pair<Character*, Path>> charactersInRange;
    for(auto &c : level.allCharacters) {
        // skip same and death characters
        if(c == &character || c->health <= 0 || c->faction != faction) {
            continue;
        }
        Path path;
        Vector2i cCharPos = GetCharacterSpritePosI(c->sprite);
        Vector2i cGridPos = PixelToGridPositionI(cCharPos.x, cCharPos.y);
        if(CalcPathWithRange(level, path, charGridPos, cGridPos, attackRange, &character, IsTileOccupied)) {
            if(checkPoints) {
                if (path.cost <= character.movePoints) {
                    charactersInRange.push_back(std::make_pair(c, path));
                }
            } else {
                charactersInRange.push_back(std::make_pair(c, path));
            }
        }
    }
    return charactersInRange;
}

std::vector<std::pair<Character*, Path>> GetCharactersWithinMoveRangePartial(Level &level, Character &character, int attackRange, bool checkPoints, CharacterFaction faction) {
    // loop through all characters in combat
    Vector2i charPos = GetCharacterSpritePosI(character.sprite);
    Vector2i charGridPos = PixelToGridPositionI(charPos.x, charPos.y);
    std::vector<std::pair<Character*, Path>> charactersInRange;
    for(auto &c : level.allCharacters) {
        // skip same and death characters
        if(c == &character || c->health <= 0 || c->faction != faction) {
            continue;
        }
        Path path;
        Vector2i cCharPos = GetCharacterSpritePosI(c->sprite);
        Vector2i cGridPos = PixelToGridPositionI(cCharPos.x, cCharPos.y);
        CalcPathWithRangePartial(level, path, charGridPos, cGridPos, attackRange, &character, IsTileOccupied);
        if(!path.path.empty()) {
            if(checkPoints) {
                if (path.cost <= character.movePoints) {
                    charactersInRange.push_back(std::make_pair(c, path));
                }
            } else {
                charactersInRange.push_back(std::make_pair(c, path));
            }
        }
    }
    return charactersInRange;
}

std::vector<Character *> GetAdjacentCharacters(Level &level, Character &character, CharacterFaction faction) {
    Vector2i charPos = GetCharacterSpritePosI(character.sprite);
    Vector2i charGridPos = PixelToGridPositionI(charPos.x, charPos.y);
    std::vector<Character*> charactersInRange;
    for(auto &c : level.allCharacters) {
        // skip same and death characters
        if(c == &character || c->health <= 0 || c->faction != faction) {
            continue;
        }
        Vector2i cCharPos = GetCharacterSpritePosI(c->sprite);
        Vector2i cGridPos = PixelToGridPositionI(cCharPos.x, cCharPos.y);
        if(abs(cGridPos.x - charGridPos.x) <= 1 && abs(cGridPos.y - charGridPos.y) <= 1) {
            charactersInRange.push_back(c);
        }
    }
    return charactersInRange;
}
