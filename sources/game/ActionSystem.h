//
// Created by bison on 19-11-25.
//

#ifndef SANDBOX_ACTIONSYSTEM_H
#define SANDBOX_ACTIONSYSTEM_H

// -----------------------------------------------------------------------------
// PUBLISH HELPERS (same API you had, but clean)
// -----------------------------------------------------------------------------

#include "data/ActionData.h"
#include "data/GameData.h"

void PushMoveParty(ActionQueue& q, Vector2i target);
void PushPartySpotted(ActionQueue& q, int spotter);
void PushEndCombat(ActionQueue& q, bool victory);
void PushExitLevel(ActionQueue& q, const std::string& levelFile, const std::string& spawnPoint);
void PushInitiateDialogue(ActionQueue& q, int npcId, int dialogueNodeId);
void PushEndDialogue(ActionQueue& q, int npcId);
void PushStartQuest(ActionQueue& q, const std::string& questId);
void PushOpenInventory(ActionQueue& q, int charId);
void PushCloseInventory(ActionQueue& q);
void PushOpenMenu(ActionQueue& q);
void PushOpenActionBar(ActionQueue& q);
void PushCloseActionBar(ActionQueue& q);
void PushOpenLootInventory(ActionQueue& q, int invId);
void PushCloseLootInventory(ActionQueue& q);

bool ProcessActions(GameData& data, Level& level, LevelSystemData& playField, ActionQueue& queue, float dt);

#endif //SANDBOX_ACTIONSYSTEM_H
