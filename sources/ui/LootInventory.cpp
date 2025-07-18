//
// Created by bison on 26-06-25.
//

#include "LootInventory.h"
#include "raymath.h"
#include "ui/UI.h"
#include "game/Items.h"
#include "Icons.h"

static const int separator = 1;
static const int itemHeightPx = 12;
static const Rectangle invRect = {140, 108, gameScreenWidth - 300, gameScreenHeight - 200};
static const int visibleItems = (int)((invRect.height - 50) / itemHeightPx);
static const int firstRowOffset = 16;
static const int rowRightMargin = 20;
static const Rectangle scrollBarRect = {invRect.x + invRect.width - 14, invRect.y + firstRowOffset, 10, invRect.height - firstRowOffset - 24};
static const int scrollbarMinHeight = 16;

void InitLootInventory(GameData& data) {
    data.ui.lootInventory.scrollOffset = 0;
    data.ui.lootInventory.selectedIndex = -1;
    data.ui.lootInventory.hoveredIndex = -1;
    data.ui.lootInventory.draggingScrollKnob = false;
    data.ui.lootInventory.dragOffsetY = 0;

    data.ui.lootInventory.buttons.clear();

    Button closeButton{};
    closeButton.label = "Close";
    closeButton.region = CreateClickRegion({invRect.x + invRect.width - 40 - 5, invRect.y + invRect.height - 18, 40, 11});
    closeButton.enabled = true;
    closeButton.hovered = false;
    data.ui.lootInventory.buttons["close"] = closeButton;

    Button takeAllButton{};
    takeAllButton.label = "Take All";
    takeAllButton.region = CreateClickRegion({invRect.x + invRect.width - 85 - 5, invRect.y + invRect.height - 18, 40, 11});
    takeAllButton.enabled = true;
    takeAllButton.hovered = false;
    data.ui.lootInventory.buttons["takeAll"] = takeAllButton;

    data.ui.lootInventory.contextButtons.clear();
}

static void UpdateContextButtons(GameData& data) {
    data.ui.lootInventory.contextButtons.clear();
    if(data.ui.lootInventory.selectedIndex == -1)
        return;

    Button takeButton{};
    takeButton.label = "Take";
    takeButton.region = CreateClickRegion({invRect.x + 5, invRect.y + invRect.height - 18, 40, 11});
    takeButton.enabled = true;
    takeButton.hovered = false;
    data.ui.lootInventory.contextButtons["take"] = takeButton;
}

void UpdateLootInventory(GameData& data, float dt) {
    Vector2 mouse = GetMousePosition();

    const auto& partyInventory = data.itemData.inventoryData[data.itemData.partyInventoryId];
    int maxItems = (int) partyInventory.items.size();

    // Scroll input
    if (CheckCollisionPointRec(mouse, invRect)) {
        int wheel = (int) GetMouseWheelMove();
        data.ui.lootInventory.scrollOffset -= wheel;
        data.ui.lootInventory.scrollOffset = Clamp(data.ui.lootInventory.scrollOffset, 0, std::max(0, maxItems - visibleItems));
    }

    // Dragging scrollbar knob
    float scrollRatio = (float)visibleItems / (float)maxItems;
    float knobHeight = Clamp(scrollBarRect.height * scrollRatio, (float)scrollbarMinHeight, scrollBarRect.height);
    float scrollRange = scrollBarRect.height - knobHeight;
    float knobY = scrollBarRect.y + (scrollRange * data.ui.lootInventory.scrollOffset / std::max(1, maxItems - visibleItems));
    Rectangle knobRect = {scrollBarRect.x, knobY, scrollBarRect.width, knobHeight};

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, knobRect)) {
        data.ui.lootInventory.draggingScrollKnob = true;
        data.ui.lootInventory.dragOffsetY = mouse.y - knobY;
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        data.ui.lootInventory.draggingScrollKnob = false;
    }

    if (data.ui.lootInventory.draggingScrollKnob) {
        float newKnobY = Clamp(mouse.y - data.ui.lootInventory.dragOffsetY, scrollBarRect.y, scrollBarRect.y + scrollRange);
        float scrollPercent = (newKnobY - scrollBarRect.y) / scrollRange;
        data.ui.lootInventory.scrollOffset = (int)(scrollPercent * (maxItems - visibleItems));
    }

    // Hover index
    data.ui.lootInventory.hoveredIndex = -1;
    float rowY = invRect.y + firstRowOffset;
    for (int i = 0; i < visibleItems; ++i) {
        int idx = i + data.ui.lootInventory.scrollOffset;
        if (idx >= maxItems) break;
        Rectangle row = {invRect.x + 4, rowY, invRect.width - rowRightMargin, (float) itemHeightPx};
        if (CheckCollisionPointRec(mouse, row)) {
            data.ui.lootInventory.hoveredIndex = idx;
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
        float knobY = scrollBarRect.y + (scrollRange * data.ui.lootInventory.scrollOffset / std::max(1, maxItems - visibleItems));
        Rectangle knobRect = {scrollBarRect.x, knobY, scrollBarRect.width, knobHeight};
        DrawRectangleRec(knobRect, DARKGRAY);
        DrawRectangleLinesEx(knobRect, 1, data.ui.lootInventory.draggingScrollKnob ? YELLOW : LIGHTGRAY);
    }
}

void RenderLootInventoryUI(GameData& data) {
    Font font = data.smallFont1;
    float fontSize = 5.0f;
    float spacing = 1.0f;

    DrawRectangleRec(invRect, Color{15, 15, 15, 200});
    DrawRectangleRoundedLinesEx(invRect, 0.02f, 4, 1.0f, DARKGRAY);

    DrawTextEx(font, "Container:", {invRect.x + 5, invRect.y + 6}, fontSize, spacing, WHITE);

    const auto& lootInventory = data.itemData.inventoryData[data.ui.lootInventory.inventoryId];
    int maxItems = (int) lootInventory.items.size();

    float rowY = invRect.y + firstRowOffset;
    for (int i = 0; i < visibleItems; ++i) {
        int idx = i + data.ui.lootInventory.scrollOffset;
        if (idx >= maxItems) break;

        int itemId = lootInventory.items.at(idx);
        ItemInstance& inst = data.itemData.instanceData[itemId];
        ItemTemplate& tmpl = data.itemData.templateData[inst.templateId];

        Rectangle row = {invRect.x + 4, rowY, invRect.width - rowRightMargin, (float) itemHeightPx};
        bool isSelected = (data.ui.lootInventory.selectedIndex == idx);
        bool isHovered = (data.ui.lootInventory.hoveredIndex == idx);

        Color border = isSelected ? YELLOW : (isHovered ? GRAY : DARKGRAY);
        DrawRectangleLinesEx(row, 1, border);

        std::string nameStr = tmpl.name;
        DrawTextEx(font, nameStr.c_str(), {row.x + 4, row.y + 3}, fontSize, spacing, isSelected ? WHITE : (isHovered ? LIGHTGRAY : GRAY));
        rowY += (float) (itemHeightPx + separator);
    }

    RenderScrollBar(data, maxItems);
    RenderButtons(data.ui.lootInventory.buttons, data.smallFont1, 5.0f);
    RenderButtons(data.ui.lootInventory.contextButtons, data.smallFont1, 5.0f);

    // Render tooltips
    if (data.ui.lootInventory.hoveredIndex >= 0 && data.ui.lootInventory.hoveredIndex < (int)lootInventory.items.size()) {
        int itemId = lootInventory.items.at(data.ui.lootInventory.hoveredIndex);
        ItemInstance& inst = data.itemData.instanceData[itemId];
        ItemTemplate& tmpl = data.itemData.templateData[inst.templateId];
        std::string tooltip = "Some placeholder tooltip for: " + tmpl.name;

        DrawToolTip(data.smallFont1, 5, 1, tooltip);
    }
}

bool HandleLootInventoryInput(GameData& data) {
    Vector2 mouse = GetMousePosition();
    HandleInputButtons(data.ui.lootInventory.buttons);
    HandleInputButtons(data.ui.lootInventory.contextButtons);

    if (IsKeyPressed(KEY_ESCAPE)) {
        PublishCloseLootInventoryEvent(data.ui.eventQueue);
        return true;
    }


    if (!CheckCollisionPointRec(mouse, invRect)) return false;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        float rowY = invRect.y + firstRowOffset;
        for (int i = 0; i < visibleItems; ++i) {
            int idx = i + data.ui.lootInventory.scrollOffset;
            if (idx >= (int)data.itemData.instanceData.size()) break;
            Rectangle row = {invRect.x + 4, rowY, invRect.width - rowRightMargin, (float) itemHeightPx};
            if (CheckCollisionPointRec(mouse, row)) {
                data.ui.lootInventory.selectedIndex = idx;
                UpdateContextButtons(data);
                return true;
            }
            rowY += (float) (itemHeightPx + separator);
        }
    }

    if(data.ui.lootInventory.buttons["close"].region.ConsumeClick()) {
        data.ui.lootInventory.buttons["close"].hovered = false;
        PublishCloseLootInventoryEvent(data.ui.eventQueue);
    }

    if(data.ui.lootInventory.buttons["takeAll"].region.ConsumeClick()) {
        data.ui.lootInventory.buttons["takeAll"].hovered = false;

        auto& partyInventory = data.itemData.inventoryData[data.itemData.partyInventoryId];
        auto& lootInventory = data.itemData.inventoryData[data.ui.lootInventory.inventoryId];
        for(auto& itemId : lootInventory.items) {
            partyInventory.items.push_back(itemId);
        }
        lootInventory.items.clear();
        data.ui.lootInventory.selectedIndex = -1;
        PublishCloseLootInventoryEvent(data.ui.eventQueue);
    }

    if(data.ui.lootInventory.contextButtons["take"].region.ConsumeClick()) {
        auto& lootInventory = data.itemData.inventoryData[data.ui.lootInventory.inventoryId];
        int itemId = lootInventory.items.at(data.ui.lootInventory.selectedIndex);
        lootInventory.items.erase(lootInventory.items.begin() + data.ui.lootInventory.selectedIndex);
        auto& partyInventory = data.itemData.inventoryData[data.itemData.partyInventoryId];
        partyInventory.items.push_back(itemId);
        data.ui.lootInventory.selectedIndex = -1;
        if(lootInventory.items.empty()) {
            PublishCloseLootInventoryEvent(data.ui.eventQueue);
        }
    }

    return true;
}
