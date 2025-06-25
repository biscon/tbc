//
// Created by bison on 25-06-25.
//

#ifndef SANDBOX_ITEMS_H
#define SANDBOX_ITEMS_H

#include <string>
#include "data/GameData.h"

void InitItemData(GameData& data, const std::string &filename);
int CreateItem(GameData& data, const std::string& templateId, int quantity);
int GetItemTypeTemplateId(GameData& data, int itemId);
std::string GetItemTemplateIdString(GameData& data, int itemId);

#endif //SANDBOX_ITEMS_H
