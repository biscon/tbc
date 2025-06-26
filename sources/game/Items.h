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
std::string GetItemTemplateIdString(GameData& data, int itemId);

int CreateInventory(GameData& data, int capacity);

void to_json(nlohmann::json& j, const Inventory& inventory);
void from_json(const nlohmann::json& j, Inventory& inventory);

InventorySaveState InventoryToSaveState(GameData& data, int invId);
int InventoryFromSaveState(GameData& data, const InventorySaveState& state);

#endif //SANDBOX_ITEMS_H
