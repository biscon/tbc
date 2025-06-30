//
// Created by bison on 11-06-25.
//

#ifndef SANDBOX_DIALOGUE_H
#define SANDBOX_DIALOGUE_H

#include <string>
#include "data/DialogueData.h"
#include "data/GameData.h"
#include "util/GameEventQueue.h"

void InitDialogueData(DialogueData& data, const std::string &filename);
void RenderDialogueUI(GameData& data);
void UpdateDialogue(GameData& data, float dt);
void HandleDialogueInput(GameData& data, GameEventQueue& eventQueue);
void InitiateDialogue(GameData& data, int nodeId, int npcId, GameEventQueue& eventQueue);


#endif //SANDBOX_DIALOGUE_H
