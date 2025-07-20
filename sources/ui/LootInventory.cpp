//
// Created by bison on 26-06-25.
//

#include "LootInventory.h"
#include "raymath.h"
#include "ui/UI.h"
#include "game/Items.h"
#include "Icons.h"
#include "SharedInventoryUI.h"

static const Rectangle invRect = {140, 8, 340, 160};
static const Rectangle invRect2 = {140, 176, 340, 160};

static void UpdateContextButtons(GameData& data) {
    data.ui.lootInventory.contextButtons.clear();
    if(data.ui.lootInventory.list1.selectedIndex != -1) {
        Button takeButton{};
        takeButton.label = "Take";
        takeButton.region = CreateClickRegion({invRect.x + 5, invRect.y + invRect.height - 18, 40, 11});
        takeButton.enabled = true;
        takeButton.hovered = false;
        data.ui.lootInventory.contextButtons["take"] = takeButton;
    }

    if(!data.itemData.inventoryData[data.ui.lootInventory.inventoryId].items.empty()) {
        Button takeAllButton{};
        takeAllButton.label = "Take All";
        takeAllButton.region = CreateClickRegion({invRect.x + invRect.width - 40 - 5, invRect.y + invRect.height - 18, 40, 11});
        takeAllButton.enabled = true;
        takeAllButton.hovered = false;
        data.ui.lootInventory.contextButtons["takeAll"] = takeAllButton;
    }

    if(data.ui.lootInventory.list2.selectedIndex != -1) {
        Button button{};
        button.label = "Put";
        button.region = CreateClickRegion({invRect2.x + 5, invRect2.y + invRect2.height - 18, 40, 11});
        button.enabled = true;
        button.hovered = false;
        data.ui.lootInventory.contextButtons["put"] = button;
    }
}

void InitLootInventory(GameData& data) {
    InitInventoryListState(data.ui.lootInventory.list1, invRect, "Container:");
    InitInventoryListState(data.ui.lootInventory.list2, invRect2, "Party Inventory:");

    data.ui.lootInventory.buttons.clear();

    Button closeButton{};
    closeButton.label = "Close";
    closeButton.region = CreateClickRegion({invRect2.x + invRect2.width - 40 - 5, invRect2.y + invRect2.height - 18, 40, 11});
    closeButton.enabled = true;
    closeButton.hovered = false;
    data.ui.lootInventory.buttons["close"] = closeButton;

    data.ui.lootInventory.contextButtons.clear();
    UpdateContextButtons(data);
}

void UpdateLootInventory(GameData& data, float dt) {
    if(data.ui.lootInventory.inventoryId != -1) {
        UpdateInventoryListState(data, data.ui.lootInventory.list1, data.ui.lootInventory.inventoryId);
    }
    UpdateInventoryListState(data, data.ui.lootInventory.list2, data.itemData.partyInventoryId);
}

void RenderLootInventoryUI(GameData& data) {
    RenderInventoryListUI(data, data.ui.lootInventory.list1, data.ui.lootInventory.inventoryId);
    RenderInventoryListUI(data, data.ui.lootInventory.list2, data.itemData.partyInventoryId);


    RenderButtons(data.ui.lootInventory.buttons, data.smallFont1, 5.0f);
    RenderButtons(data.ui.lootInventory.contextButtons, data.smallFont1, 5.0f);

    RenderInventoryListToolTips(data, data.ui.lootInventory.list1, data.ui.lootInventory.inventoryId);
    RenderInventoryListToolTips(data, data.ui.lootInventory.list2, data.itemData.partyInventoryId);
}

bool HandleLootInventoryInput(GameData& data) {
    Vector2 mouse = GetMousePosition();
    HandleInputButtons(data.ui.lootInventory.buttons);
    HandleInputButtons(data.ui.lootInventory.contextButtons);

    if (IsKeyPressed(KEY_ESCAPE)) {
        PublishCloseLootInventoryEvent(data.ui.eventQueue);
        return true;
    }


    if (!CheckCollisionPointRec(mouse, invRect) && !CheckCollisionPointRec(mouse, invRect2)) return false;

    if (HandleInputInventoryList(data, data.ui.lootInventory.list1, data.ui.lootInventory.inventoryId)) {
        data.ui.lootInventory.list2.selectedIndex = -1;
        UpdateContextButtons(data);
        return true;
    }

    if (HandleInputInventoryList(data, data.ui.lootInventory.list2, data.itemData.partyInventoryId)) {
        data.ui.lootInventory.list1.selectedIndex = -1;
        UpdateContextButtons(data);
        return true;
    }

    if(data.ui.lootInventory.buttons["close"].region.ConsumeClick()) {
        data.ui.lootInventory.buttons["close"].hovered = false;
        PublishCloseLootInventoryEvent(data.ui.eventQueue);
    }

    if(data.ui.lootInventory.contextButtons["takeAll"].region.ConsumeClick()) {
        auto& partyInventory = data.itemData.inventoryData[data.itemData.partyInventoryId];
        auto& lootInventory = data.itemData.inventoryData[data.ui.lootInventory.inventoryId];
        for(auto& itemId : lootInventory.items) {
            partyInventory.items.insert(partyInventory.items.begin(), itemId);
        }
        lootInventory.items.clear();
        data.ui.lootInventory.list1.selectedIndex = -1;
        PublishCloseLootInventoryEvent(data.ui.eventQueue);
    }

    if(data.ui.lootInventory.contextButtons["take"].region.ConsumeClick()) {
        auto& lootInventory = data.itemData.inventoryData[data.ui.lootInventory.inventoryId];
        int& selectedIndex = data.ui.lootInventory.list1.selectedIndex;

        // Get and remove item from loot
        int itemId = lootInventory.items.at(selectedIndex);
        lootInventory.items.erase(lootInventory.items.begin() + selectedIndex);

        // Move item to party inventory
        auto& partyInventory = data.itemData.inventoryData[data.itemData.partyInventoryId];
        partyInventory.items.insert(partyInventory.items.begin(), itemId);

        // Update selection
        if (!lootInventory.items.empty()) {
            if (selectedIndex >= static_cast<int>(lootInventory.items.size())) {
                selectedIndex = static_cast<int>(lootInventory.items.size()) - 1;
            }
            // else keep selectedIndex as-is
        } else {
            selectedIndex = -1;
        }
        if(lootInventory.items.empty()) {
            PublishCloseLootInventoryEvent(data.ui.eventQueue);
        }
    }

    if(data.ui.lootInventory.contextButtons["put"].region.ConsumeClick()) {
        auto& lootInventory = data.itemData.inventoryData[data.ui.lootInventory.inventoryId];
        auto& partyInventory = data.itemData.inventoryData[data.itemData.partyInventoryId];
        int& selectedIndex = data.ui.lootInventory.list2.selectedIndex;

        // Get and remove item from party
        int itemId = partyInventory.items.at(selectedIndex);
        partyInventory.items.erase(partyInventory.items.begin() + selectedIndex);

        // Move item to loot
        lootInventory.items.insert(lootInventory.items.begin(), itemId);

        // Update selection
        if (!partyInventory.items.empty()) {
            if (selectedIndex >= static_cast<int>(partyInventory.items.size())) {
                selectedIndex = static_cast<int>(partyInventory.items.size()) - 1;
            }
            // else keep selectedIndex as-is
        } else {
            selectedIndex = -1;
        }
        if(lootInventory.items.empty()) {
            PublishCloseLootInventoryEvent(data.ui.eventQueue);
        }
    }

    return true;
}
