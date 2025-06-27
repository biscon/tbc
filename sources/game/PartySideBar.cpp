//
// Created by bison on 25-06-25.
//

#include "PartySideBar.h"
#include "raylib.h"
#include "util/StringUtil.h"
#include <vector>
#include <string>

struct PartySidebarSlot {
    Rectangle rect;
    bool hovered = false;
};

static std::vector<PartySidebarSlot> sidebarSlots;

constexpr int portraitSize = 40;
constexpr int portraitMargin = 8;
constexpr int textHeight = 5;
constexpr int spacing = 6;
constexpr Color portraitColor = Color{15, 15, 15, 255};
constexpr Color hoverBorder = YELLOW;
constexpr Color borderColor = DARKGRAY;

void InitPartySideBar(GameData& data) {
    sidebarSlots.clear();

    int count = static_cast<int>(data.party.size());
    int x = gameScreenWidth - portraitSize - portraitMargin;

    for (int i = 0; i < count; ++i) {
        int y = portraitMargin + i * (portraitSize + textHeight + spacing);
        Rectangle r = {
                static_cast<float>(x),
                static_cast<float>(y),
                static_cast<float>(portraitSize),
                static_cast<float>(portraitSize)
        };
        PartySidebarSlot slot;
        slot.rect = r;
        sidebarSlots.emplace_back(slot);
    }
}

void UpdatePartySideBar(GameData& data, float dt) {

}

void RenderPartySideBarUI(GameData& data) {
    static const Color bgColor = Color{15, 15, 15, 255};

    int count = static_cast<int>(sidebarSlots.size());
    for (int i = 0; i < count; ++i) {
        auto& slot = sidebarSlots[i];
        //DrawRectangleRec(slot.rect, portraitColor);
        // Border
        //DrawRectangleLinesEx(slot.rect, 1, slot.hovered ? hoverBorder : borderColor);

        DrawRectangleRounded(slot.rect, 0.1f, 16, portraitColor);
        DrawRectangleRoundedLinesEx(slot.rect, 0.1f, 16, 1.0f, slot.hovered ? hoverBorder : borderColor);

        // Name
        int charId = data.party[i];
        std::string name = data.charData.name[charId];
        Vector2 textDim = MeasureTextEx(data.smallFont1, name.c_str(), textHeight, 1);

        Vector2 namePos;
        if(textDim.x <= slot.rect.width) {
            namePos = {
                    roundf(slot.rect.x + (slot.rect.width / 2.0f) - (textDim.x / 2.0f)),
                    floorf(slot.rect.y + slot.rect.height + 3.0f)
            };
        } else {
            name = TruncateWithEllipsis(name, 9);
            namePos = { slot.rect.x, slot.rect.y + slot.rect.height + 3};
        }
        DrawTextEx(data.smallFont1, name.c_str(), namePos, 5, 1, WHITE);
    }
}

bool HandlePartySideBarInput(GameData& data, GameEventQueue& eventQueue) {
    Vector2 mouse = GetMousePosition();
    for (auto& slot : sidebarSlots) {
        slot.hovered = CheckCollisionPointRec(mouse, slot.rect);
    }
    bool anyHovered = false;
    for (size_t i = 0; i < sidebarSlots.size(); ++i) {
        if (sidebarSlots[i].hovered) {
            anyHovered = true;
            // TODO: Emit event or change state to show inventory for character i
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                int charId = data.party[i];
                TraceLog(LOG_INFO, "Clicked on character: %s", data.charData.name[charId].c_str());
                PublishOpenInventoryEvent(eventQueue, charId);
                return true;
            }
        }
    }
    return anyHovered;
}
