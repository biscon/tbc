//
// Created by Stinus Troels Petersen on 14/06/2025.
//

#include <fstream>
#include "Settings.h"
#include "raygui.h"

void ApplySettings(SettingsData& settings) {
    Resolution res = settings.availableResolutions[settings.selectedResolutionIndex];

    switch (settings.displayMode) {
        case DisplayMode::Windowed:
            ClearWindowState(FLAG_FULLSCREEN_MODE);
            ClearWindowState(FLAG_BORDERLESS_WINDOWED_MODE);
            SetWindowSize(res.width, res.height);

            // 🧠 Manually center on primary monitor (assumes monitor starts at 0,0)
            {
                int monitorWidth = GetMonitorWidth(0);
                int monitorHeight = GetMonitorHeight(0);
                int winX = (monitorWidth - res.width) / 2;
                int winY = (monitorHeight - res.height) / 2;
                SetWindowPosition(winX, winY);
            }
            break;

        case DisplayMode::Fullscreen:
            ClearWindowState(FLAG_BORDERLESS_WINDOWED_MODE);
            SetWindowState(FLAG_FULLSCREEN_MODE);
            SetWindowSize(res.width, res.height);
            break;

        case DisplayMode::Borderless:
            ClearWindowState(FLAG_FULLSCREEN_MODE);
            SetWindowState(FLAG_BORDERLESS_WINDOWED_MODE);
            SetWindowSize(GetMonitorWidth(0), GetMonitorHeight(0));
            break;
    }

    settings.originalResolutionIndex = settings.selectedResolutionIndex;
    settings.originalDisplayMode = settings.displayMode;
    settings.needsApply = false;
}

void SaveSettings(const SettingsData& settings) {
    nlohmann::json j;
    j["resolutionIndex"] = settings.selectedResolutionIndex;
    j["displayMode"] = static_cast<int>(settings.displayMode);

    std::ofstream file(settings.filename);
    if (file) {
        file << j.dump(2);
    }
}

void InitSettings(SettingsData& data, const std::string &filename) {
    data.filename = filename;

    // Load from JSON
    std::ifstream file(filename);
    if (file) {
        nlohmann::json j;
        file >> j;

        data.selectedResolutionIndex = j.value("resolutionIndex", 0);
        data.displayMode = static_cast<DisplayMode>(j.value("displayMode", 0));
    }

    data.originalResolutionIndex = data.selectedResolutionIndex;
    data.originalDisplayMode = data.displayMode;

    // Populate available resolutions (multiples of 480x270)
    data.availableResolutions.clear();
    const int baseW = 480, baseH = 270;
    const int scales[] = {1, 2, 3, 4, 6, 8};

    for (int scale : scales) {
        int w = baseW * scale;
        int h = baseH * scale;
        if (w <= GetMonitorWidth(0) && h <= GetMonitorHeight(0)) {
            data.availableResolutions.push_back({w, h});
        }
    }

    if (data.selectedResolutionIndex >= data.availableResolutions.size()) {
        data.selectedResolutionIndex = 0;
    }
}

static Color GuiIntToColor(int colInt) {
    Color c;
    c.r = (colInt >> 0) & 0xFF;
    c.g = (colInt >> 8) & 0xFF;
    c.b = (colInt >> 16) & 0xFF;
    c.a = (colInt >> 24) & 0xFF;
    return c;
}

void RenderSettingsUI(GameData& data) {
    SettingsData& settings = data.settingsData;

    int x = 20;
    int y = 40;

    Color labelColor = { 255, 200, 0, 255 }; // Yellowish
    GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, ColorToInt(labelColor));

    // --- Resolution Label ---
    GuiLabel({ (float)x, (float)y, 100, 24 }, "Resolution:");

    y += 30;

    // --- Listbox-style resolution selector ---
    const int itemHeight = 18;
    int listHeight = itemHeight * (int)settings.availableResolutions.size();
    Rectangle listRect = { (float)x, (float)y, 200, (float)listHeight };

    Color bgColor = GuiIntToColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR));
    Color borderColor = GuiIntToColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL));
    Color baseColor = GuiIntToColor(GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL));
    Color hoverColor = GuiIntToColor(GuiGetStyle(DEFAULT, BASE_COLOR_FOCUSED));
    Color selectColor = GuiIntToColor(GuiGetStyle(DEFAULT, BASE_COLOR_PRESSED));
    Color textColor = labelColor;

    // Draw background and border
    DrawRectangleRec(listRect, bgColor);
    DrawRectangleLinesEx(listRect, 1, borderColor);

    Vector2 mousePos = GetMousePosition();

    for (size_t i = 0; i < settings.availableResolutions.size(); ++i) {
        Rectangle itemRect = { listRect.x, listRect.y + i * itemHeight, listRect.width, (float)itemHeight };
        bool hovered = CheckCollisionPointRec(mousePos, itemRect);

        // Background color for item
        if ((int)i == settings.selectedResolutionIndex) {
            DrawRectangleRec(itemRect, selectColor);
        } else if (hovered) {
            DrawRectangleRec(itemRect, hoverColor);
        } else {
            DrawRectangleRec(itemRect, baseColor);
        }

        // Draw resolution text with padding
        char entryText[32];
        snprintf(entryText, sizeof(entryText), "%dx%d", settings.availableResolutions[i].width, settings.availableResolutions[i].height);

        DrawText(entryText, (int)itemRect.x + 10, (int)itemRect.y + 6, 10, textColor);

        if (hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            settings.selectedResolutionIndex = (int)i;
            settings.needsApply = true;
        }
    }

    // --- Display Mode ---
    y += listHeight + 20;
    GuiLabel({ (float)x, (float)y, 120, 24 }, "Display Mode:");

    static int displayModeIndex = 0;
    displayModeIndex = static_cast<int>(settings.displayMode);

    y += 30;
    Rectangle toggleRect = { (float)x, (float)y, 200, 24 };
    static const char* modeText = "Windowed;Fullscreen;Borderless";

    GuiToggleGroup(toggleRect, modeText, &displayModeIndex);
    if ((int)settings.displayMode != displayModeIndex) {
        settings.displayMode = static_cast<DisplayMode>(displayModeIndex);
        settings.needsApply = true;
    }

    // --- Buttons Centered ---
    y += 60;
    const float applyBtnWidth = 120;
    const float saveBtnWidth = 140;
    const float spacing = 20;
    const float totalBtnWidth = applyBtnWidth + saveBtnWidth + spacing;
    float btnStartX = (480 - totalBtnWidth) / 2;

    Rectangle applyBtn = { btnStartX, (float)y, applyBtnWidth, 30 };
    Rectangle saveBtn = { btnStartX + applyBtnWidth + spacing, (float)y, saveBtnWidth, 30 };

    if (GuiButton(applyBtn, "Apply")) {
        ApplySettings(settings);
    }
    if (GuiButton(saveBtn, "Save & Close")) {
        ApplySettings(settings);
        SaveSettings(settings);
        // TODO: signal game state to exit settings
    }
}


void HandleSettingsInput(GameData& data, GameEventQueue& eventQueue) {
    // Optional hotkey support
}

void UpdateSettings(GameData& data, float dt) {
    // Optional animation / transitions
}

