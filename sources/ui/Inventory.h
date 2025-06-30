//
// Created by bison on 26-06-25.
//

#ifndef SANDBOX_INVENTORY_H
#define SANDBOX_INVENTORY_H

#include "data/GameData.h"
#include "util/GameEventQueue.h"

void InitInventory(GameData& data);
void RenderInventoryUI(GameData& data);
void UpdateInventory(GameData& data, float dt);
bool HandleInventoryInput(GameData& data, GameEventQueue& eventQueue); // returns true if it "swallowed" the input


#endif //SANDBOX_INVENTORY_H
