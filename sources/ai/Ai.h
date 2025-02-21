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
std::vector<Character*> GetPlayersWithinAttackRange(Level &combat, Character &character, int range);
std::vector<Character*> GetEnemiesWithinAttackRange(Level &combat, Character &character, int range);
std::vector<std::pair<Character*, Path>> GetPlayersWithinMoveRange(Level &combat, Character &character, int attackRange, bool checkPoints);
std::vector<std::pair<Character*, Path>> GetEnemiesWithinMoveRange(Level &combat, Character &character, int attackRange, bool checkPoints);
std::vector<std::pair<Character*, Path>> GetPlayersWithinMoveRangePartial(Level &combat, Character &character, int attackRange, bool checkPoints);
std::vector<std::pair<Character*, Path>> GetEnemiesWithinMoveRangePartial(Level &combat, Character &character, int attackRange, bool checkPoints);
void SortCharactersByThreat(Level& combat, std::vector<Character*>& characters);
void SortCharactersByThreat(Level& combat, std::vector<std::pair<Character*, Path>>& characters);
std::vector<Character*> GetAdjacentEnemies(Level &combat, Character &character);
std::vector<Character*> GetAdjacentPlayers(Level &combat, Character &character);

#endif //SANDBOX_AI_H
