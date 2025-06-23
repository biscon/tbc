//
// Created by Stinus Troels Petersen on 14/06/2025.
//

#include <fstream>
#include "Settings.h"
#include "raygui.h"

void ApplySettings(SettingsData& settings) {
    Resolution res = settings.availableResolutions[settings.selectedResolutionIndex];

    switch (settings.displayMode) {
        case DisplayMode::Windowed: {
            ClearWindowState(FLAG_FULLSCREEN_MODE);
            ClearWindowState(FLAG_BORDERLESS_WINDOWED_MODE);
            SetWindowSize(res.width, res.height);

            // Manually center on primary monitor (assumes monitor starts at 0,0)
            int monitorWidth = GetMonitorWidth(0);
            int monitorHeight = GetMonitorHeight(0);
            int winX = (monitorWidth - res.width) / 2;
            int winY = (monitorHeight - res.height) / 2;
            SetWindowPosition(winX, winY);
            break;
        }

        case DisplayMode::Fullscreen: {
            ClearWindowState(FLAG_BORDERLESS_WINDOWED_MODE);
            SetWindowSize(res.width, res.height);
            SetWindowState(FLAG_FULLSCREEN_MODE);
            break;
        }

        case DisplayMode::Borderless: {
            ClearWindowState(FLAG_FULLSCREEN_MODE);
            SetWindowState(FLAG_BORDERLESS_WINDOWED_MODE);
            SetWindowSize(GetMonitorWidth(0), GetMonitorHeight(0));
            break;
        }
    }

    settings.originalResolutionIndex = settings.selectedResolutionIndex;
    settings.originalDisplayMode = settings.displayMode;

    if(settings.fpsLock) {
        TraceLog(LOG_INFO, "Enabling fps lock!!!!!!!!!!!!!!!!!!!!!!!!!!");
        SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    } else {
        TraceLog(LOG_INFO, "Disabling fps lock!!!!!!!!!!!!!!!!!!!!!!!!!!");
        SetTargetFPS(0);
    }

    settings.needsApply = false;
}

void SaveSettings(const SettingsData& settings) {
    nlohmann::json j;
    j["resolutionIndex"] = settings.selectedResolutionIndex;
    j["displayMode"] = static_cast<int>(settings.displayMode);
    j["showFPS"] = settings.showFPS;
    j["lockFPS"] = settings.fpsLock;

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
        if(j["showFPS"] != nullptr) {
            j["showFPS"].get_to(data.showFPS);
        }
        if(j.contains("lockFPS")) {
            j["lockFPS"].get_to(data.fpsLock);
        }
    }

    data.originalResolutionIndex = data.selectedResolutionIndex;
    data.originalDisplayMode = data.displayMode;

    // Populate available resolutions
    data.availableResolutions.clear();
    const int baseW = gameScreenWidth, baseH = gameScreenHeight;
    const int scales[] = {1, 2, 3, 4, 6};

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
