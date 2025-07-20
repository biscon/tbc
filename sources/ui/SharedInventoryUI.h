//
// Created by Stinus Troels Petersen on 20/07/2025.
//

#ifndef SANDBOX_SHAREDINVENTORYUI_H
#define SANDBOX_SHAREDINVENTORYUI_H

#include "data/UiData.h"
#include "data/GameData.h"

void InitInventoryListState(InventoryListState& list, const Rectangle& invRect, const std::string& name);
void UpdateInventoryListState(GameData& data, InventoryListState& list, int inventoryId);
void RenderInventoryListUI(GameData& data, InventoryListState& list, int inventoryId);
void RenderInventoryListToolTips(GameData& data, InventoryListState& list, int inventoryId);
bool HandleInputInventoryList(GameData& data, InventoryListState& list, int inventoryId);

#endif //SANDBOX_SHAREDINVENTORYUI_H
