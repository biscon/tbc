//
// Created by bison on 25-06-25.
//

#include <fstream>
#include "Items.h"

void InitItemData(GameData& data, const std::string &filename) {
    SetDataInstancePtrs(&data.itemData, &data.weaponData);
    std::ifstream file(filename);
    if (!file) {
        TraceLog(LOG_ERROR, "No item template data found.");
        std::abort();
    }

    nlohmann::json j;
    file >> j;

    auto& itemData = data.itemData;
    // Clear existing data
    itemData.templateData.clear();
    itemData.instanceData.clear();
    itemData.templateIdToIndex.clear();
    itemData.indexToTemplateId.clear();

    // Ensure JSON is an array
    if (!j.is_array()) {
        TraceLog(LOG_ERROR, "Item data must be a JSON array.");
        std::abort();
    }

    for (const auto& entry : j) {
        try {
            ItemTemplate tmpl = entry.get<ItemTemplate>();
            itemData.templateData.push_back(std::move(tmpl));
        } catch (const std::exception& e) {
            TraceLog(LOG_ERROR, "Failed to load item template: %s", e.what());
        }
    }

    TraceLog(LOG_INFO, "Loaded %zu item templates from %s", itemData.templateData.size(), filename.c_str());
}