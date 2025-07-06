//
// Created by bison on 25-06-25.
//

#include <fstream>
#include "Items.h"
#include "character/Weapon.h"

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
    for(int i = 0; i < 30; i++) {

    }
    itemData.inventoryData[itemData.partyInventoryId].items.push_back(CreateItem(data, "item_weapon_knife", 1));
    itemData.inventoryData[itemData.partyInventoryId].items.push_back(CreateItem(data, "item_weapon_club", 1));
    itemData.inventoryData[itemData.partyInventoryId].items.push_back(CreateItem(data, "item_weapon_pistol", 1));
    itemData.inventoryData[itemData.partyInventoryId].items.push_back(CreateItem(data, "item_weapon_assault_rifle", 1));
}

int CreateItem(GameData& data, const std::string& templateId, int quantity) {
    int templateIndex = data.itemData.templateIdToIndex.at(templateId);
    return CreateItem(data, templateIndex, quantity);
}

int CreateItem(GameData& data, int templateId, int quantity) {
    int id = (int) data.itemData.instanceData.size();
    ItemInstance instance{};
    instance.id = id;
    instance.quantity = quantity;
    instance.templateId = templateId;

    ItemTemplate &itemTemplate = data.itemData.templateData.at(templateId);
    switch (itemTemplate.type) {
        case ItemType::Weapon: {
            instance.typeInstanceId = CreateWeaponInstance(data.weaponData, itemTemplate.typeTemplateId);
            break;
        }
        case ItemType::Consumable:
            break;
        case ItemType::Grenade:
            break;
        case ItemType::KeyItem:
            break;
        case ItemType::Armor:
            break;
    }
    data.itemData.instanceData.push_back(instance);
    return id;
}


int GetItemTypeTemplateId(GameData &data, int itemId) {
    int templateId = data.itemData.instanceData.at(itemId).templateId;
    return data.itemData.templateData[templateId].typeTemplateId;
}

int GetItemTemplateId(GameData &data, int itemId) {
    int templateId = data.itemData.instanceData.at(itemId).templateId;
    return templateId;
}

std::string GetItemTemplateIdString(GameData& data, int itemId) {
    int templateId = data.itemData.instanceData[itemId].templateId;
    return data.itemData.indexToTemplateId.at(templateId);
}


int CreateInventory(GameData &data, int capacity) {
    int id = (int) data.itemData.inventoryData.size();
    Inventory instance{};
    instance.id = id;
    instance.capacity = capacity;
    data.itemData.inventoryData.push_back(instance);
    return id;
}

static void ApplyWeaponInstanceSaveState(GameData& data, SaveData& saveData, const ItemInstanceSaveState& itemState, ItemInstance& itemInstance, ItemTemplate& itemTemplate) {
    WeaponInstance& weaponInstance = data.weaponData.instanceData[itemInstance.typeInstanceId];
    WeaponInstanceSaveState weaponState = saveData.weaponInstances[itemState.instanceDataIdx];
    weaponInstance.currentAmmo = weaponState.currentAmmo;
    weaponInstance.jammed = weaponState.jammed;
}

void ApplyItemInstanceSaveState(GameData& data, SaveData& saveData, const ItemInstanceSaveState& itemState, int itemId) {
    ItemInstance& itemInstance = data.itemData.instanceData[itemId];
    ItemTemplate& itemTemplate = data.itemData.templateData[itemInstance.templateId];
    switch(itemTemplate.type) {
        case ItemType::Weapon:ApplyWeaponInstanceSaveState(data, saveData, itemState, itemInstance, itemTemplate);break;
        case ItemType::Consumable:break;
        case ItemType::Grenade:break;
        case ItemType::KeyItem:break;
        case ItemType::Armor:break;
    }
}

InventorySaveState InventoryToSaveState(GameData& data, int invId, SaveData& saveData) {
    const Inventory& inventory = data.itemData.inventoryData[invId];
    InventorySaveState state;
    state.capacity = inventory.capacity;
    for(const int& itemId : inventory.items) {
        std::string templateId = GetItemTemplateIdString(data, itemId);
        ItemInstanceSaveState itemState;
        itemState.templateId = templateId;
        itemState.instanceDataIdx = SaveItemInstanceData(data, saveData, itemId);
        state.instances.push_back(itemState);
    }
    return state;
}

int InventoryFromSaveState(GameData& data, SaveData& saveData, const InventorySaveState& state) {
    int invId = CreateInventory(data, state.capacity);
    for(const auto& itemState : state.instances) {
        int itemId = CreateItem(data, itemState.templateId, 1);
        ApplyItemInstanceSaveState(data, saveData, itemState, itemId);
        data.itemData.inventoryData[invId].items.push_back(itemId);
    }
    return invId;
}

static int SaveWeaponInstanceData(GameData& data, SaveData& saveData, ItemInstance& itemInstance, ItemTemplate& itemTemplate) {
    WeaponTemplate& weaponTemplate = data.weaponData.templateData[itemTemplate.typeTemplateId];
    WeaponInstance& weaponInstance = data.weaponData.instanceData[itemInstance.typeInstanceId];

    int id = static_cast<int>(saveData.weaponInstances.size());
    WeaponInstanceSaveState state;
    state.templateId = weaponTemplate.name;
    state.jammed = weaponInstance.jammed;
    state.currentAmmo = weaponInstance.currentAmmo;
    saveData.weaponInstances.push_back(state);
    return id;
}

int SaveItemInstanceData(GameData& data, SaveData& saveData, int itemId) {
    ItemInstance& itemInstance = data.itemData.instanceData[itemId];
    ItemTemplate& itemTemplate = data.itemData.templateData[itemInstance.templateId];
    switch(itemTemplate.type) {
        case ItemType::Weapon: return SaveWeaponInstanceData(data, saveData, itemInstance, itemTemplate);
        case ItemType::Consumable:break;
        case ItemType::Grenade:break;
        case ItemType::KeyItem:break;
        case ItemType::Armor:break;
    }
    return -1;
}


