//
// Created by bison on 26-06-25.
//

#include "Inventory.h"
#include "raymath.h"
#include "ui/UI.h"
#include "game/Items.h"
#include "Icons.h"
#include "SharedInventoryUI.h"



static const Rectangle invRect = {140, 8, gameScreenWidth - 200, gameScreenHeight - 100};
static const Rectangle charInfoRect = {8, 8, 125, gameScreenHeight - 100};

void InitInventory(GameData& data) {

    InitInventoryListState(data.ui.inventory.list, invRect, "Party Inventory:");

    data.ui.inventory.buttons.clear();

    Button closeButton{};
    closeButton.label = "Close";
    closeButton.region = CreateClickRegion({invRect.x + invRect.width - 40 - 5, invRect.y + invRect.height - 18, 40, 11});
    closeButton.enabled = true;
    closeButton.hovered = false;
    data.ui.inventory.buttons["close"] = closeButton;

    data.ui.inventory.contextButtons.clear();
    data.ui.inventory.weapon1Region = CreateClickRegion({charInfoRect.x + 5, 88, charInfoRect.width - 10, 9});
    data.ui.inventory.weapon2Region = CreateClickRegion({charInfoRect.x + 5, 98, charInfoRect.width - 10, 9});

    data.ui.inventory.leftTabRegion = CreateClickRegion({charInfoRect.x + 5, 76, 10, 10});
    data.ui.inventory.rightTabRegion = CreateClickRegion({charInfoRect.x + charInfoRect.width - 15, 76, 10, 10});
}

static void UpdateContextButtons(GameData& data) {
    data.ui.inventory.contextButtons.clear();
    if(data.ui.inventory.list.selectedIndex == -1)
        return;

    Button equipButton{};
    equipButton.label = "Equip";
    equipButton.region = CreateClickRegion({invRect.x + 5, invRect.y + invRect.height - 18, 40, 11});
    equipButton.enabled = true;
    equipButton.hovered = false;
    data.ui.inventory.contextButtons["equip"] = equipButton;
}

void UpdateInventory(GameData& data, float dt) {
    UpdateInventoryListState(data, data.ui.inventory.list, data.itemData.partyInventoryId);
}

static void RenderStatsTab(GameData& data, int charId) {
    RenderCharacterStats(data.charData, charId, charInfoRect.x + 5, 90, charInfoRect.width - 10, data.smallFont1, 5);
}

static void RenderSkillsTab(GameData& data, int charId) {
    RenderCharacterSkills(data.charData, charId, charInfoRect.x + 5, 90, charInfoRect.width - 10, data.smallFont1, 5);
}

static void RenderEquipmentTab(GameData& data, int charId) {
    int ypos = 90;
    for (size_t i = 0; i < static_cast<size_t>(ItemEquipSlot::COUNT); ++i) {
        int itemId = data.charData.equippedItemIdx[charId][i];
        ItemEquipSlot slot = static_cast<ItemEquipSlot>(i);
        std::string slotName = GetEquipSlotName(slot);
        bool selectedSlot = data.charData.selectedWeaponSlot[charId] == i;
        bool hovered = false;

        if(data.ui.inventory.weapon1Region.hovered && i == static_cast<int>(ItemEquipSlot::Weapon1)) {
            hovered = true;
        }
        if(data.ui.inventory.weapon2Region.hovered && i == static_cast<int>(ItemEquipSlot::Weapon2)) {
            hovered = true;
        }

        DrawTextEx(data.smallFont1, slotName.c_str(), { charInfoRect.x + 5, (float) ypos}, 5, 1, selectedSlot ? YELLOW : (hovered ? WHITE : LIGHTGRAY));
        std::string itemName = "none";

        Color itemColor = selectedSlot ? YELLOW : (hovered ? WHITE : GRAY);
        if(itemId != -1) {
            ItemTemplate& tmpl = data.itemData.templateData[GetItemTemplateId(data, itemId)];
            itemName = tmpl.name;
        }
        Vector2 textDims = MeasureTextEx(data.smallFont1, itemName.c_str(), 5.0f, 1.0f);
        DrawTextEx(data.smallFont1, itemName.c_str(), { charInfoRect.x + charInfoRect.width - textDims.x - 5, (float) ypos}, 5, 1, itemColor);
        ypos += 10;
    }
}

static void RenderCharacterInfo(GameData& data) {
    int charId = data.party[data.ui.selectedCharacter];
    auto& sprite = data.charData.sprite[charId];

    DrawRectangleRec(charInfoRect, Color{15, 15, 15, 230});
    DrawRectangleRoundedLinesEx(charInfoRect, 0.03f, 4, 1.0f, DARKGRAY);
    float halfWidth = charInfoRect.width/2;
    float halfHeight = charInfoRect.height/2;
    //DrawTextEx(data.smallFont1, data.charData.name[charId].c_str(), {charInfoRect.x + 5, charInfoRect.y + 6}, 5, 1, WHITE);
    int nameWidth = MeasureText(data.charData.name[charId].c_str(), 10);
    DrawText(data.charData.name[charId].c_str(), charInfoRect.x + halfWidth - (nameWidth/2), charInfoRect.y + 6, 10, WHITE);

    //DrawLine(charInfoRect.x + 5, 28, charInfoRect.x + charInfoRect.width - 5, 28, DARKGRAY);

    StartPausedCharacterSpriteAnim(data.spriteData, sprite, SpriteAnimationType::WalkRight, true);
    DrawCharacterSpriteScaled(data.spriteData, sprite, floorf(charInfoRect.x + 40), charInfoRect.y + 60, 2.0f);

    StartPausedCharacterSpriteAnim(data.spriteData, sprite, SpriteAnimationType::WalkDown, true);
    DrawCharacterSpriteScaled(data.spriteData, sprite, floorf(charInfoRect.x + charInfoRect.width - 40), charInfoRect.y + 60, 2.0f);

    // restore
    SpriteAnimationType animType = CharacterOrientationToAnimType(data, charId);
    StartPausedCharacterSpriteAnim(data.spriteData, sprite, animType, true);

    DrawLine(charInfoRect.x + 5, 72, charInfoRect.x + charInfoRect.width - 5, 72, DARKGRAY);

    std::string tabText;
    switch(data.ui.inventory.currentCharTab) {
        case CharacterTabs::Stats:tabText = "Stats";break;
        case CharacterTabs::Equipment:tabText = "Equipment";break;
        case CharacterTabs::Skills:tabText = "Skills";break;
    }
    int labelWidth = MeasureText(tabText.c_str(), 10);
    DrawText(tabText.c_str(), charInfoRect.x + halfWidth - (labelWidth/2), 76, 10, WHITE);

    DrawIcon(data,
             (int) data.ui.inventory.leftTabRegion.rect.x,
             (int) data.ui.inventory.leftTabRegion.rect.y,
             data.ui.inventory.leftTabRegion.hovered ? WHITE : GRAY,
             ICON_LEFT);

    DrawIcon(data,
             (int) data.ui.inventory.rightTabRegion.rect.x+5,
             (int) data.ui.inventory.rightTabRegion.rect.y,
             data.ui.inventory.rightTabRegion.hovered ? WHITE : GRAY,
             ICON_RIGHT);



    switch(data.ui.inventory.currentCharTab) {
        case CharacterTabs::Stats:RenderStatsTab(data, charId);break;
        case CharacterTabs::Equipment:RenderEquipmentTab(data, charId);break;
        case CharacterTabs::Skills:RenderSkillsTab(data, charId);break;
    }
}

void RenderInventoryUI(GameData& data) {
    RenderInventoryListUI(data, data.ui.inventory.list, data.itemData.partyInventoryId);
    RenderButtons(data.ui.inventory.buttons, data.smallFont1, 5.0f);
    RenderButtons(data.ui.inventory.contextButtons, data.smallFont1, 5.0f);
    RenderCharacterInfo(data);
    RenderInventoryListToolTips(data, data.ui.inventory.list, data.itemData.partyInventoryId);
}

static void EquipSelectedItem(GameData& data) {
    auto& partyInventory = data.itemData.inventoryData[data.itemData.partyInventoryId];
    int itemId = partyInventory.items.at(data.ui.inventory.list.selectedIndex);
    ItemTemplate& tpl = data.itemData.templateData[GetItemTemplateId(data, itemId)];
    int charId = data.ui.selectedCharacter;
    switch(tpl.type) {
        case ItemType::Weapon: {
            ItemEquipSlot selectedSlot = static_cast<ItemEquipSlot>(data.charData.selectedWeaponSlot[charId]);
            int prevItem = GetEquippedItem(data, charId, selectedSlot);
            SetEquippedItem(data, charId, selectedSlot, itemId);
            partyInventory.items.erase(partyInventory.items.begin() + data.ui.inventory.list.selectedIndex);
            if(prevItem != -1) {
                partyInventory.items.push_back(prevItem);
            }
            data.ui.inventory.list.selectedIndex = -1;
            UpdateContextButtons(data);
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
}

bool HandleInventoryInput(GameData& data) {
    Vector2 mouse = GetMousePosition();
    HandleInputButtons(data.ui.inventory.buttons);
    HandleInputButtons(data.ui.inventory.contextButtons);
    if(data.ui.inventory.currentCharTab == CharacterTabs::Equipment) {
        data.ui.inventory.weapon1Region.Update(mouse);
        data.ui.inventory.weapon2Region.Update(mouse);
    }
    data.ui.inventory.leftTabRegion.Update(mouse);
    data.ui.inventory.rightTabRegion.Update(mouse);

    if (IsKeyPressed(KEY_ESCAPE)) {
        PublishCloseInventoryEvent(data.ui.eventQueue);
        return true;
    }


    if (!CheckCollisionPointRec(mouse, invRect) && !CheckCollisionPointRec(mouse, charInfoRect)) return false;

    auto& partyInventory = data.itemData.inventoryData[data.itemData.partyInventoryId];

    if (HandleInputInventoryList(data, data.ui.inventory.list, data.itemData.partyInventoryId)) {
        UpdateContextButtons(data);
        return true;
    }

    if(data.ui.inventory.buttons["close"].region.ConsumeClick()) {
        data.ui.inventory.buttons["close"].hovered = false;
        PublishCloseInventoryEvent(data.ui.eventQueue);
    }

    if(data.ui.inventory.contextButtons["equip"].region.ConsumeClick()) {
        EquipSelectedItem(data);
        data.ui.actionBar.selectedModeIdx = 0;
    }

    if(data.ui.inventory.weapon1Region.ConsumeClick()) {
        SetSelectedWeaponSlot(data, data.ui.selectedCharacter, ItemEquipSlot::Weapon1);
        data.ui.actionBar.selectedModeIdx = 0;
    }

    if(data.ui.inventory.weapon1Region.ConsumeDblClick()) {
        int prevItem = GetEquippedItem(data, data.ui.selectedCharacter, ItemEquipSlot::Weapon1);
        if(prevItem != -1) {
            SetEquippedItem(data, data.ui.selectedCharacter, ItemEquipSlot::Weapon1, -1);
            data.ui.actionBar.selectedModeIdx = 0;
            partyInventory.items.push_back(prevItem);
            data.ui.inventory.list.selectedIndex = -1;
            UpdateContextButtons(data);
        }
    }

    if(data.ui.inventory.weapon2Region.ConsumeClick()) {
        SetSelectedWeaponSlot(data, data.ui.selectedCharacter, ItemEquipSlot::Weapon2);
        data.ui.actionBar.selectedModeIdx = 0;
    }

    if(data.ui.inventory.weapon2Region.ConsumeDblClick()) {
        int prevItem = GetEquippedItem(data, data.ui.selectedCharacter, ItemEquipSlot::Weapon2);
        if(prevItem != -1) {
            SetEquippedItem(data, data.ui.selectedCharacter, ItemEquipSlot::Weapon2, -1);
            data.ui.actionBar.selectedModeIdx = 0;
            partyInventory.items.push_back(prevItem);
            data.ui.inventory.list.selectedIndex = -1;
            UpdateContextButtons(data);
        }
    }

    if(data.ui.inventory.leftTabRegion.ConsumeClick()) {
        int idx = static_cast<int>(data.ui.inventory.currentCharTab);
        idx -= 1;
        if(idx < 0) {
            idx = static_cast<int>(CharacterTabs::COUNT)-1;
        }
        data.ui.inventory.currentCharTab = static_cast<CharacterTabs>(idx);
    }
    if(data.ui.inventory.rightTabRegion.ConsumeClick()) {
        int idx = static_cast<int>(data.ui.inventory.currentCharTab);
        idx += 1;
        if(idx > static_cast<int>(CharacterTabs::COUNT)-1) {
            idx = 0;
        }
        data.ui.inventory.currentCharTab = static_cast<CharacterTabs>(idx);
    }
    return true;
}
