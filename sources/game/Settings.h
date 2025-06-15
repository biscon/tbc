//
// Created by Stinus Troels Petersen on 14/06/2025.
//

#ifndef SANDBOX_SETTINGS_H
#define SANDBOX_SETTINGS_H

#include <string>
#include "data/GameData.h"
#include "util/GameEventQueue.h"

void ApplySettings(SettingsData& settings);
void SaveSettings(const SettingsData& settings);
void InitSettings(SettingsData& data, const std::string &filename);

#endif //SANDBOX_SETTINGS_H
