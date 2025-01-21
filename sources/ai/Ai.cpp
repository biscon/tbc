//
// Created by bison on 20-01-25.
//

#include <algorithm>
#include "Ai.h"

static std::map<std::string, AiInterface> aiInterfaces;

void CreateAiInterface(const std::string& name, void (*HandleTurn)(CombatState &, GridState &)) {
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

void HandleTurn(AiInterface &ai, CombatState &combat, GridState &gridState) {
    ai.HandleTurn(combat, gridState);
}

void SortCharactersByThreat(CombatState& combat, std::vector<Character*>& characters) {
    // Sort the characters vector using a custom comparator based on the threatTable in CombatState
    std::sort(characters.begin(), characters.end(), [&combat](Character* a, Character* b) {
        int threatA = combat.threatTable.count(a) ? combat.threatTable[a] : 0;
        int threatB = combat.threatTable.count(b) ? combat.threatTable[b] : 0;
        return threatA > threatB; // Higher threat values come first
    });
}

void SortCharactersByThreat(CombatState& combat, std::vector<std::pair<Character*, Path>>& characters) {
    // Sort the characters vector using a custom comparator based on the threatTable in CombatState
    std::sort(characters.begin(), characters.end(), [&combat](const std::pair<Character*, Path>& a, const std::pair<Character*, Path>& b) {
        int threatA = combat.threatTable.count(a.first) ? combat.threatTable[a.first] : 0;
        int threatB = combat.threatTable.count(b.first) ? combat.threatTable[b.first] : 0;
        return threatA > threatB; // Higher threat values come first
    });
}

static std::vector<Character*> GetCharactersWithinAttackRange(CombatState &combat, Character &character, int range, std::vector<Character*> &characters) {
    // loop through all characters in combat
    Vector2i charGridPos = PixelToGridPositionI(character.sprite.player.position.x, character.sprite.player.position.y);
    std::vector<Character*> charactersInRange;
    for(auto &c : characters) {
        // skip same and death characters
        if(c == &character || c->health <= 0) {
            continue;
        }
        Path path;
        Vector2i cGridPos = PixelToGridPositionI(c->sprite.player.position.x, c->sprite.player.position.y);
        if(InitPathWithRange(combat, path, charGridPos, cGridPos, range, &character)) {
            if(path.cost <= range) {
                charactersInRange.push_back(c);
            }
        }
    }
    return charactersInRange;
}

std::vector<Character*> GetPlayersWithinAttackRange(CombatState &combat, Character &character, int range) {
    return GetCharactersWithinAttackRange(combat, character, range, combat.playerCharacters);
}

std::vector<Character*> GetEnemiesWithinAttackRange(CombatState &combat, Character &character, int range) {
    return GetCharactersWithinAttackRange(combat, character, range, combat.enemyCharacters);
}

static std::vector<std::pair<Character*, Path>> GetCharactersWithinMoveRange(CombatState &combat, Character &character, int attackRange, std::vector<Character*> &characters, bool checkPoints) {
    // loop through all characters in combat
    Vector2i charGridPos = PixelToGridPositionI(character.sprite.player.position.x, character.sprite.player.position.y);
    std::vector<std::pair<Character*, Path>> charactersInRange;
    for(auto &c : characters) {
        // skip same and death characters
        if(c == &character || c->health <= 0) {
            continue;
        }
        Path path;
        Vector2i cGridPos = PixelToGridPositionI(c->sprite.player.position.x, c->sprite.player.position.y);
        if(InitPathWithRange(combat, path, charGridPos, cGridPos, attackRange, &character)) {
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

std::vector<std::pair<Character*, Path>> GetPlayersWithinMoveRange(CombatState &combat, Character &character, int attackRange, bool checkPoints) {
    return GetCharactersWithinMoveRange(combat, character, attackRange, combat.playerCharacters, checkPoints);
}

std::vector<std::pair<Character*, Path>> GetEnemiesWithinMoveRange(CombatState &combat, Character &character, int attackRange, bool checkPoints) {
    return GetCharactersWithinMoveRange(combat, character, attackRange, combat.enemyCharacters, checkPoints);
}

std::vector<Character *> GetAdjacentEnemies(CombatState &combat, Character &character) {
    Vector2i charGridPos = PixelToGridPositionI(character.sprite.player.position.x, character.sprite.player.position.y);
    std::vector<Character*> charactersInRange;
    for(auto &c : combat.enemyCharacters) {
        // skip same and death characters
        if(c == &character || c->health <= 0) {
            continue;
        }
        Vector2i cGridPos = PixelToGridPositionI(c->sprite.player.position.x, c->sprite.player.position.y);
        if(abs(cGridPos.x - charGridPos.x) <= 1 && abs(cGridPos.y - charGridPos.y) <= 1) {
            charactersInRange.push_back(c);
        }
    }
    return charactersInRange;
}

std::vector<Character *> GetAdjacentPlayers(CombatState &combat, Character &character) {
    Vector2i charGridPos = PixelToGridPositionI(character.sprite.player.position.x, character.sprite.player.position.y);
    std::vector<Character*> charactersInRange;
    for(auto &c : combat.playerCharacters) {
        // skip same and death characters
        if(c == &character || c->health <= 0) {
            continue;
        }
        Vector2i cGridPos = PixelToGridPositionI(c->sprite.player.position.x, c->sprite.player.position.y);
        if(abs(cGridPos.x - charGridPos.x) <= 1 && abs(cGridPos.y - charGridPos.y) <= 1) {
            charactersInRange.push_back(c);
        }
    }
    return charactersInRange;
}
