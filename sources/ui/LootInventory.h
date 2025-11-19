//
// Created by bison on 26-06-25.
//

#ifndef SANDBOX_LOOTINVENTORY_H
#define SANDBOX_LOOTINVENTORY_H

#include "data/GameData.h"

void InitLootInventory(GameData& data);
void RenderLootInventoryUI(GameData& data);
void UpdateLootInventory(GameData& data, float dt);
bool HandleLootInventoryInput(GameData& data); // returns true if it "swallowed" the input


#endif
