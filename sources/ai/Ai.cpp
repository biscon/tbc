//
// Created by bison on 20-01-25.
//

#include <algorithm>
#include "Ai.h"
#include "PathFinding.h"
#include "graphics/CharacterSprite.h"
#include "raymath.h"

static std::map<std::string, AiInterface> aiInterfaces;

void CreateAiInterface(const std::string& name, void (*HandleTurn)(GameData&, Level &, PlayField &),
                       void (*StartTurn)(GameData&, Level &, PlayField &)) {
    AiInterface ai{};
    ai.HandleTurn = HandleTurn;
    ai.StartTurn = StartTurn;
    aiInterfaces[name] = ai;
}

AiInterface* GetAiInterface(const std::string& name) {
    auto it = aiInterfaces.find(name);
    if (it != aiInterfaces.end()) {
        return &it->second; // Return a pointer to the found value
    }
    return nullptr; // Key not found, return nullptr
}

void HandleTurn(AiInterface &ai, GameData& data, Level &level, PlayField &playField) {
    ai.HandleTurn(data, level, playField);
}

void StartTurn(AiInterface &ai, GameData& data, Level &level, PlayField &playField) {
    ai.StartTurn(data, level, playField);
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

std::vector<int> GetCharactersWithinAttackRange(GameData& data, Level &level, int character, int range, CharacterFaction faction) {
    // loop through all characters in combat
    Vector2i charPos = GetCharacterSpritePosI(data.spriteData, data.charData.sprite[character]);
    Vector2i charGridPos = PixelToGridPositionI(charPos.x, charPos.y);
    std::vector<int> charactersInRange;
    for(auto &c : level.allCharacters) {
        // skip same and death characters
        if(c == character || data.charData.stats[c].HP <= 0 || data.charData.faction[c] != faction) {
            continue;
        }
        Path path;
        Vector2i cCharPos = GetCharacterSpritePosI(data.spriteData, data.charData.sprite[c]);
        Vector2i cGridPos = PixelToGridPositionI(cCharPos.x, cCharPos.y);
        if(CalcPathWithRange(data.spriteData, data.charData, level, path, charGridPos, cGridPos, range, character, IsTileOccupied)) {
            if(path.cost <= range) {
                charactersInRange.push_back(c);
            }
        }
    }
    return charactersInRange;
}

std::vector<std::pair<int, Path>> GetCharactersWithinMoveRange(GameData& data, Level &level, int character, int attackRange, bool checkPoints, CharacterFaction faction) {
    // loop through all characters in combat
    Vector2i charPos = GetCharacterSpritePosI(data.spriteData, data.charData.sprite[character]);
    Vector2i charGridPos = PixelToGridPositionI(charPos.x, charPos.y);
    std::vector<std::pair<int, Path>> charactersInRange;
    for(auto &c : level.allCharacters) {
        // skip same and death characters
        if(c == character || data.charData.stats[c].HP <= 0 || data.charData.faction[c] != faction) {
            continue;
        }
        Path path;
        Vector2i cCharPos = GetCharacterSpritePosI(data.spriteData, data.charData.sprite[c]);
        Vector2i cGridPos = PixelToGridPositionI(cCharPos.x, cCharPos.y);
        if(CalcPathWithRange(data.spriteData, data.charData, level, path, charGridPos, cGridPos, attackRange, character, IsTileOccupied)) {
            if(checkPoints) {
                if (path.cost <= data.charData.stats[character].AP) {
                    charactersInRange.emplace_back(c, path);
                }
            } else {
                charactersInRange.emplace_back(c, path);
            }
        }
    }
    return charactersInRange;
}

std::vector<std::pair<int, Path>> GetCharactersWithinMoveRangePartial(GameData& data, Level &level, int character, int attackRange, bool checkPoints, CharacterFaction faction) {
    // loop through all characters in combat
    Vector2i charPos = GetCharacterSpritePosI(data.spriteData, data.charData.sprite[character]);
    Vector2i charGridPos = PixelToGridPositionI(charPos.x, charPos.y);
    std::vector<std::pair<int, Path>> charactersInRange;
    for(auto &c : level.allCharacters) {
        // skip same and death characters
        if(c == character || data.charData.stats[c].HP <= 0 || data.charData.faction[c] != faction) {
            continue;
        }
        Path path;
        Vector2i cCharPos = GetCharacterSpritePosI(data.spriteData, data.charData.sprite[c]);
        Vector2i cGridPos = PixelToGridPositionI(cCharPos.x, cCharPos.y);
        CalcPathWithRangePartial(data.spriteData, data.charData, level, path, charGridPos, cGridPos, attackRange, character, IsTileOccupied);
        if(!path.path.empty()) {
            if(checkPoints) {
                if (path.cost <= data.charData.stats[character].AP) {
                    charactersInRange.emplace_back(c, path);
                }
            } else {
                charactersInRange.emplace_back(c, path);
            }
        }
    }
    return charactersInRange;
}

std::vector<int> GetAdjacentCharacters(GameData& data, Level &level, int character, CharacterFaction faction) {
    Vector2i charPos = GetCharacterSpritePosI(data.spriteData, data.charData.sprite[character]);
    Vector2i charGridPos = PixelToGridPositionI(charPos.x, charPos.y);
    std::vector<int> charactersInRange;
    for(auto &c : level.allCharacters) {
        // skip same and death characters
        if(c == character || data.charData.stats[c].HP <= 0 || data.charData.faction[c] != faction) {
            continue;
        }
        Vector2i cCharPos = GetCharacterSpritePosI(data.spriteData, data.charData.sprite[c]);
        Vector2i cGridPos = PixelToGridPositionI(cCharPos.x, cCharPos.y);
        if(abs(cGridPos.x - charGridPos.x) <= 1 && abs(cGridPos.y - charGridPos.y) <= 1) {
            charactersInRange.push_back(c);
        }
    }
    return charactersInRange;
}

bool IsAdjacentToCharacter(GameData& data, Level &level, Vector2i gridPos, CharacterFaction faction) {
    std::vector<int> charactersInRange;
    for(auto &c : level.allCharacters) {
        // skip same and death characters
        if(data.charData.stats[c].HP <= 0 || data.charData.faction[c] != faction) {
            continue;
        }
        Vector2i cCharPos = GetCharacterSpritePosI(data.spriteData, data.charData.sprite[c]);
        Vector2i cGridPos = PixelToGridPositionI(cCharPos.x, cCharPos.y);
        if(abs(cGridPos.x - gridPos.x) <= 1 && abs(cGridPos.y - gridPos.y) <= 1) {
            return true;
        }
    }
    return false;
}

float GetDistanceToClosestCharacter(GameData& data, Level& level, Vector2i gridPos, CharacterFaction faction) {
    float distance = INFINITY;
    for(auto &c : level.allCharacters) {
        // skip same and death characters
        if(data.charData.stats[c].HP <= 0 || data.charData.faction[c] != faction) {
            continue;
        }
        Vector2i cCharPos = GetCharacterSpritePosI(data.spriteData, data.charData.sprite[c]);
        Vector2i cGridPos = PixelToGridPositionI(cCharPos.x, cCharPos.y);
        float d = Distance(gridPos, cGridPos);
        if(d < distance) {
            distance = d;
        }
    }
    return distance;
}

int GetClosestCharacter(GameData& data, Level& level, Vector2i gridPos, CharacterFaction faction) {
    float distance = INFINITY;
    int foundChar = -1;
    for(auto &c : level.allCharacters) {
        // skip same and death characters
        if(data.charData.stats[c].HP <= 0 || data.charData.faction[c] != faction) {
            continue;
        }
        Vector2i cCharPos = GetCharacterSpritePosI(data.spriteData, data.charData.sprite[c]);
        Vector2i cGridPos = PixelToGridPositionI(cCharPos.x, cCharPos.y);
        float d = Distance(gridPos, cGridPos);
        if(d < distance) {
            foundChar = c;
        }
    }
    return foundChar;
}


Vector2i ComputeFleeDirection(GameData& data, Level& level, int aiCharId) {
    Vector2 center = {0, 0};
    int count = 0;
    Vector2i aiPos = GetCharacterSpritePosI(data.spriteData, data.charData.sprite[aiCharId]);
    for (auto& pc : level.partyCharacters) {
        Vector2i pcPos = GetCharacterSpritePosI(data.spriteData, data.charData.sprite[pc]);
        if(HasLineOfSight(level, aiPos, pcPos, 16)) {
            center.x += (float) pcPos.x;
            center.y += (float) pcPos.y;
            count++;
        }
    }
    if (count == 0) return {0, 0}; // No enemies seen
    center.x /= (float) count;
    center.y /= (float) count;

    Vector2 fleeVec = Vector2Normalize(Vector2{aiPos.x - center.x, aiPos.y - center.y});
    return ToDirectionVector(fleeVec); // E.g. snap to 8-dir unit vector
}

Vector2i ChooseBestFleeTile(GameData& data,  Level& level, int aiCharId, int maxAP) {
    Vector2i aiPos = GetCharacterGridPosI(data.spriteData, data.charData.sprite[aiCharId]);
    std::vector<Vector2i> reachable = GetReachableTiles(level, aiPos, maxAP);
    Vector2i bestTile = aiPos;
    float bestScore = -INFINITY;

    for (auto& tile : reachable) {
        float distToClosestEnemy = GetDistanceToClosestCharacter(data, level, tile, CharacterFaction::Player);

        if (IsAdjacentToCharacter(data, level, tile, CharacterFaction::Player)) distToClosestEnemy -= 2.0f;
        float score = distToClosestEnemy;

        if (score > bestScore) {
            bestScore = score;
            bestTile = tile;
        }
    }
    return bestTile;
}

std::vector<int> GetCharactersWithinShootingRange(GameData &data, Level &level, int charId, int range, CharacterFaction faction) {
    Vector2i charPos = GetCharacterGridPosI(data.spriteData, data.charData.sprite[charId]);

    // Pair distance with character ID
    std::vector<std::pair<float, int>> charactersWithDistance;

    for (int c : level.allCharacters) {
        // Skip dead characters and wrong faction
        if (c == charId || data.charData.stats[c].HP <= 0 || data.charData.faction[c] != faction) {
            continue;
        }
        // Skip if no LoS
        if (!HasLineOfSight(data, level, charId, c, range)) {
            continue;
        }

        Vector2i otherCharPos = GetCharacterGridPosI(data.spriteData, data.charData.sprite[c]);
        float d = Distance(charPos, otherCharPos);

        if (d <= range) {
            charactersWithDistance.emplace_back(d, c);
        }
    }

    // Sort by distance (nearest first)
    std::sort(charactersWithDistance.begin(), charactersWithDistance.end());

    // Extract only the character IDs
    std::vector<int> sortedCharacters;
    for (const auto& [dist, id] : charactersWithDistance) {
        sortedCharacters.push_back(id);
    }

    return sortedCharacters;
}

