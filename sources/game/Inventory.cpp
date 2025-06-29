//
// Created by bison on 26-06-25.
//

#include "Inventory.h"
#include "raymath.h"
#include "ui/UI.h"
#include "Items.h"

static const int separator = 1;
static const int itemHeightPx = 12;
static const Rectangle invRect = {140, 8, gameScreenWidth - 200, gameScreenHeight - 100};
static const int visibleItems = (int)((invRect.height - 50) / itemHeightPx);
static const int firstRowOffset = 16;
static const int rowRightMargin = 20;
static const Rectangle scrollBarRect = {invRect.x + invRect.width - 14, invRect.y + firstRowOffset, 10, invRect.height - firstRowOffset - 24};
static const int scrollbarMinHeight = 16;

void InitInventory(GameData& data) {
    data.ui.inventory.scrollOffset = 0;
    data.ui.inventory.selectedIndex = -1;
    data.ui.inventory.hoveredIndex = -1;
    data.ui.inventory.draggingScrollKnob = false;
    data.ui.inventory.dragOffsetY = 0;

    data.ui.inventory.buttons.clear();

    Button closeButton{};
    closeButton.label = "Close";
    closeButton.region = CreateClickRegion({invRect.x + invRect.width - 40 - 5, invRect.y + invRect.height - 18, 40, 11});
    closeButton.enabled = true;
    closeButton.hovered = false;
    data.ui.inventory.buttons["close"] = closeButton;

    data.ui.inventory.contextButtons.clear();
}

static void UpdateContextButtons(GameData& data) {
    data.ui.inventory.contextButtons.clear();
    if(data.ui.inventory.selectedIndex == -1)
        return;

    Button equipButton{};
    equipButton.label = "Equip";
    equipButton.region = CreateClickRegion({invRect.x + 5, invRect.y + invRect.height - 18, 40, 11});
    equipButton.enabled = true;
    equipButton.hovered = false;
    data.ui.inventory.contextButtons["equip"] = equipButton;
}

void UpdateInventory(GameData& data, float dt) {
    Vector2 mouse = GetMousePosition();

    const auto& partyInventory = data.itemData.inventoryData[data.itemData.partyInventoryId];
    int maxItems = (int) partyInventory.items.size();

    // Scroll input
    if (CheckCollisionPointRec(mouse, invRect)) {
        int wheel = (int) GetMouseWheelMove();
        data.ui.inventory.scrollOffset -= wheel;
        data.ui.inventory.scrollOffset = Clamp(data.ui.inventory.scrollOffset, 0, std::max(0, maxItems - visibleItems));
    }

    // Dragging scrollbar knob
    float scrollRatio = (float)visibleItems / (float)maxItems;
    float knobHeight = Clamp(scrollBarRect.height * scrollRatio, (float)scrollbarMinHeight, scrollBarRect.height);
    float scrollRange = scrollBarRect.height - knobHeight;
    float knobY = scrollBarRect.y + (scrollRange * data.ui.inventory.scrollOffset / std::max(1, maxItems - visibleItems));
    Rectangle knobRect = {scrollBarRect.x, knobY, scrollBarRect.width, knobHeight};

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, knobRect)) {
        data.ui.inventory.draggingScrollKnob = true;
        data.ui.inventory.dragOffsetY = mouse.y - knobY;
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        data.ui.inventory.draggingScrollKnob = false;
    }

    if (data.ui.inventory.draggingScrollKnob) {
        float newKnobY = Clamp(mouse.y - data.ui.inventory.dragOffsetY, scrollBarRect.y, scrollBarRect.y + scrollRange);
        float scrollPercent = (newKnobY - scrollBarRect.y) / scrollRange;
        data.ui.inventory.scrollOffset = (int)(scrollPercent * (maxItems - visibleItems));
    }

    // Hover index
    data.ui.inventory.hoveredIndex = -1;
    float rowY = invRect.y + firstRowOffset;
    for (int i = 0; i < visibleItems; ++i) {
        int idx = i + data.ui.inventory.scrollOffset;
        if (idx >= maxItems) break;
        Rectangle row = {invRect.x + 4, rowY, invRect.width - rowRightMargin, (float) itemHeightPx};
        if (CheckCollisionPointRec(mouse, row)) {
            data.ui.inventory.hoveredIndex = idx;
            break;
        }
        rowY += (float) (itemHeightPx + separator);
    }
}

static void RenderScrollBar(GameData& data, int maxItems) {
    DrawRectangleLinesEx(scrollBarRect, 1, DARKGRAY);
    if (maxItems > visibleItems) {
        float scrollRatio = (float)visibleItems / (float)maxItems;
        float knobHeight = Clamp(scrollBarRect.height * scrollRatio, (float)scrollbarMinHeight, scrollBarRect.height);
        float scrollRange = scrollBarRect.height - knobHeight;
        float knobY = scrollBarRect.y + (scrollRange * data.ui.inventory.scrollOffset / std::max(1, maxItems - visibleItems));
        Rectangle knobRect = {scrollBarRect.x, knobY, scrollBarRect.width, knobHeight};
        DrawRectangleRec(knobRect, DARKGRAY);
        DrawRectangleLinesEx(knobRect, 1, data.ui.inventory.draggingScrollKnob ? YELLOW : LIGHTGRAY);
    }
}

static void RenderCharacterInfo(GameData& data) {
    int charId = data.party[data.ui.selectedCharacter];
    auto& sprite = data.charData.sprite[charId];
    static const Rectangle charInfoRect = {8, 8, 125, gameScreenHeight - 100};

    DrawRectangleRec(charInfoRect, Color{15, 15, 15, 200});
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
    RenderCharacterStats(data.charData, charId, charInfoRect.x + 5, 76, charInfoRect.width - 10, GetFontDefault());

    int labelWidth = MeasureText("Equipment", 10);
    DrawLine(charInfoRect.x + 5, 138, charInfoRect.x + charInfoRect.width - 5, 138, DARKGRAY);
    DrawText("Equipment", charInfoRect.x + halfWidth - (labelWidth/2), 144, 10, WHITE);
    int ypos = 160;

    for (size_t i = 0; i < static_cast<size_t>(ItemEquipSlot::COUNT); ++i) {
        int itemId = data.charData.equippedItemIdx[charId][i];
        std::string slotName = GetEquipSlotName(static_cast<ItemEquipSlot>(i));
        DrawTextEx(data.smallFont1, slotName.c_str(), { charInfoRect.x + 5, (float) ypos}, 5, 1, LIGHTGRAY);
        std::string itemName = "none";
        Color itemColor = GRAY;
        if(itemId != -1) {
            ItemTemplate& tmpl = data.itemData.templateData[GetItemTemplateId(data, itemId)];
            itemName = tmpl.name;
            itemColor = YELLOW;
        }
        Vector2 textDims = MeasureTextEx(data.smallFont1, itemName.c_str(), 5.0f, 1.0f);
        DrawTextEx(data.smallFont1, itemName.c_str(), { charInfoRect.x + charInfoRect.width - textDims.x - 5, (float) ypos}, 5, 1, itemColor);
        ypos += 10;
    }
}

void RenderInventoryUI(GameData& data) {
    Font font = data.smallFont1;
    float fontSize = 5.0f;
    float spacing = 1.0f;

    DrawRectangleRec(invRect, Color{15, 15, 15, 200});
    DrawRectangleRoundedLinesEx(invRect, 0.02f, 4, 1.0f, DARKGRAY);

    DrawTextEx(font, "Party Inventory", {invRect.x + 5, invRect.y + 6}, fontSize, spacing, WHITE);

    const auto& partyInventory = data.itemData.inventoryData[data.itemData.partyInventoryId];
    int maxItems = (int) partyInventory.items.size();

    int selectedChar = data.ui.selectedCharacter;

    float rowY = invRect.y + firstRowOffset;
    for (int i = 0; i < visibleItems; ++i) {
        int idx = i + data.ui.inventory.scrollOffset;
        if (idx >= maxItems) break;

        int itemId = partyInventory.items.at(idx);
        ItemInstance& inst = data.itemData.instanceData[itemId];
        ItemTemplate& tmpl = data.itemData.templateData[inst.templateId];

        Rectangle row = {invRect.x + 4, rowY, invRect.width - rowRightMargin, (float) itemHeightPx};
        bool isSelected = (data.ui.inventory.selectedIndex == idx);
        bool isHovered = (data.ui.inventory.hoveredIndex == idx);

        Color border = isSelected ? YELLOW : (isHovered ? GRAY : DARKGRAY);
        DrawRectangleLinesEx(row, 1, border);

        std::string nameStr = tmpl.name;
        /*
        if (IsItemEquippedByCharacter(inst.id, selectedChar, data)) {
            nameStr += " (E)";
        }
        */
        DrawTextEx(font, nameStr.c_str(), {row.x + 4, row.y + 3}, fontSize, spacing, isSelected ? WHITE : (isHovered ? LIGHTGRAY : GRAY));
        rowY += (float) (itemHeightPx + separator);
    }

    RenderScrollBar(data, maxItems);
    RenderButtons(data.ui.inventory.buttons, data.smallFont1, 5.0f);
    RenderButtons(data.ui.inventory.contextButtons, data.smallFont1, 5.0f);
    RenderCharacterInfo(data);

    // Render tooltips
    if (data.ui.inventory.hoveredIndex >= 0 && data.ui.inventory.hoveredIndex < (int)partyInventory.items.size()) {
        int itemId = partyInventory.items.at(data.ui.inventory.hoveredIndex);
        ItemInstance& inst = data.itemData.instanceData[itemId];
        ItemTemplate& tmpl = data.itemData.templateData[inst.templateId];
        std::string tooltip = "Some placeholder tooltip for: " + tmpl.name;

        Vector2 mouse = GetMousePosition();
        Vector2 size = MeasureTextEx(font, tooltip.c_str(), fontSize, spacing);
        size.x = ceilf(size.x); size.y = ceilf(size.y);

        Rectangle tipRect = {floorf(mouse.x + 8), ceilf(mouse.y + 8), size.x + 4, size.y + 4};
        DrawRectangleRec(tipRect, Color{15, 15, 15, 200});
        DrawRectangleLinesEx(tipRect, 1, DARKGRAY);
        DrawTextEx(font, tooltip.c_str(), {tipRect.x + 2, tipRect.y + 2}, fontSize, spacing, LIGHTGRAY);
    }
}

static void EquipSelectedItem(GameData& data) {
    auto& partyInventory = data.itemData.inventoryData[data.itemData.partyInventoryId];
    int itemId = partyInventory.items.at(data.ui.inventory.selectedIndex);
    ItemTemplate& tpl = data.itemData.templateData[GetItemTemplateId(data, itemId)];
    int charId = data.ui.selectedCharacter;
    switch(tpl.type) {
        case ItemType::Weapon: {
            int prevItem = GetEquippedItem(data, charId, ItemEquipSlot::Weapon1);
            SetEquippedItem(data, charId, ItemEquipSlot::Weapon1, itemId);
            partyInventory.items.erase(partyInventory.items.begin() + data.ui.inventory.selectedIndex);
            if(prevItem != -1) {
                partyInventory.items.push_back(prevItem);
            }
            data.ui.inventory.selectedIndex = -1;
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

bool HandleInventoryInput(GameData& data, GameEventQueue& eventQueue) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        PublishCloseInventoryEvent(eventQueue);
        return true;
    }
    Vector2 mouse = GetMousePosition();

    if (!CheckCollisionPointRec(mouse, invRect)) return false;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        float rowY = invRect.y + firstRowOffset;
        for (int i = 0; i < visibleItems; ++i) {
            int idx = i + data.ui.inventory.scrollOffset;
            if (idx >= (int)data.itemData.instanceData.size()) break;
            Rectangle row = {invRect.x + 4, rowY, invRect.width - rowRightMargin, (float) itemHeightPx};
            if (CheckCollisionPointRec(mouse, row)) {
                data.ui.inventory.selectedIndex = idx;
                UpdateContextButtons(data);
                return true;
            }
            rowY += (float) (itemHeightPx + separator);
        }
    }
    HandleInputButtons(data.ui.inventory.buttons);
    if(data.ui.inventory.buttons["close"].region.ConsumeClick()) {
        data.ui.inventory.buttons["close"].hovered = false;
        PublishCloseInventoryEvent(eventQueue);
    }
    HandleInputButtons(data.ui.inventory.contextButtons);
    if(data.ui.inventory.contextButtons["equip"].region.ConsumeClick()) {
        EquipSelectedItem(data);
    }
    return true;
}
