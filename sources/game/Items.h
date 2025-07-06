//
// Created by bison on 25-06-25.
//

#ifndef SANDBOX_ITEMS_H
#define SANDBOX_ITEMS_H

#include <string>
#include "data/GameData.h"

void InitItemData(GameData& data, const std::string &filename);
int CreateItem(GameData& data, const std::string& templateId, int quantity);
int CreateItem(GameData& data, int templateId, int quantity);
int GetItemTypeTemplateId(GameData& data, int itemId);
int GetItemTemplateId(GameData& data, int itemId);
std::string GetItemTemplateIdString(GameData& data, int itemId);

int CreateInventory(GameData& data, int capacity);

InventorySaveState InventoryToSaveState(GameData& data, int invId, SaveData& saveData);
int InventoryFromSaveState(GameData& data, SaveData& saveData, const InventorySaveState& state);

int SaveItemInstanceData(GameData& data, SaveData& saveData, int itemId);

void ApplyItemInstanceSaveState(GameData& data, SaveData& saveData, const ItemInstanceSaveState& itemState, int itemId);

#endif //SANDBOX_ITEMS_H
