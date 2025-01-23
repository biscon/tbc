//
// Created by bison on 20-01-25.
//

#ifndef SANDBOX_AI_H
#define SANDBOX_AI_H

#include "CombatState.h"
#include "Grid.h"

struct AiInterface {
    void (*HandleTurn)(CombatState &combat, GridState &gridState);
};

void CreateAiInterface(const std::string& name, void (*HandleTurn)(CombatState& combat, GridState& gridState));
AiInterface* GetAiInterface(const std::string& name);
void HandleTurn(AiInterface& ai, CombatState& combat, GridState& gridState);
std::vector<Character*> GetPlayersWithinAttackRange(CombatState &combat, Character &character, int range);
std::vector<Character*> GetEnemiesWithinAttackRange(CombatState &combat, Character &character, int range);
std::vector<std::pair<Character*, Path>> GetPlayersWithinMoveRange(CombatState &combat, Character &character, int attackRange, bool checkPoints);
std::vector<std::pair<Character*, Path>> GetEnemiesWithinMoveRange(CombatState &combat, Character &character, int attackRange, bool checkPoints);
std::vector<std::pair<Character*, Path>> GetPlayersWithinMoveRangePartial(CombatState &combat, Character &character, int attackRange, bool checkPoints);
std::vector<std::pair<Character*, Path>> GetEnemiesWithinMoveRangePartial(CombatState &combat, Character &character, int attackRange, bool checkPoints);
void SortCharactersByThreat(CombatState& combat, std::vector<Character*>& characters);
void SortCharactersByThreat(CombatState& combat, std::vector<std::pair<Character*, Path>>& characters);
std::vector<Character*> GetAdjacentEnemies(CombatState &combat, Character &character);
std::vector<Character*> GetAdjacentPlayers(CombatState &combat, Character &character);

#endif //SANDBOX_AI_H
