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

void HandleTurn(AiInterface &ai, Level &combat, PlayField &gridState) {
    ai.HandleTurn(combat, gridState);
}

void SortCharactersByThreat(Level& combat, std::vector<Character*>& characters) {
    // Sort the characters vector using a custom comparator based on the threatTable in CombatState
    std::sort(characters.begin(), characters.end(), [&combat](Character* a, Character* b) {
        int threatA = combat.threatTable.count(a) ? combat.threatTable[a] : 0;
        int threatB = combat.threatTable.count(b) ? combat.threatTable[b] : 0;
        return threatA > threatB; // Higher threat values come first
    });
}

void SortCharactersByThreat(Level& combat, std::vector<std::pair<Character*, Path>>& characters) {
    // Sort the characters vector using a custom comparator based on the threatTable in CombatState
    std::sort(characters.begin(), characters.end(), [&combat](const std::pair<Character*, Path>& a, const std::pair<Character*, Path>& b) {
        int threatA = combat.threatTable.count(a.first) ? combat.threatTable[a.first] : 0;
        int threatB = combat.threatTable.count(b.first) ? combat.threatTable[b.first] : 0;
        return threatA > threatB; // Higher threat values come first
    });
}

std::vector<Character*> GetCharactersWithinAttackRange(Level &combat, Character &character, int range, CharacterFaction faction) {
    // loop through all characters in combat
    Vector2 charPos = GetCharacterSpritePos(character.sprite);
    Vector2i charGridPos = PixelToGridPositionI(charPos.x, charPos.y);
    std::vector<Character*> charactersInRange;
    for(auto &c : combat.allCharacters) {
        // skip same and death characters
        if(c == &character || c->health <= 0 || c->faction != faction) {
            continue;
        }
        Path path;
        Vector2i cCharPos = GetCharacterSpritePosI(c->sprite);
        Vector2i cGridPos = PixelToGridPositionI(cCharPos.x, cCharPos.y);
        if(CalcPathWithRange(combat, path, charGridPos, cGridPos, range, &character, IsTileOccupied)) {
            if(path.cost <= range) {
                charactersInRange.push_back(c);
            }
        }
    }
    return charactersInRange;
}

std::vector<std::pair<Character*, Path>> GetCharactersWithinMoveRange(Level &combat, Character &character, int attackRange, bool checkPoints, CharacterFaction faction) {
    // loop through all characters in combat
    Vector2i charPos = GetCharacterSpritePosI(character.sprite);
    Vector2i charGridPos = PixelToGridPositionI(charPos.x, charPos.y);
    std::vector<std::pair<Character*, Path>> charactersInRange;
    for(auto &c : combat.allCharacters) {
        // skip same and death characters
        if(c == &character || c->health <= 0 || c->faction != faction) {
            continue;
        }
        Path path;
        Vector2i cCharPos = GetCharacterSpritePosI(c->sprite);
        Vector2i cGridPos = PixelToGridPositionI(cCharPos.x, cCharPos.y);
        if(CalcPathWithRange(combat, path, charGridPos, cGridPos, attackRange, &character, IsTileOccupied)) {
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

std::vector<std::pair<Character*, Path>> GetCharactersWithinMoveRangePartial(Level &combat, Character &character, int attackRange, bool checkPoints, CharacterFaction faction) {
    // loop through all characters in combat
    Vector2i charPos = GetCharacterSpritePosI(character.sprite);
    Vector2i charGridPos = PixelToGridPositionI(charPos.x, charPos.y);
    std::vector<std::pair<Character*, Path>> charactersInRange;
    for(auto &c : combat.allCharacters) {
        // skip same and death characters
        if(c == &character || c->health <= 0 || c->faction != faction) {
            continue;
        }
        Path path;
        Vector2i cCharPos = GetCharacterSpritePosI(c->sprite);
        Vector2i cGridPos = PixelToGridPositionI(cCharPos.x, cCharPos.y);
        CalcPathWithRangePartial(combat, path, charGridPos, cGridPos, attackRange, &character, IsTileOccupied);
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

std::vector<Character *> GetAdjacentCharacters(Level &combat, Character &character, CharacterFaction faction) {
    Vector2i charPos = GetCharacterSpritePosI(character.sprite);
    Vector2i charGridPos = PixelToGridPositionI(charPos.x, charPos.y);
    std::vector<Character*> charactersInRange;
    for(auto &c : combat.allCharacters) {
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
