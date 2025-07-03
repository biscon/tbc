//
// Created by bison on 25-06-25.
//

#include "PartySideBar.h"
#include "raylib.h"
#include "util/StringUtil.h"
#include "ui/UI.h"
#include "portraits.h"
#include <vector>
#include <string>

constexpr int portraitSize = 40;
constexpr int portraitMargin = 8;
constexpr int textHeight = 5;
constexpr int spacing = 6;
constexpr Color portraitColor = Color{15, 15, 15, 255};
constexpr Color hoverBorder = WHITE;
constexpr Color borderColor = DARKGRAY;

void InitPartySideBar(GameData& data) {
    data.ui.selectedCharacter = 0;
    data.ui.sideBar.sidebarSlots.clear();

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
        slot.region = CreateClickRegion(r);
        data.ui.sideBar.sidebarSlots.emplace_back(slot);
    }
    data.ui.sideBar.buttons.clear();
    Button menuButton{};
    menuButton.label = "Menu";
    menuButton.region = CreateClickRegion({gameScreenWidthF - portraitSize - portraitMargin, gameScreenHeightF - 20, portraitSize, 11});
    menuButton.enabled = true;
    menuButton.hovered = false;
    data.ui.sideBar.buttons["menu"] = menuButton;

    Button invButton{};
    invButton.label = "Inventory";
    invButton.region = CreateClickRegion({gameScreenWidthF - portraitSize - portraitMargin, gameScreenHeightF - 36, portraitSize, 11});
    invButton.enabled = true;
    invButton.hovered = false;
    data.ui.sideBar.buttons["inventory"] = invButton;
}

void UpdatePartySideBar(GameData& data, float dt) {

}

void RenderPartySideBarUI(GameData& data) {
    int count = static_cast<int>(data.ui.sideBar.sidebarSlots.size());
    for (int i = 0; i < count; ++i) {
        auto& slot = data.ui.sideBar.sidebarSlots[i];
        int charId = data.party[i];
        //DrawRectangleRec(slot.rect, portraitColor);
        // Border
        //DrawRectangleLinesEx(slot.rect, 1, slot.hovered ? hoverBorder : borderColor);
        bool selected = data.ui.selectedCharacter == charId;
        DrawRectangleRounded(slot.region.rect, 0.1f, 4, portraitColor);
        DrawRectangleRoundedLinesEx(slot.region.rect, 0.1f, 4, 1.0f,
                                    selected ? YELLOW : (slot.hovered ? hoverBorder : borderColor));

        DrawPortrait(data, slot.region.rect.x, slot.region.rect.y, GRAY, PORTRAIT_DEFAULT);
        // Name

        std::string name = data.charData.name[charId];
        Vector2 textDim = MeasureTextEx(data.smallFont1, name.c_str(), textHeight, 1);

        Vector2 namePos;
        if(textDim.x <= slot.region.rect.width) {
            namePos = {
                    roundf(slot.region.rect.x + (slot.region.rect.width / 2.0f) - (textDim.x / 2.0f)),
                    floorf(slot.region.rect.y + slot.region.rect.height + 3.0f)
            };
        } else {
            name = TruncateWithEllipsis(name, 9);
            namePos = { slot.region.rect.x, slot.region.rect.y + slot.region.rect.height + 3};
        }
        DrawTextEx(data.smallFont1, name.c_str(), namePos, 5, 1, selected ? YELLOW : (slot.hovered ? hoverBorder : borderColor));
    }

    RenderButtons(data.ui.sideBar.buttons, data.smallFont1, 5.0f);
}

bool HandlePartySideBarInput(GameData& data) {
    Vector2 mouse = GetMousePosition();
    if(!data.ui.inCombat) {
        for (auto &slot: data.ui.sideBar.sidebarSlots) {
            slot.hovered = CheckCollisionPointRec(mouse, slot.region.rect);
            slot.region.Update(mouse);
        }
    }
    HandleInputButtons(data.ui.sideBar.buttons);
    
    for (size_t i = 0; i < data.ui.sideBar.sidebarSlots.size(); ++i) {
        if (data.ui.sideBar.sidebarSlots[i].region.ConsumeClick()) {
            int charId = data.party[i];
            data.ui.selectedCharacter = charId;
            TraceLog(LOG_INFO, "Selected character: %s", data.charData.name[charId].c_str());
            return true;
        }
    }
    if(data.ui.sideBar.buttons["inventory"].region.ConsumeClick()) {
        if(data.state != GameState::INVENTORY) {
            PublishOpenInventoryEvent(data.ui.eventQueue, data.ui.selectedCharacter);
        } else {
            PublishCloseInventoryEvent(data.ui.eventQueue);
        }
        return true;
    }
    if(data.ui.sideBar.buttons["menu"].region.ConsumeClick()) {
        PublishOpenMenuEvent(data.ui.eventQueue);
        return true;
    }

    return false;
}
