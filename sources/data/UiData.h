//
// Created by bison on 26-06-25.
//

#ifndef SANDBOX_UIDATA_H
#define SANDBOX_UIDATA_H


#include <vector>
#include <string>
#include <unordered_map>
#include "raylib.h"
#include "util/GameEventQueue.h"
#include "character/Skill.h"

struct ClickRegion {
    Rectangle rect;
    bool pressed = false;
    bool hovered = false;
    bool wasClicked = false;
    bool wasDoubleClicked = false;


    float lastClickTime = -1.0f;
    float doubleClickThreshold = 0.3f; // seconds

    void Update(Vector2 mouse) {
        float timeNow = static_cast<float>(GetTime());

        if (CheckCollisionPointRec(mouse, rect)) {
            hovered = true;
        } else {
            hovered = false;
        }

        // Start press
        if (hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            pressed = true;
        }

        // Handle release
        if (pressed && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            pressed = false;
            if (CheckCollisionPointRec(mouse, rect)) {
                wasClicked = true;

                if (timeNow - lastClickTime <= doubleClickThreshold) {
                    wasDoubleClicked = true;
                }

                lastClickTime = timeNow;
            }
        } else {
            wasClicked = false;
            wasDoubleClicked = false;
        }

        // Cancel if dragged out
        if (pressed && !CheckCollisionPointRec(mouse, rect) && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            pressed = false;
        }
    }

    bool ConsumeClick() {
        if (wasClicked && !wasDoubleClicked) {
            wasClicked = false;
            return true;
        }
        return false;
    }

    bool ConsumeDblClick() {
        if (wasDoubleClicked) {
            wasDoubleClicked = false;
            wasClicked = false; // clear single click too
            return true;
        }
        return false;
    }
};


ClickRegion CreateClickRegion(Rectangle rect);

struct Button {
    std::string label;
    ClickRegion region;
    bool hovered = false;
    bool enabled = true;
};

struct PartySidebarSlot {
    ClickRegion region;
    bool hovered = false;
};

struct PartySideBarData {
    std::vector<PartySidebarSlot> sidebarSlots;
    std::unordered_map<std::string, Button> buttons;
};

struct InventoryUiState {
    int scrollOffset = 0;
    int selectedIndex = -1;
    int hoveredIndex = -1;
    bool draggingScrollKnob = false;
    float dragOffsetY = 0;
    ClickRegion weapon1Region;
    ClickRegion weapon2Region;
    std::unordered_map<std::string, Button> buttons;
    std::unordered_map<std::string, Button> contextButtons;
};

struct ActionIcon {
    char text[64];
    char description[256];
    bool disabled;
    Skill* skill;
};

struct ActionBarUI {
    int actionIconScrollIndex;
    bool showActionBarTitle;
};

struct PlayFieldUI {
    int floatingStatsCharacter = -1;
};

struct UiState {
    PartySideBarData sideBar;
    InventoryUiState inventory;
    ActionBarUI actionBar{};
    PlayFieldUI playField;
    int selectedCharacter = 0;
    GameEventQueue eventQueue;

};

#endif //SANDBOX_UIDATA_H
