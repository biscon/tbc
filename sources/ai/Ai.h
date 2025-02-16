//
// Created by bison on 20-01-25.
//

#ifndef SANDBOX_AI_H
#define SANDBOX_AI_H

#include "ui/Grid.h"

struct AiInterface {
    void (*HandleTurn)(LevelState &combat, GridState &gridState);
};

void CreateAiInterface(const std::string& name, void (*HandleTurn)(LevelState& combat, GridState& gridState));
AiInterface* GetAiInterface(const std::string& name);
void HandleTurn(AiInterface& ai, LevelState& combat, GridState& gridState);
std::vector<Character*> GetPlayersWithinAttackRange(LevelState &combat, Character &character, int range);
std::vector<Character*> GetEnemiesWithinAttackRange(LevelState &combat, Character &character, int range);
std::vector<std::pair<Character*, Path>> GetPlayersWithinMoveRange(LevelState &combat, Character &character, int attackRange, bool checkPoints);
std::vector<std::pair<Character*, Path>> GetEnemiesWithinMoveRange(LevelState &combat, Character &character, int attackRange, bool checkPoints);
std::vector<std::pair<Character*, Path>> GetPlayersWithinMoveRangePartial(LevelState &combat, Character &character, int attackRange, bool checkPoints);
std::vector<std::pair<Character*, Path>> GetEnemiesWithinMoveRangePartial(LevelState &combat, Character &character, int attackRange, bool checkPoints);
void SortCharactersByThreat(LevelState& combat, std::vector<Character*>& characters);
void SortCharactersByThreat(LevelState& combat, std::vector<std::pair<Character*, Path>>& characters);
std::vector<Character*> GetAdjacentEnemies(LevelState &combat, Character &character);
std::vector<Character*> GetAdjacentPlayers(LevelState &combat, Character &character);

#endif //SANDBOX_AI_H
