//
// Created by bison on 20-01-25.
//

#ifndef SANDBOX_AI_H
#define SANDBOX_AI_H

#include "level/PlayField.h"

struct AiInterface {
    void (*HandleTurn)(CharacterData& charData, Level &combat, PlayField &gridState);
};

void CreateAiInterface(const std::string& name, void (*HandleTurn)(CharacterData& charData, Level& level, PlayField& gridState));
AiInterface* GetAiInterface(const std::string& name);
void HandleTurn(AiInterface& ai, CharacterData& charData, Level& level, PlayField& gridState);
std::vector<int> GetCharactersWithinAttackRange(CharacterData& charData, Level &level, int character, int range, CharacterFaction faction);
std::vector<std::pair<int, Path>> GetCharactersWithinMoveRange(CharacterData& charData, Level &level, int character, int attackRange, bool checkPoints, CharacterFaction faction);
std::vector<std::pair<int, Path>> GetCharactersWithinMoveRangePartial(CharacterData& charData, Level &level, int character, int attackRange, bool checkPoints, CharacterFaction faction);
void SortCharactersByThreat(Level& level, std::vector<int>& characters);
void SortCharactersByThreat(Level& level, std::vector<std::pair<int, Path>>& characters);
std::vector<int> GetAdjacentCharacters(CharacterData& charData, Level &level, int character, CharacterFaction faction);

#endif //SANDBOX_AI_H
