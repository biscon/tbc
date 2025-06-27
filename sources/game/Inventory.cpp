//
// Created by bison on 26-06-25.
//

#include "Inventory.h"
#include "raymath.h"

static const int separator = 1;
static const int itemHeightPx = 12;
static const Rectangle invRect = {100, 50, gameScreenWidth - 200, gameScreenHeight - 100};
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

void RenderInventoryUI(GameData& data) {
    Font font = data.smallFont1;
    float fontSize = 5.0f;
    float spacing = 1.0f;

    DrawRectangleRec(invRect, Color{15, 15, 15, 200});
    DrawRectangleLinesEx(invRect, 1, DARKGRAY);

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
                return true;
            }
            rowY += (float) (itemHeightPx + separator);
        }
    }

    return true;
}
