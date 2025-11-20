//
// Created by bison on 20-01-25.
//

#ifndef SANDBOX_AI_H
#define SANDBOX_AI_H

#include "data/GameData.h"

enum class AiState {
    Idle,
    Reloading,
    MovingToRange,
    Attacking,
    Retreating,
    Done
};

struct AiInterface {
    void (*HandleTurn)(GameData&, Level &level, LevelSystemData &playField);
    void (*StartTurn)(GameData&, Level &level, LevelSystemData &playField);
};

void CreateAiInterface(const std::string& name, void (*HandleTurn)(GameData& data, Level& level, LevelSystemData& playField),
                       void (*StartTurn)(GameData& data, Level& level, LevelSystemData& playField));
AiInterface* GetAiInterface(const std::string& name);
void HandleTurn(AiInterface& ai, GameData& data, Level& level, LevelSystemData& playField);
void StartTurn(AiInterface &ai, GameData& data, Level &level, LevelSystemData &playField);
std::vector<int> GetCharactersWithinAttackRange(GameData& data, Level &level, int character, int range, CharacterFaction faction);
std::vector<std::pair<int, Path>> GetCharactersWithinMoveRange(GameData& data, Level &level, int character, int attackRange, bool checkPoints, CharacterFaction faction);
std::vector<std::pair<int, Path>> GetCharactersWithinMoveRangePartial(GameData& data, Level &level, int character, int attackRange, bool checkPoints, CharacterFaction faction);
void SortCharactersByThreat(Level& level, std::vector<int>& characters);
void SortCharactersByThreat(Level& level, std::vector<std::pair<int, Path>>& characters);
std::vector<int> GetAdjacentCharacters(GameData& data, Level &level, int character, CharacterFaction faction);
bool IsAdjacentToCharacter(GameData& data, Level &level, Vector2i gridPos, CharacterFaction faction);
float GetDistanceToClosestCharacter(GameData& data, Level& level, Vector2i gridPos, CharacterFaction faction);
int GetClosestCharacter(GameData& data, Level& level, Vector2i gridPos, CharacterFaction faction);
Vector2i ComputeFleeDirection(GameData& data, Level& level, int aiCharId);
Vector2i ChooseBestFleeTile(GameData& data,  Level& level, int aiCharId, int maxAP);
std::vector<int> GetCharactersWithinShootingRange(GameData &data, Level &level, int charId, int range, CharacterFaction faction);


#endif //SANDBOX_AI_H
