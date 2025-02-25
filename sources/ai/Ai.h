//
// Created by bison on 20-01-25.
//

#ifndef SANDBOX_AI_H
#define SANDBOX_AI_H

#include "level/PlayField.h"

struct AiInterface {
    void (*HandleTurn)(Level &combat, PlayField &gridState);
};

void CreateAiInterface(const std::string& name, void (*HandleTurn)(Level& combat, PlayField& gridState));
AiInterface* GetAiInterface(const std::string& name);
void HandleTurn(AiInterface& ai, Level& combat, PlayField& gridState);
std::vector<Character*> GetCharactersWithinAttackRange(Level &combat, Character &character, int range, CharacterFaction faction);
std::vector<std::pair<Character*, Path>> GetCharactersWithinMoveRange(Level &combat, Character &character, int attackRange, bool checkPoints, CharacterFaction faction);
std::vector<std::pair<Character*, Path>> GetCharactersWithinMoveRangePartial(Level &combat, Character &character, int attackRange, bool checkPoints, CharacterFaction faction);
void SortCharactersByThreat(Level& combat, std::vector<Character*>& characters);
void SortCharactersByThreat(Level& combat, std::vector<std::pair<Character*, Path>>& characters);
std::vector<Character*> GetAdjacentCharacters(Level &combat, Character &character, CharacterFaction faction);

#endif //SANDBOX_AI_H
