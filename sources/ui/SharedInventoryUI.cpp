//
// Created by Stinus Troels Petersen on 20/07/2025.
//

#include "SharedInventoryUI.h"
#include "raymath.h"
#include "UI.h"

static const int itemHeightPx = 12;
static const int scrollbarMinHeight = 16;
static const int separator = 1;
static const int firstRowOffset = 16;
static const int rowRightMargin = 20;

void InitInventoryListState(InventoryListState& list, const Rectangle& invRect, const std::string& name) {
    list.scrollOffset = 0;
    list.selectedIndex = -1;
    list.hoveredIndex = -1;
    list.draggingScrollKnob = false;
    list.dragOffsetY = 0;
    list.invRect = invRect;
    list.visibleItems = (int)((invRect.height - 50) / itemHeightPx);
    list.scrollBarRect = {invRect.x + invRect.width - 14, invRect.y + firstRowOffset,
                               10, invRect.height - firstRowOffset - 24};
    list.name = name;
}

void UpdateInventoryListState(GameData& data, InventoryListState& list, int inventoryId) {
    Vector2 mouse = GetMousePosition();

    const auto& partyInventory = data.itemData.inventoryData[inventoryId];
    int maxItems = (int) partyInventory.items.size();

    // Scroll input
    if (CheckCollisionPointRec(mouse, list.invRect)) {
        int wheel = (int) GetMouseWheelMove();
        list.scrollOffset -= wheel;
        list.scrollOffset = Clamp(list.scrollOffset, 0, std::max(0, maxItems - list.visibleItems));
    }

    // Dragging scrollbar knob
    float scrollRatio = (float)list.visibleItems / (float)maxItems;
    float knobHeight = Clamp(list.scrollBarRect.height * scrollRatio, (float)scrollbarMinHeight, list.scrollBarRect.height);
    float scrollRange = list.scrollBarRect.height - knobHeight;
    float knobY = list.scrollBarRect.y + (scrollRange * list.scrollOffset / std::max(1, maxItems - list.visibleItems));
    Rectangle knobRect = {list.scrollBarRect.x, knobY, list.scrollBarRect.width, knobHeight};

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, knobRect)) {
        list.draggingScrollKnob = true;
        list.dragOffsetY = mouse.y - knobY;
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        list.draggingScrollKnob = false;
    }

    if (list.draggingScrollKnob) {
        float newKnobY = Clamp(mouse.y - list.dragOffsetY, list.scrollBarRect.y, list.scrollBarRect.y + scrollRange);
        float scrollPercent = (newKnobY - list.scrollBarRect.y) / scrollRange;
        list.scrollOffset = (int)(scrollPercent * (maxItems - list.visibleItems));
    }

    // Hover index
    list.hoveredIndex = -1;
    float rowY = list.invRect.y + firstRowOffset;
    for (int i = 0; i < list.visibleItems; ++i) {
        int idx = i + list.scrollOffset;
        if (idx >= maxItems) break;
        Rectangle row = {list.invRect.x + 4, rowY, list.invRect.width - rowRightMargin, (float) itemHeightPx};
        if (CheckCollisionPointRec(mouse, row)) {
            list.hoveredIndex = idx;
            break;
        }
        rowY += (float) (itemHeightPx + separator);
    }
}

static void RenderListScrollBar(InventoryListState& list, int maxItems) {
    DrawRectangleLinesEx(list.scrollBarRect, 1, DARKGRAY);
    if (maxItems > list.visibleItems) {
        float scrollRatio = (float)list.visibleItems / (float)maxItems;
        float knobHeight = Clamp(list.scrollBarRect.height * scrollRatio, (float)scrollbarMinHeight, list.scrollBarRect.height);
        float scrollRange = list.scrollBarRect.height - knobHeight;
        float knobY = list.scrollBarRect.y + (scrollRange * list.scrollOffset / std::max(1, maxItems - list.visibleItems));
        Rectangle knobRect = {list.scrollBarRect.x, knobY, list.scrollBarRect.width, knobHeight};
        DrawRectangleRec(knobRect, DARKGRAY);
        DrawRectangleLinesEx(knobRect, 1, list.draggingScrollKnob ? YELLOW : LIGHTGRAY);
    }
}

void RenderInventoryListUI(GameData& data, InventoryListState& list, int inventoryId) {
    Font font = data.smallFont1;
    float fontSize = 5.0f;
    float spacing = 1.0f;

    DrawRectangleRec(list.invRect, Color{15, 15, 15, 200});
    DrawRectangleRoundedLinesEx(list.invRect, 0.02f, 4, 1.0f, DARKGRAY);

    DrawTextEx(font, list.name.c_str(), {list.invRect.x + 5, list.invRect.y + 6}, fontSize, spacing, WHITE);

    const auto& partyInventory = data.itemData.inventoryData[inventoryId];
    int maxItems = (int) partyInventory.items.size();

    float rowY = list.invRect.y + firstRowOffset;
    for (int i = 0; i < list.visibleItems; ++i) {
        int idx = i + list.scrollOffset;
        if (idx >= maxItems) break;

        int itemId = partyInventory.items.at(idx);
        ItemInstance& inst = data.itemData.instanceData[itemId];
        ItemTemplate& tmpl = data.itemData.templateData[inst.templateId];

        Rectangle row = {list.invRect.x + 4, rowY, list.invRect.width - rowRightMargin, (float) itemHeightPx};
        bool isSelected = (list.selectedIndex == idx);
        bool isHovered = (list.hoveredIndex == idx);

        Color border = isSelected ? YELLOW : (isHovered ? GRAY : DARKGRAY);
        DrawRectangleLinesEx(row, 1, border);

        std::string nameStr = tmpl.name;
        DrawTextEx(font, nameStr.c_str(), {row.x + 4, row.y + 3}, fontSize, spacing, isSelected ? WHITE : (isHovered ? LIGHTGRAY : GRAY));
        rowY += (float) (itemHeightPx + separator);
    }

    RenderListScrollBar(list, maxItems);
}

void RenderInventoryListToolTips(GameData& data, InventoryListState& list, int inventoryId) {
    const auto& partyInventory = data.itemData.inventoryData[inventoryId];
    if (list.hoveredIndex >= 0 && list.hoveredIndex < (int)partyInventory.items.size()) {
        int itemId = partyInventory.items.at(list.hoveredIndex);
        ItemInstance& inst = data.itemData.instanceData[itemId];
        ItemTemplate& tmpl = data.itemData.templateData[inst.templateId];
        std::string tooltip = "Some placeholder tooltip for: " + tmpl.name;

        DrawToolTip(data.smallFont1, 5, 1, tooltip);
    }
}

bool HandleInputInventoryList(GameData& data, InventoryListState& list, int inventoryId) {
    Vector2 mouse = GetMousePosition();
    const auto& inventory = data.itemData.inventoryData[inventoryId];
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        float rowY = list.invRect.y + firstRowOffset;
        for (int i = 0; i < list.visibleItems; ++i) {
            int idx = i + list.scrollOffset;
            if (idx >= (int)inventory.items.size()) break;
            Rectangle row = {list.invRect.x + 4, rowY, list.invRect.width - rowRightMargin, (float) itemHeightPx};
            if (CheckCollisionPointRec(mouse, row)) {
                list.selectedIndex = idx;
                return true;
            }
            rowY += (float) (itemHeightPx + separator);
        }
    }
    return false;
}


