//
// Created by bison on 25-06-25.
//

#include <fstream>
#include "Items.h"

static GameData* game = nullptr;

void InitItemData(GameData& data, const std::string &filename) {
    game = nullptr;
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
    itemData.partyInventoryId = CreateInventory(data, 100);
    //itemData.inventoryData[itemData.partyInventoryId].items.push_back(CreateItem(data, "item_weapon_staff", 1));
    //itemData.inventoryData[itemData.partyInventoryId].items.push_back(CreateItem(data, "item_weapon_bow", 1));
}

int CreateItem(GameData& data, const std::string& templateId, int quantity) {
    int id = (int) data.itemData.instanceData.size();
    int templateIndex = data.itemData.templateIdToIndex.at(templateId);
    ItemInstance instance{};
    instance.id = id;
    instance.quantity = quantity;
    instance.templateId = templateIndex;
    data.itemData.instanceData.push_back(instance);
    return id;
}

int CreateItem(GameData& data, int templateId, int quantity) {
    int id = (int) data.itemData.instanceData.size();
    ItemInstance instance{};
    instance.id = id;
    instance.quantity = quantity;
    instance.templateId = templateId;
    data.itemData.instanceData.push_back(instance);
    return id;
}


int GetItemTypeTemplateId(GameData &data, int itemId) {
    int templateId = data.itemData.instanceData.at(itemId).templateId;
    return data.itemData.templateData[templateId].typeTemplateId;
}

std::string GetItemTemplateIdString(GameData& data, int itemId) {
    int templateId = data.itemData.instanceData[itemId].templateId;
    return data.itemData.indexToTemplateId.at(templateId);
}

void to_json(nlohmann::json &j, const Inventory &inventory) {
    j = nlohmann::json{
            {"id", inventory.id},
            {"capacity", inventory.capacity}
    };
    nlohmann::json jItems;
    for(const int& itemId : inventory.items) {
        ItemInstance& inst = game->itemData.instanceData[itemId];
        jItems.push_back(inst);
    }
    j["items"] = jItems;
}

void from_json(const nlohmann::json &j, Inventory &inventory) {
    j.at("id").get_to(inventory.id);
    j.at("capacity").get_to(inventory.capacity);
    nlohmann::json jItems = j.at("items");
    for(const auto& jItem : jItems) {
        auto inst = jItem.get<ItemInstance>();
        int newItemId = CreateItem(*game, inst.templateId, inst.quantity);
        inventory.items.push_back(newItemId);
    }
}

int CreateInventory(GameData &data, int capacity) {
    int id = (int) data.itemData.inventoryData.size();
    Inventory instance{};
    instance.id = id;
    instance.capacity = capacity;
    data.itemData.inventoryData.push_back(instance);
    return id;
}

InventorySaveState InventoryToSaveState(GameData& data, int invId) {
    const Inventory& inventory = data.itemData.inventoryData[invId];
    InventorySaveState state;
    state.capacity = inventory.capacity;
    for(const int& itemId : inventory.items) {
        std::string templateId = GetItemTemplateIdString(data, itemId);
        state.itemTemplateIds.push_back(templateId);
    }
    return state;
}

int InventoryFromSaveState(GameData& data, const InventorySaveState& state) {
    int invId = CreateInventory(data, state.capacity);
    for(const auto& itemTemplateId : state.itemTemplateIds) {
        int itemId = CreateItem(data, itemTemplateId, 1);
        data.itemData.inventoryData[invId].items.push_back(itemId);
    }
    return invId;
}
