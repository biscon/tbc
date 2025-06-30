//
// Created by bison on 25-06-25.
//

#ifndef SANDBOX_PARTYSIDEBAR_H
#define SANDBOX_PARTYSIDEBAR_H

#include "data/GameData.h"
#include "util/GameEventQueue.h"

void InitPartySideBar(GameData& data); // call after level is loaded to setup the sidebar
void RenderPartySideBarUI(GameData& data);
void UpdatePartySideBar(GameData& data, float dt);
bool HandlePartySideBarInput(GameData& data);


#endif //SANDBOX_PARTYSIDEBAR_H
