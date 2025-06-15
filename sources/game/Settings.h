//
// Created by Stinus Troels Petersen on 14/06/2025.
//

#ifndef SANDBOX_SETTINGS_H
#define SANDBOX_SETTINGS_H

#include <string>
#include "data/GameData.h"
#include "util/GameEventQueue.h"

void InitSettings(SettingsData& data, const std::string &filename);
void RenderSettingsUI(GameData& data);
void UpdateSettings(GameData& data, float dt);
void HandleSettingsInput(GameData& data, GameEventQueue& eventQueue);

#endif //SANDBOX_SETTINGS_H
