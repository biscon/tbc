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
#include "PathFindingData.h"

struct ClickRegion {
    Rectangle rect;
    bool pressed = false;
    bool hovered = false;
    bool showToolTip = false;
    bool wasClicked = false;
    bool wasDoubleClicked = false;
    float lastClickTime = -1.0f;
    float doubleClickThreshold = 0.3f; // seconds

    float toolTipDelay = 0.5f; // seconds
    float hoverStartTime = -1.0f;
    Vector2 lastMousePos = {-9999, -9999};

    void Update(Vector2 mouse) {
        float timeNow = static_cast<float>(GetTime());
        hovered = false;
        showToolTip = false;

        bool mouseInRegion = CheckCollisionPointRec(mouse, rect);
        if (mouseInRegion) {
            hovered = true;

            if (lastMousePos.x != mouse.x || lastMousePos.y != mouse.y) {
                hoverStartTime = timeNow; // Reset on movement
                lastMousePos = mouse;
            }

            if (hoverStartTime >= 0 && timeNow - hoverStartTime >= toolTipDelay) {
                showToolTip = true;
            }
        } else {
            hoverStartTime = -1.0f;
            lastMousePos = {-9999, -9999};
        }

        // Start press
        if (hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            pressed = true;
        }

        // Handle release
        if (pressed && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            pressed = false;
            if (mouseInRegion) {
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
        if (pressed && !mouseInRegion && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
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

enum class CharacterTabs {
    Stats,
    Equipment,
    Skills,
    COUNT
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
    CharacterTabs currentCharTab = CharacterTabs::Stats;
    ClickRegion leftTabRegion;
    ClickRegion rightTabRegion;
};

enum class ActionBarAction {
    Move,
    Attack,
    Reload,
    EndTurn
};

struct ActionBarIcon {
    ActionBarAction action;
    std::string text;
    std::string tooltip;
    int apCost = -1;
    ClickRegion region;
    bool enabled;
    bool selectable;
    int icon = -1;
};

struct ActionBarUI {
    ClickRegion switchWeapons;
    std::array<ActionBarIcon, 8> actionIcons;
    std::array<ActionBarIcon, 4> modeIcons;
    int selectedActionIdx = -1;
    int selectedModeIdx = 0;
    int previewApUse = -1;
    bool hovered = false;
};

struct AttackInfo {
    float hitChance = 0;
    int apCost = 0;
};

struct PlayFieldUI {
    int floatingStatsCharacter = -1;
    Path movePath;
    bool validMovePath = false;
    AttackInfo attackInfo;
};

struct UiState {
    PartySideBarData sideBar;
    InventoryUiState inventory;
    ActionBarUI actionBar{};
    PlayFieldUI playField;
    int selectedCharacter = 0;
    GameEventQueue eventQueue;
    bool inCombat = false;
    int iconSpriteSheet;
    int portraitSpriteSheet;
    bool showActionBar = false;
};

#endif //SANDBOX_UIDATA_H
